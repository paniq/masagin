// vkcore: see vkcore.hpp
#include "vkcore.hpp"
#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vector>
#include <map>
#include <string>
#include <mutex>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct VkcImg
{
  VkImage Image; VkImageView View; VkDeviceMemory Mem;
  int W,H,Mips; bool RT; VkImageLayout Layout;
};

static bool VkDebug(){ static int v=-1; if(v<0) v=getenv("MSG_VK_DEBUG")?1:0; return v!=0; }
namespace {

#define CHECK(x) do { VkResult r_=(x); if(r_!=VK_SUCCESS) { fprintf(stderr,"[vk] %s failed: %d (%s:%d)\n",#x,(int)r_,__FILE__,__LINE__); abort(); } } while(0)

struct Chunk { VkBuffer Buf; VkDeviceMemory Mem; uint8_t *Map; uint64_t Size, Used; };

struct State
{
  VkcConfig Cfg; bool Headless, Ready;
  VkInstance Inst; VkPhysicalDevice Phys; VkDevice Dev; VkQueue Queue; uint32_t Family;
  VkPhysicalDeviceMemoryProperties MemProps; VkPhysicalDeviceProperties Props;
  VkSurfaceKHR Surface; VkSwapchainKHR Swap; VkFormat SwapFormat; VkExtent2D SwapExtent;
  std::vector<VkImage> SwapImages; std::vector<VkSemaphore> RenderDone; uint32_t SwapIndex; bool SwapValid;
  VkSemaphore Acquire;
  VkCommandPool Pool; VkCommandBuffer Cmd; VkFence Fence;
  VkCommandPool ImmPool;
  std::mutex Mutex;
  std::vector<Chunk> Chunks;
  Chunk Ring;
  VkBuffer QuadIB; VkDeviceMemory QuadIBMem;
  VkDescriptorSetLayout DSL; VkPipelineLayout PL;
  std::vector<VkDescriptorPool> DPools; size_t DPoolUsed; uint32_t DPoolCount;
  std::map<std::string,VkPipeline> Pipes;
  std::map<std::string,VkSampler> Samplers;
  std::map<std::string,VkShaderModule> Modules;
  VkcImg Screen; VkImage DepthImg; VkImageView DepthView; VkDeviceMemory DepthMem; bool DepthInit;
  VkcImg *Black;
  // frame state
  bool InFrame; bool Rendering; VkcImg *Target; bool TargetIsScreen; int RectX0,RectY0,RectX1,RectY1;
  int PendingClear; uint32_t PendingColor; bool StateDirty;
  VkPipeline BoundPipe; VkDescriptorSet BoundSet; VkImageView SetViews[VKC_MAXTEX]; VkSampler SetSamplers[VKC_MAXTEX]; bool SetValid;
  VkBuffer BoundVB; uint64_t BoundVBOff; VkBuffer BoundIB; uint64_t BoundIBOff; int BoundIBType;
  uint32_t Frame, Draws;
  VkQueryPool QP; int QN;
  VkBuffer Readback; VkDeviceMemory ReadbackMem; void *ReadbackMap; bool DumpThisFrame;
};
State G;

uint32_t memType(uint32_t bits,VkMemoryPropertyFlags want)
{
  for(uint32_t i=0;i<G.MemProps.memoryTypeCount;i++)
    if((bits&(1u<<i)) && (G.MemProps.memoryTypes[i].propertyFlags&want)==want) return i;
  fprintf(stderr,"[vk] no memory type\n"); abort();
}

void makeBuffer(uint64_t size,VkBufferUsageFlags usage,VkMemoryPropertyFlags props,VkBuffer &buf,VkDeviceMemory &mem,void **map)
{
  VkBufferCreateInfo bi={VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO}; bi.size=size; bi.usage=usage; bi.sharingMode=VK_SHARING_MODE_EXCLUSIVE;
  CHECK(vkCreateBuffer(G.Dev,&bi,0,&buf));
  VkMemoryRequirements mr; vkGetBufferMemoryRequirements(G.Dev,buf,&mr);
  VkMemoryAllocateInfo ai={VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO}; ai.allocationSize=mr.size; ai.memoryTypeIndex=memType(mr.memoryTypeBits,props);
  CHECK(vkAllocateMemory(G.Dev,&ai,0,&mem)); CHECK(vkBindBufferMemory(G.Dev,buf,mem,0));
  if(map) CHECK(vkMapMemory(G.Dev,mem,0,VK_WHOLE_SIZE,0,map));
}

const VkMemoryPropertyFlags HOSTVIS = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
const VkBufferUsageFlags GEOUSAGE = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT|VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

void barrier(VkCommandBuffer cb,VkImage img,VkImageAspectFlags aspect,VkImageLayout o,VkImageLayout n,int levels=1)
{
  if(o==n) return;
  VkImageMemoryBarrier2 b={VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
  b.srcStageMask=VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT; b.dstStageMask=VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
  b.srcAccessMask=VK_ACCESS_2_MEMORY_WRITE_BIT; b.dstAccessMask=VK_ACCESS_2_MEMORY_READ_BIT|VK_ACCESS_2_MEMORY_WRITE_BIT;
  b.oldLayout=o; b.newLayout=n; b.srcQueueFamilyIndex=b.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED; b.image=img;
  b.subresourceRange.aspectMask=aspect; b.subresourceRange.levelCount=levels; b.subresourceRange.layerCount=1;
  VkDependencyInfo di={VK_STRUCTURE_TYPE_DEPENDENCY_INFO}; di.imageMemoryBarrierCount=1; di.pImageMemoryBarriers=&b;
  vkCmdPipelineBarrier2(cb,&di);
}
void imgBarrier(VkCommandBuffer cb,VkcImg *t,VkImageLayout n)
{ barrier(cb,t->Image,VK_IMAGE_ASPECT_COLOR_BIT,t->Layout,n,t->Mips); t->Layout=n; }

VkCommandBuffer immBegin()
{
  VkCommandBufferAllocateInfo ai={VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO}; ai.commandPool=G.ImmPool; ai.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY; ai.commandBufferCount=1;
  VkCommandBuffer cb; CHECK(vkAllocateCommandBuffers(G.Dev,&ai,&cb));
  VkCommandBufferBeginInfo bi={VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO}; bi.flags=VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  CHECK(vkBeginCommandBuffer(cb,&bi)); return cb;
}
void immEnd(VkCommandBuffer cb)
{
  CHECK(vkEndCommandBuffer(cb));
  VkFenceCreateInfo fi={VK_STRUCTURE_TYPE_FENCE_CREATE_INFO}; VkFence f; CHECK(vkCreateFence(G.Dev,&fi,0,&f));
  VkSubmitInfo si={VK_STRUCTURE_TYPE_SUBMIT_INFO}; si.commandBufferCount=1; si.pCommandBuffers=&cb;
  { std::lock_guard<std::mutex> l(G.Mutex); CHECK(vkQueueSubmit(G.Queue,1,&si,f)); }
  CHECK(vkWaitForFences(G.Dev,1,&f,VK_TRUE,~0ull));
  vkDestroyFence(G.Dev,f,0); vkFreeCommandBuffers(G.Dev,G.ImmPool,1,&cb);
}

VkcImg *makeImage(int w,int h,int mips,VkImageUsageFlags usage)
{
  VkcImg *t=new VkcImg; t->W=w; t->H=h; t->Mips=mips; t->RT=false; t->Layout=VK_IMAGE_LAYOUT_UNDEFINED;
  VkImageCreateInfo ci={VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO}; ci.imageType=VK_IMAGE_TYPE_2D; ci.format=VK_FORMAT_B8G8R8A8_UNORM;
  ci.extent.width=w; ci.extent.height=h; ci.extent.depth=1; ci.mipLevels=mips; ci.arrayLayers=1; ci.samples=VK_SAMPLE_COUNT_1_BIT;
  ci.tiling=VK_IMAGE_TILING_OPTIMAL; ci.usage=usage; ci.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED;
  CHECK(vkCreateImage(G.Dev,&ci,0,&t->Image));
  VkMemoryRequirements mr; vkGetImageMemoryRequirements(G.Dev,t->Image,&mr);
  VkMemoryAllocateInfo ai={VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO}; ai.allocationSize=mr.size; ai.memoryTypeIndex=memType(mr.memoryTypeBits,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  CHECK(vkAllocateMemory(G.Dev,&ai,0,&t->Mem)); CHECK(vkBindImageMemory(G.Dev,t->Image,t->Mem,0));
  VkImageViewCreateInfo vi={VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO}; vi.image=t->Image; vi.viewType=VK_IMAGE_VIEW_TYPE_2D; vi.format=ci.format;
  vi.subresourceRange.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT; vi.subresourceRange.levelCount=mips; vi.subresourceRange.layerCount=1;
  CHECK(vkCreateImageView(G.Dev,&vi,0,&t->View));
  return t;
}
void freeImage(VkcImg *t)
{
  vkDestroyImageView(G.Dev,t->View,0); vkDestroyImage(G.Dev,t->Image,0); vkFreeMemory(G.Dev,t->Mem,0);
}

// ---------------------------------------------------------------- png writer (stored deflate)
uint32_t crcTab[256]; bool crcInit=false;
uint32_t crc(uint32_t c,const uint8_t *p,size_t n)
{
  if(!crcInit){ for(uint32_t i=0;i<256;i++){ uint32_t x=i; for(int k=0;k<8;k++) x=(x&1)?0xedb88320u^(x>>1):x>>1; crcTab[i]=x; } crcInit=true; }
  c=~c; while(n--) c=crcTab[(c^*p++)&255]^(c>>8); return ~c;
}
void be32(std::vector<uint8_t>&v,uint32_t x){ v.push_back(x>>24); v.push_back(x>>16); v.push_back(x>>8); v.push_back(x); }
void chunk(FILE *f,const char *type,const std::vector<uint8_t>&d)
{
  std::vector<uint8_t> h; be32(h,(uint32_t)d.size()); fwrite(h.data(),1,4,f); fwrite(type,1,4,f);
  uint32_t c=crc(0,(const uint8_t*)type,4); if(d.size()){ fwrite(d.data(),1,d.size(),f); c=crc(c^0,d.data(),d.size()); }
  // crc over type+data: recompute properly
  std::vector<uint8_t> all(type,type+4); all.insert(all.end(),d.begin(),d.end());
  c=crc(0,all.data(),all.size()); std::vector<uint8_t> t; be32(t,c); fwrite(t.data(),1,4,f);
}
bool writePNG(const char *path,const uint8_t *bgra,int w,int h)
{
  FILE *f=fopen(path,"wb"); if(!f) return false;
  const uint8_t sig[8]={137,80,78,71,13,10,26,10}; fwrite(sig,1,8,f);
  std::vector<uint8_t> ihdr; be32(ihdr,w); be32(ihdr,h); ihdr.push_back(8); ihdr.push_back(2); ihdr.push_back(0); ihdr.push_back(0); ihdr.push_back(0);
  chunk(f,"IHDR",ihdr);
  std::vector<uint8_t> raw; raw.reserve((size_t)(w*3+1)*h);
  for(int y=0;y<h;y++){ raw.push_back(0); const uint8_t *s=bgra+(size_t)y*w*4; for(int x=0;x<w;x++){ raw.push_back(s[2]); raw.push_back(s[1]); raw.push_back(s[0]); s+=4; } }
  std::vector<uint8_t> z; z.push_back(0x78); z.push_back(0x01);
  size_t pos=0; uint32_t a=1,b=0;
  while(pos<raw.size()||pos==0){ size_t n=std::min<size_t>(65535,raw.size()-pos); bool last=pos+n>=raw.size();
    z.push_back(last?1:0); z.push_back(n&255); z.push_back(n>>8); z.push_back(~n&255); z.push_back((~n>>8)&255);
    z.insert(z.end(),raw.begin()+pos,raw.begin()+pos+n); pos+=n; if(last) break; }
  for(size_t i=0;i<raw.size();i++){ a=(a+raw[i])%65521; b=(b+a)%65521; }
  be32(z,(b<<16)|a); chunk(f,"IDAT",z); chunk(f,"IEND",std::vector<uint8_t>()); fclose(f); return true;
}

// ---------------------------------------------------------------- shaders / pipelines
VkShaderModule loadModule(const std::string &name)
{
  auto it=G.Modules.find(name); if(it!=G.Modules.end()) return it->second;
  std::string path=std::string(G.Cfg.ShaderDir)+"/"+name+".spv";
  FILE *f=fopen(path.c_str(),"rb"); if(!f){ fprintf(stderr,"[vk] cannot open shader %s\n",path.c_str()); abort(); }
  fseek(f,0,SEEK_END); long n=ftell(f); fseek(f,0,SEEK_SET); std::vector<uint32_t> d((n+3)/4); fread(d.data(),1,n,f); fclose(f);
  VkShaderModuleCreateInfo ci={VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO}; ci.codeSize=n; ci.pCode=d.data();
  VkShaderModule m; CHECK(vkCreateShaderModule(G.Dev,&ci,0,&m)); G.Modules[name]=m; return m;
}

VkBlendFactor bf(uint32_t d3d)
{
  switch(d3d){ case 1: return VK_BLEND_FACTOR_ZERO; case 2: return VK_BLEND_FACTOR_ONE; case 3: return VK_BLEND_FACTOR_SRC_COLOR;
    case 4: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR; case 5: return VK_BLEND_FACTOR_SRC_ALPHA; case 6: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case 7: return VK_BLEND_FACTOR_DST_ALPHA; case 8: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA; case 9: return VK_BLEND_FACTOR_DST_COLOR;
    case 10: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR; case 11: return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
    case 14: return VK_BLEND_FACTOR_CONSTANT_COLOR; case 15: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR; }
  return VK_BLEND_FACTOR_ONE;
}
VkBlendOp bo(uint32_t d3d)
{ switch(d3d){ case 2: return VK_BLEND_OP_SUBTRACT; case 3: return VK_BLEND_OP_REVERSE_SUBTRACT; case 4: return VK_BLEND_OP_MIN; case 5: return VK_BLEND_OP_MAX; } return VK_BLEND_OP_ADD; }

VkPipeline getPipeline(const VkcPipeKey &k)
{
  std::string key((const char*)&k,sizeof(k));
  auto it=G.Pipes.find(key); if(it!=G.Pipes.end()) return it->second;
  static const char *vsn[]={"simple.vsMain","msg.vsPlain","msg.vsGrowable","ipp.vsMain"};
  static const char *psn[]={"simple.psMain","msg.psPlain","msg.psGrowable","ipp.psMain"};
  VkPipelineShaderStageCreateInfo st[2]={{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO},{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO}};
  st[0].stage=VK_SHADER_STAGE_VERTEX_BIT; st[0].module=loadModule(vsn[k.Kind]); st[0].pName="main";
  st[1].stage=VK_SHADER_STAGE_FRAGMENT_BIT; st[1].module=loadModule(psn[k.Kind]); st[1].pName="main";
  // specialization constants (ids: see the .slang files)
  uint32_t sv[5]={0,0,0,0,0}; VkSpecializationMapEntry me[5]; uint32_t mc=0;
  if(k.Kind==0){ sv[0]=(k.Perm&1)!=0; sv[1]=(k.Perm&2)!=0; sv[2]=(k.Perm&4)!=0; mc=3; }
  else if(k.Kind==1||k.Kind==2){ sv[0]=(uint32_t)k.AlphaRef; mc=1; }
  else { sv[0]=(k.Perm&1)!=0; sv[1]=(k.Perm&2)!=0; sv[2]=(k.Perm&4)!=0; sv[3]=(k.Perm&8)!=0; sv[4]=(k.Perm>>8)&15; mc=5; }
  for(uint32_t i=0;i<mc;i++){ me[i].constantID=i; me[i].offset=i*4; me[i].size=4; }
  VkSpecializationInfo spec={mc,me,mc*4,sv}; st[0].pSpecializationInfo=&spec; st[1].pSpecializationInfo=&spec;

  VkVertexInputBindingDescription vb={0,k.Stride,VK_VERTEX_INPUT_RATE_VERTEX};
  VkVertexInputAttributeDescription va[VKC_MAXATTR];
  static const VkFormat fm[]={VK_FORMAT_R32G32_SFLOAT,VK_FORMAT_R32G32B32_SFLOAT,VK_FORMAT_R32G32B32A32_SFLOAT,VK_FORMAT_B8G8R8A8_UNORM};
  for(uint32_t i=0;i<k.AttrCount;i++){ va[i].location=k.Attr[i].Location; va[i].binding=0; va[i].format=fm[k.Attr[i].Format]; va[i].offset=k.Attr[i].Offset; }
  VkPipelineVertexInputStateCreateInfo vi={VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
  vi.vertexBindingDescriptionCount=1; vi.pVertexBindingDescriptions=&vb; vi.vertexAttributeDescriptionCount=k.AttrCount; vi.pVertexAttributeDescriptions=va;
  VkPipelineInputAssemblyStateCreateInfo ia={VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
  static const VkPrimitiveTopology tp[]={VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,VK_PRIMITIVE_TOPOLOGY_LINE_LIST,VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP};
  ia.topology=tp[k.Topology];
  VkPipelineViewportStateCreateInfo vp={VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO}; vp.viewportCount=1; vp.scissorCount=1;
  VkPipelineRasterizationStateCreateInfo rs={VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
  rs.polygonMode=VK_POLYGON_MODE_FILL; rs.lineWidth=1.0f;
  rs.cullMode=k.Cull==0?VK_CULL_MODE_NONE:(k.Cull==1?VK_CULL_MODE_BACK_BIT:VK_CULL_MODE_FRONT_BIT);
  rs.frontFace=VK_FRONT_FACE_CLOCKWISE;   // D3DCULL_CCW culls counter-clockwise (screen space, y down)
  VkPipelineMultisampleStateCreateInfo ms={VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO}; ms.rasterizationSamples=VK_SAMPLE_COUNT_1_BIT;
  VkPipelineDepthStencilStateCreateInfo ds={VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
  if(k.HasDepth){ ds.depthTestEnable=(k.DepthTest||k.DepthWrite); ds.depthWriteEnable=k.DepthWrite; ds.depthCompareOp=(VkCompareOp)(k.DepthTest?k.DepthFunc:7); }
  VkPipelineColorBlendAttachmentState cba={};
  cba.blendEnable=k.BlendOn; cba.srcColorBlendFactor=bf(k.SrcC); cba.dstColorBlendFactor=bf(k.DstC); cba.colorBlendOp=bo(k.OpC);
  cba.srcAlphaBlendFactor=bf(k.SrcA); cba.dstAlphaBlendFactor=bf(k.DstA); cba.alphaBlendOp=bo(k.OpA); cba.colorWriteMask=k.ColorMask;
  VkPipelineColorBlendStateCreateInfo cb={VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO}; cb.attachmentCount=1; cb.pAttachments=&cba;
  VkDynamicState dyn[]={VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR,VK_DYNAMIC_STATE_BLEND_CONSTANTS};
  VkPipelineDynamicStateCreateInfo dy={VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO}; dy.dynamicStateCount=3; dy.pDynamicStates=dyn;
  VkFormat colorFmt=VK_FORMAT_B8G8R8A8_UNORM;
  VkPipelineRenderingCreateInfo ri={VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO}; ri.colorAttachmentCount=1; ri.pColorAttachmentFormats=&colorFmt;
  ri.depthAttachmentFormat=k.HasDepth?VK_FORMAT_D32_SFLOAT:VK_FORMAT_UNDEFINED;
  VkGraphicsPipelineCreateInfo pi={VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO}; pi.pNext=&ri; pi.stageCount=2; pi.pStages=st;
  pi.pVertexInputState=&vi; pi.pInputAssemblyState=&ia; pi.pViewportState=&vp; pi.pRasterizationState=&rs; pi.pMultisampleState=&ms;
  pi.pDepthStencilState=&ds; pi.pColorBlendState=&cb; pi.pDynamicState=&dy; pi.layout=G.PL;
  VkPipeline p; CHECK(vkCreateGraphicsPipelines(G.Dev,VK_NULL_HANDLE,1,&pi,0,&p));
  G.Pipes[key]=p; return p;
}

VkSampler getSampler(const VkcSampler &s)
{
  std::string key((const char*)&s,sizeof(s));
  auto it=G.Samplers.find(key); if(it!=G.Samplers.end()) return it->second;
  VkSamplerCreateInfo ci={VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
  ci.magFilter=ci.minFilter=s.Filter?VK_FILTER_LINEAR:VK_FILTER_NEAREST;
  ci.mipmapMode=s.Mip?VK_SAMPLER_MIPMAP_MODE_LINEAR:VK_SAMPLER_MIPMAP_MODE_NEAREST;
  VkSamplerAddressMode am=VK_SAMPLER_ADDRESS_MODE_REPEAT; VkBorderColor bc=VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
  switch(s.Addr){ case 1: am=VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; break; case 2: am=VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT; break;
    case 3: am=VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER; break; case 4: am=VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER; bc=VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE; break; }
  ci.addressModeU=ci.addressModeV=ci.addressModeW=am; ci.borderColor=bc; ci.mipLodBias=s.LodBias;
  ci.maxLod=s.Mip?VK_LOD_CLAMP_NONE:0.0f;
  if(s.Aniso && G.Props.limits.maxSamplerAnisotropy>1){ ci.anisotropyEnable=VK_TRUE; ci.maxAnisotropy=std::min(8.0f,G.Props.limits.maxSamplerAnisotropy); }
  VkSampler r; CHECK(vkCreateSampler(G.Dev,&ci,0,&r)); G.Samplers[key]=r; return r;
}

VkDescriptorSet allocSet()
{
  for(;;){
    if(G.DPoolCount<G.DPools.size()||true){
      if(G.DPoolUsed<G.DPools.size()){
        VkDescriptorSetAllocateInfo ai={VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO}; ai.descriptorPool=G.DPools[G.DPoolUsed]; ai.descriptorSetCount=1; ai.pSetLayouts=&G.DSL;
        VkDescriptorSet s; VkResult r=vkAllocateDescriptorSets(G.Dev,&ai,&s); if(r==VK_SUCCESS) return s;
        G.DPoolUsed++; continue;
      }
      VkDescriptorPoolSize ps={VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,4*4096};
      VkDescriptorPoolCreateInfo pi={VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO}; pi.maxSets=4096; pi.poolSizeCount=1; pi.pPoolSizes=&ps;
      VkDescriptorPool p; CHECK(vkCreateDescriptorPool(G.Dev,&pi,0,&p)); G.DPools.push_back(p);
    }
  }
}

// ---------------------------------------------------------------- swapchain
void destroySwap()
{
  if(!G.Swap) return;
  vkDeviceWaitIdle(G.Dev);
  for(auto s:G.RenderDone) vkDestroySemaphore(G.Dev,s,0); G.RenderDone.clear();
  vkDestroySwapchainKHR(G.Dev,G.Swap,0); G.Swap=VK_NULL_HANDLE; G.SwapImages.clear();
}
void createSwap()
{
  destroySwap();
  VkSurfaceCapabilitiesKHR caps; CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(G.Phys,G.Surface,&caps));
  int pw=0,ph=0; SDL_GetWindowSizeInPixels(G.Cfg.Window,&pw,&ph);
  VkExtent2D e=caps.currentExtent;
  if(e.width==0xffffffffu){ e.width=std::max<uint32_t>(caps.minImageExtent.width,std::min<uint32_t>(caps.maxImageExtent.width,pw)); e.height=std::max<uint32_t>(caps.minImageExtent.height,std::min<uint32_t>(caps.maxImageExtent.height,ph)); }
  G.SwapValid=(e.width>0&&e.height>0); if(!G.SwapValid) return;
  uint32_t n; vkGetPhysicalDeviceSurfaceFormatsKHR(G.Phys,G.Surface,&n,0); std::vector<VkSurfaceFormatKHR> fs(n); vkGetPhysicalDeviceSurfaceFormatsKHR(G.Phys,G.Surface,&n,fs.data());
  VkSurfaceFormatKHR sf=fs[0]; for(auto&f:fs) if(f.format==VK_FORMAT_B8G8R8A8_UNORM){ sf=f; break; }
  vkGetPhysicalDeviceSurfacePresentModesKHR(G.Phys,G.Surface,&n,0); std::vector<VkPresentModeKHR> pm(n); vkGetPhysicalDeviceSurfacePresentModesKHR(G.Phys,G.Surface,&n,pm.data());
  VkPresentModeKHR mode=VK_PRESENT_MODE_FIFO_KHR;
  if(!G.Cfg.Vsync){ for(auto m:pm) if(m==VK_PRESENT_MODE_IMMEDIATE_KHR) mode=m; if(mode==VK_PRESENT_MODE_FIFO_KHR) for(auto m:pm) if(m==VK_PRESENT_MODE_MAILBOX_KHR) mode=m; }
  VkSwapchainCreateInfoKHR ci={VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR}; ci.surface=G.Surface;
  ci.minImageCount=std::max(caps.minImageCount,2u); if(caps.maxImageCount&&ci.minImageCount>caps.maxImageCount) ci.minImageCount=caps.maxImageCount;
  ci.imageFormat=sf.format; ci.imageColorSpace=sf.colorSpace; ci.imageExtent=e; ci.imageArrayLayers=1;
  ci.imageUsage=VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; ci.imageSharingMode=VK_SHARING_MODE_EXCLUSIVE;
  ci.preTransform=caps.currentTransform; ci.compositeAlpha=VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; ci.presentMode=mode; ci.clipped=VK_TRUE;
  CHECK(vkCreateSwapchainKHR(G.Dev,&ci,0,&G.Swap)); G.SwapFormat=sf.format; G.SwapExtent=e;
  vkGetSwapchainImagesKHR(G.Dev,G.Swap,&n,0); G.SwapImages.resize(n); vkGetSwapchainImagesKHR(G.Dev,G.Swap,&n,G.SwapImages.data());
  VkSemaphoreCreateInfo si={VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
  for(uint32_t i=0;i<n;i++){ VkSemaphore s; CHECK(vkCreateSemaphore(G.Dev,&si,0,&s)); G.RenderDone.push_back(s); }
}

// ---------------------------------------------------------------- rendering state
void endRendering()
{ if(G.Rendering){ vkCmdEndRendering(G.Cmd); G.Rendering=false; } }

void targetSize(int &w,int &h){ if(G.TargetIsScreen){ w=G.Screen.W; h=G.Screen.H; } else { w=G.Target->W; h=G.Target->H; } }

void ensureRendering()
{
  if(G.Rendering) return;
  VkcImg *t=G.TargetIsScreen?&G.Screen:G.Target;
  imgBarrier(G.Cmd,t,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  int w,h; targetSize(w,h);
  VkRenderingAttachmentInfo ca={VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO}; ca.imageView=t->View; ca.imageLayout=VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  ca.loadOp=VK_ATTACHMENT_LOAD_OP_LOAD; ca.storeOp=VK_ATTACHMENT_STORE_OP_STORE;
  VkRenderingAttachmentInfo da={VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
  VkRenderingInfo ri={VK_STRUCTURE_TYPE_RENDERING_INFO}; ri.renderArea.extent.width=w; ri.renderArea.extent.height=h; ri.layerCount=1;
  ri.colorAttachmentCount=1; ri.pColorAttachments=&ca;
  if(G.TargetIsScreen){
    VkImageLayout old=G.DepthInit?VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:VK_IMAGE_LAYOUT_UNDEFINED;
    barrier(G.Cmd,G.DepthImg,VK_IMAGE_ASPECT_DEPTH_BIT,old,VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
    da.imageView=G.DepthView; da.imageLayout=VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL; da.storeOp=VK_ATTACHMENT_STORE_OP_STORE;
    da.loadOp=G.DepthInit?VK_ATTACHMENT_LOAD_OP_LOAD:VK_ATTACHMENT_LOAD_OP_CLEAR; da.clearValue.depthStencil.depth=1.0f;
    G.DepthInit=true; ri.pDepthAttachment=&da;
  }
  vkCmdBeginRendering(G.Cmd,&ri); G.Rendering=true; G.StateDirty=true;
  if(G.PendingClear){
    VkClearAttachment cl[2]; int n=0;
    if(G.PendingClear&1){ cl[n].aspectMask=VK_IMAGE_ASPECT_COLOR_BIT; cl[n].colorAttachment=0;
      uint32_t c=G.PendingColor; cl[n].clearValue.color.float32[0]=((c>>16)&255)/255.0f; cl[n].clearValue.color.float32[1]=((c>>8)&255)/255.0f;
      cl[n].clearValue.color.float32[2]=(c&255)/255.0f; cl[n].clearValue.color.float32[3]=((c>>24)&255)/255.0f; n++; }
    if((G.PendingClear&2)&&G.TargetIsScreen){ cl[n].aspectMask=VK_IMAGE_ASPECT_DEPTH_BIT; cl[n].colorAttachment=0; cl[n].clearValue.depthStencil.depth=1.0f; cl[n].clearValue.depthStencil.stencil=0; n++; }
    VkClearRect cr; cr.rect.offset.x=G.RectX0; cr.rect.offset.y=G.RectY0; cr.rect.extent.width=std::max(0,G.RectX1-G.RectX0); cr.rect.extent.height=std::max(0,G.RectY1-G.RectY0); cr.baseArrayLayer=0; cr.layerCount=1;
    if(n&&cr.rect.extent.width&&cr.rect.extent.height) vkCmdClearAttachments(G.Cmd,n,cl,1,&cr);
    G.PendingClear=0;
  }
}

void setDynamic(const float *bc)
{
  int w,h; targetSize(w,h);
#ifndef MSG_VK_FLIP_Y_VIEWPORT
#define MSG_VK_FLIP_Y_VIEWPORT 1   // the picture is rendered top-down, so the viewport carries a negative height (set 0 to disable)
#endif
#if MSG_VK_FLIP_Y_VIEWPORT
  VkViewport v={0,(float)h,(float)w,-(float)h,0,1};
#else
  VkViewport v={0,0,(float)w,(float)h,0,1};
#endif
  vkCmdSetViewport(G.Cmd,0,1,&v);
  VkRect2D sc; sc.offset.x=G.RectX0; sc.offset.y=G.RectY0; sc.extent.width=std::max(0,G.RectX1-G.RectX0); sc.extent.height=std::max(0,G.RectY1-G.RectY0); vkCmdSetScissor(G.Cmd,0,1,&sc);
  vkCmdSetBlendConstants(G.Cmd,bc);
}

} // namespace

// ================================================================= public api
bool vkc_headless(){ return G.Headless; }
void vkc_screen_size(int &w,int &h){ w=G.Screen.W; h=G.Screen.H; }
bool vkc_in_frame(){ return G.InFrame; }
uint32_t vkc_stat_draws(){ return G.Draws; }

bool vkc_init(const VkcConfig &cfg)
{
  G.Cfg=cfg; G.Headless=(cfg.Window==0); G.Chunks.reserve(256); // Chunk* must stay stable (256 x 32MB max)
  VkApplicationInfo ai={VK_STRUCTURE_TYPE_APPLICATION_INFO}; ai.pApplicationName="masagin"; ai.apiVersion=VK_API_VERSION_1_3;
  std::vector<const char*> iext;
  if(!G.Headless){ Uint32 n=0; const char *const *e=SDL_Vulkan_GetInstanceExtensions(&n); for(Uint32 i=0;i<n;i++) iext.push_back(e[i]); }
  std::vector<const char*> layers;
  if(getenv("MSG_VK_VALIDATE")) layers.push_back("VK_LAYER_KHRONOS_validation");
  VkInstanceCreateInfo ici={VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO}; ici.pApplicationInfo=&ai; ici.enabledExtensionCount=iext.size(); ici.ppEnabledExtensionNames=iext.data();
  ici.enabledLayerCount=layers.size(); ici.ppEnabledLayerNames=layers.data();
  if(vkCreateInstance(&ici,0,&G.Inst)!=VK_SUCCESS){ fprintf(stderr,"[vk] vkCreateInstance failed\n"); return false; }
  if(!G.Headless && !SDL_Vulkan_CreateSurface(cfg.Window,G.Inst,0,&G.Surface)){ fprintf(stderr,"[vk] surface: %s\n",SDL_GetError()); return false; }
  uint32_t n=0; vkEnumeratePhysicalDevices(G.Inst,&n,0); if(!n){ fprintf(stderr,"[vk] no devices\n"); return false; }
  std::vector<VkPhysicalDevice> pd(n); vkEnumeratePhysicalDevices(G.Inst,&n,pd.data());
  int want=getenv("MSG_VK_DEVICE")?atoi(getenv("MSG_VK_DEVICE")):-1; int best=-1;
  for(uint32_t i=0;i<n;i++){
    VkPhysicalDeviceProperties p; vkGetPhysicalDeviceProperties(pd[i],&p);
    uint32_t qn; vkGetPhysicalDeviceQueueFamilyProperties(pd[i],&qn,0); std::vector<VkQueueFamilyProperties> qf(qn); vkGetPhysicalDeviceQueueFamilyProperties(pd[i],&qn,qf.data());
    bool ok=false; for(uint32_t q=0;q<qn;q++){ VkBool32 pres=G.Headless; if(!G.Headless) vkGetPhysicalDeviceSurfaceSupportKHR(pd[i],q,G.Surface,&pres); if((qf[q].queueFlags&VK_QUEUE_GRAPHICS_BIT)&&pres){ ok=true; if(best<0||(int)i==want) G.Family=q; break; } }
    if(!ok) continue;
    if(want==(int)i){ best=i; break; }
    if(best<0 || (p.deviceType==VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && want<0)) best=i;
  }
  if(best<0){ fprintf(stderr,"[vk] no suitable device\n"); return false; }
  G.Phys=pd[best];
  { uint32_t qn; vkGetPhysicalDeviceQueueFamilyProperties(G.Phys,&qn,0); std::vector<VkQueueFamilyProperties> qf(qn); vkGetPhysicalDeviceQueueFamilyProperties(G.Phys,&qn,qf.data());
    for(uint32_t q=0;q<qn;q++){ VkBool32 pres=G.Headless; if(!G.Headless) vkGetPhysicalDeviceSurfaceSupportKHR(G.Phys,q,G.Surface,&pres); if((qf[q].queueFlags&VK_QUEUE_GRAPHICS_BIT)&&pres){ G.Family=q; break; } } }
  vkGetPhysicalDeviceProperties(G.Phys,&G.Props); vkGetPhysicalDeviceMemoryProperties(G.Phys,&G.MemProps);
  fprintf(stderr,"[vk] device: %s\n",G.Props.deviceName);
  float prio=1; VkDeviceQueueCreateInfo qi={VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO}; qi.queueFamilyIndex=G.Family; qi.queueCount=1; qi.pQueuePriorities=&prio;
  VkPhysicalDeviceVulkan13Features f13={VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES}; f13.dynamicRendering=VK_TRUE; f13.synchronization2=VK_TRUE;
  VkPhysicalDeviceFeatures2 f2={VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2}; f2.pNext=&f13;
  { VkPhysicalDeviceFeatures sup; vkGetPhysicalDeviceFeatures(G.Phys,&sup); f2.features.samplerAnisotropy=sup.samplerAnisotropy; }
  std::vector<const char*> dext; if(!G.Headless) dext.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  VkDeviceCreateInfo di={VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO}; di.pNext=&f2; di.queueCreateInfoCount=1; di.pQueueCreateInfos=&qi; di.enabledExtensionCount=dext.size(); di.ppEnabledExtensionNames=dext.data();
  if(vkCreateDevice(G.Phys,&di,0,&G.Dev)!=VK_SUCCESS){ fprintf(stderr,"[vk] vkCreateDevice failed (need Vulkan 1.3)\n"); return false; }
  vkGetDeviceQueue(G.Dev,G.Family,0,&G.Queue);
  VkCommandPoolCreateInfo pci={VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO}; pci.queueFamilyIndex=G.Family; pci.flags=VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  CHECK(vkCreateCommandPool(G.Dev,&pci,0,&G.Pool)); CHECK(vkCreateCommandPool(G.Dev,&pci,0,&G.ImmPool));
  VkCommandBufferAllocateInfo cai={VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO}; cai.commandPool=G.Pool; cai.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY; cai.commandBufferCount=1;
  CHECK(vkAllocateCommandBuffers(G.Dev,&cai,&G.Cmd));
  VkFenceCreateInfo fi={VK_STRUCTURE_TYPE_FENCE_CREATE_INFO}; fi.flags=VK_FENCE_CREATE_SIGNALED_BIT; CHECK(vkCreateFence(G.Dev,&fi,0,&G.Fence));
  VkSemaphoreCreateInfo si={VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO}; CHECK(vkCreateSemaphore(G.Dev,&si,0,&G.Acquire));
  // layouts
  VkDescriptorSetLayoutBinding lb[VKC_MAXTEX]; for(int i=0;i<VKC_MAXTEX;i++){ lb[i]={(uint32_t)i,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1,VK_SHADER_STAGE_FRAGMENT_BIT,0}; }
  VkDescriptorSetLayoutCreateInfo dli={VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO}; dli.bindingCount=VKC_MAXTEX; dli.pBindings=lb; CHECK(vkCreateDescriptorSetLayout(G.Dev,&dli,0,&G.DSL));
  VkPushConstantRange pcr={VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT,0,128};
  VkPipelineLayoutCreateInfo pli={VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO}; pli.setLayoutCount=1; pli.pSetLayouts=&G.DSL; pli.pushConstantRangeCount=1; pli.pPushConstantRanges=&pcr;
  CHECK(vkCreatePipelineLayout(G.Dev,&pli,0,&G.PL));
  // geometry ring + quad indices
  G.Ring.Size=128ull<<20; G.Ring.Used=0; makeBuffer(G.Ring.Size,GEOUSAGE,HOSTVIS,G.Ring.Buf,G.Ring.Mem,(void**)&G.Ring.Map);
  { uint64_t bytes=65536*6/4*2*1; void *m; makeBuffer(bytes,VK_BUFFER_USAGE_INDEX_BUFFER_BIT,HOSTVIS,G.QuadIB,G.QuadIBMem,&m);
    uint16_t *ip=(uint16_t*)m; for(int q=0;q<16384;q++){ int o=q*4; *ip++=o; *ip++=o+1; *ip++=o+2; *ip++=o; *ip++=o+2; *ip++=o+3; } }
  // screen
  G.Screen.W=cfg.Width; G.Screen.H=cfg.Height;
  { VkcImg *s=makeImage(cfg.Width,cfg.Height,1,VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_SAMPLED_BIT); G.Screen=*s; delete s; }
  { VkImageCreateInfo ci={VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO}; ci.imageType=VK_IMAGE_TYPE_2D; ci.format=VK_FORMAT_D32_SFLOAT; ci.extent.width=cfg.Width; ci.extent.height=cfg.Height; ci.extent.depth=1;
    ci.mipLevels=1; ci.arrayLayers=1; ci.samples=VK_SAMPLE_COUNT_1_BIT; ci.tiling=VK_IMAGE_TILING_OPTIMAL; ci.usage=VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT; ci.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED;
    CHECK(vkCreateImage(G.Dev,&ci,0,&G.DepthImg)); VkMemoryRequirements mr; vkGetImageMemoryRequirements(G.Dev,G.DepthImg,&mr);
    VkMemoryAllocateInfo ma={VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO}; ma.allocationSize=mr.size; ma.memoryTypeIndex=memType(mr.memoryTypeBits,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    CHECK(vkAllocateMemory(G.Dev,&ma,0,&G.DepthMem)); CHECK(vkBindImageMemory(G.Dev,G.DepthImg,G.DepthMem,0));
    VkImageViewCreateInfo vi={VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO}; vi.image=G.DepthImg; vi.viewType=VK_IMAGE_VIEW_TYPE_2D; vi.format=ci.format; vi.subresourceRange.aspectMask=VK_IMAGE_ASPECT_DEPTH_BIT; vi.subresourceRange.levelCount=1; vi.subresourceRange.layerCount=1;
    CHECK(vkCreateImageView(G.Dev,&vi,0,&G.DepthView)); G.DepthInit=false; }
  G.Black=vkc_tex_create(1,1,1,false); { uint32_t px=0xff000000; vkc_tex_upload(G.Black,0,&px,4,1,1); }
  if(!G.Headless) createSwap();
  if(G.Cfg.DumpDir){ uint64_t bytes=(uint64_t)cfg.Width*cfg.Height*4; makeBuffer(bytes,VK_BUFFER_USAGE_TRANSFER_DST_BIT,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,G.Readback,G.ReadbackMem,&G.ReadbackMap); }
  G.Ready=true; return true;
}

void vkc_exit()
{
  if(!G.Ready) return;
  vkDeviceWaitIdle(G.Dev); G.Ready=false;   // resources are leaked deliberately: the process exits right after
}

VkcBuf vkc_alloc_static(uint64_t bytes)
{
  std::lock_guard<std::mutex> l(G.Mutex);
  bytes=(bytes+15)&~15ull;
  for(auto &c:G.Chunks) if(c.Size-c.Used>=bytes){ VkcBuf b={&c,c.Used,c.Map+c.Used,bytes}; c.Used+=bytes; return b; }
  Chunk c; c.Size=std::max<uint64_t>(bytes,32ull<<20); c.Used=0; makeBuffer(c.Size,GEOUSAGE,HOSTVIS,c.Buf,c.Mem,(void**)&c.Map);
  G.Chunks.push_back(c); Chunk &r=G.Chunks.back(); VkcBuf b={&r,r.Used,r.Map,bytes}; r.Used+=bytes; return b;
}
VkcBuf vkc_alloc_frame(uint64_t bytes)
{
  std::lock_guard<std::mutex> l(G.Mutex);
  bytes=(bytes+15)&~15ull;
  if(G.Ring.Used+bytes>G.Ring.Size){ fprintf(stderr,"[vk] per-frame geometry ring exhausted\n"); abort(); }
  VkcBuf b={&G.Ring,G.Ring.Used,G.Ring.Map+G.Ring.Used,bytes}; G.Ring.Used+=bytes; return b;
}
void vkc_buf_free(VkcBuf &b){ b.Chunk=0; b.Map=0; }   // static memory is never returned (see vkc_exit)

VkcImg *vkc_tex_create(int w,int h,int mips,bool rt)
{
  VkImageUsageFlags u=VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  if(rt) u|=VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  VkcImg *t=makeImage(w,h,mips,u); t->RT=rt; return t;
}
void vkc_tex_destroy(VkcImg *t){ if(!t) return; vkDeviceWaitIdle(G.Dev); freeImage(t); delete t; }

void vkc_tex_upload(VkcImg *t,int mip,const void *data,int pitch,int w,int h)
{
  VkBuffer sb; VkDeviceMemory sm; void *map; uint64_t bytes=(uint64_t)w*h*4;
  makeBuffer(bytes,VK_BUFFER_USAGE_TRANSFER_SRC_BIT,HOSTVIS,sb,sm,&map);
  for(int y=0;y<h;y++) memcpy((uint8_t*)map+(size_t)y*w*4,(const uint8_t*)data+(size_t)y*pitch,(size_t)w*4);
  VkCommandBuffer cb=immBegin();
  VkImageLayout old=t->Layout; barrier(cb,t->Image,VK_IMAGE_ASPECT_COLOR_BIT,old,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,t->Mips);
  VkBufferImageCopy r={}; r.imageSubresource.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT; r.imageSubresource.mipLevel=mip; r.imageSubresource.layerCount=1; r.imageExtent.width=w; r.imageExtent.height=h; r.imageExtent.depth=1;
  vkCmdCopyBufferToImage(cb,sb,t->Image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1,&r);
  barrier(cb,t->Image,VK_IMAGE_ASPECT_COLOR_BIT,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,t->Mips);
  immEnd(cb); t->Layout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  vkDestroyBuffer(G.Dev,sb,0); vkFreeMemory(G.Dev,sm,0);
}

void vkc_begin_frame()
{
  CHECK(vkWaitForFences(G.Dev,1,&G.Fence,VK_TRUE,~0ull)); CHECK(vkResetFences(G.Dev,1,&G.Fence));
  G.Ring.Used=0; for(auto p:G.DPools) vkResetDescriptorPool(G.Dev,p,0); G.DPoolUsed=0;
  G.SwapIndex=~0u;
  if(!G.Headless){
    if(!G.Swap) createSwap();
    if(G.SwapValid){
      VkResult r=vkAcquireNextImageKHR(G.Dev,G.Swap,~0ull,G.Acquire,VK_NULL_HANDLE,&G.SwapIndex);
      if(r==VK_ERROR_OUT_OF_DATE_KHR){ createSwap(); if(G.SwapValid) r=vkAcquireNextImageKHR(G.Dev,G.Swap,~0ull,G.Acquire,VK_NULL_HANDLE,&G.SwapIndex); }
      if(r!=VK_SUCCESS&&r!=VK_SUBOPTIMAL_KHR) G.SwapIndex=~0u;
    }
  }
  CHECK(vkResetCommandBuffer(G.Cmd,0));
  VkCommandBufferBeginInfo bi={VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO}; bi.flags=VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; CHECK(vkBeginCommandBuffer(G.Cmd,&bi));
  if(VkDebug()){ if(!G.QP){ VkQueryPoolCreateInfo qi={VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO}; qi.queryType=VK_QUERY_TYPE_OCCLUSION; qi.queryCount=64; vkCreateQueryPool(G.Dev,&qi,0,&G.QP); } vkCmdResetQueryPool(G.Cmd,G.QP,0,64); G.QN=0; }
  G.InFrame=true; G.Rendering=false; G.Target=0; G.TargetIsScreen=true; G.RectX0=G.RectY0=0; G.RectX1=G.Screen.W; G.RectY1=G.Screen.H;
  G.PendingClear=0; G.StateDirty=true; G.BoundPipe=VK_NULL_HANDLE; G.SetValid=false; G.BoundVB=G.BoundIB=VK_NULL_HANDLE; G.Draws=0;
  G.DumpThisFrame=G.Cfg.DumpDir && (int)G.Frame>=G.Cfg.DumpStart && G.Cfg.DumpEvery>0 && (((int)G.Frame-G.Cfg.DumpStart)%G.Cfg.DumpEvery)==0;
}

void vkc_end_frame()
{
  if(!G.InFrame) return;
  endRendering();
  imgBarrier(G.Cmd,&G.Screen,VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  if(G.DumpThisFrame){
    VkBufferImageCopy r={}; r.imageSubresource.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT; r.imageSubresource.layerCount=1; r.imageExtent.width=G.Screen.W; r.imageExtent.height=G.Screen.H; r.imageExtent.depth=1;
    vkCmdCopyImageToBuffer(G.Cmd,G.Screen.Image,VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,G.Readback,1,&r);
  }
  bool present=(G.SwapIndex!=~0u);
  if(present){
    VkImage si=G.SwapImages[G.SwapIndex];
    barrier(G.Cmd,si,VK_IMAGE_ASPECT_COLOR_BIT,VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    VkImageBlit b={}; b.srcSubresource.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT; b.srcSubresource.layerCount=1; b.srcOffsets[1]={G.Screen.W,G.Screen.H,1};
    b.dstSubresource=b.srcSubresource; b.dstOffsets[1]={(int)G.SwapExtent.width,(int)G.SwapExtent.height,1};
    vkCmdBlitImage(G.Cmd,G.Screen.Image,VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,si,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1,&b,VK_FILTER_LINEAR);
    barrier(G.Cmd,si,VK_IMAGE_ASPECT_COLOR_BIT,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
  }
  CHECK(vkEndCommandBuffer(G.Cmd));
  VkPipelineStageFlags ws=VK_PIPELINE_STAGE_TRANSFER_BIT;
  VkSubmitInfo si={VK_STRUCTURE_TYPE_SUBMIT_INFO}; si.commandBufferCount=1; si.pCommandBuffers=&G.Cmd;
  if(present){ si.waitSemaphoreCount=1; si.pWaitSemaphores=&G.Acquire; si.pWaitDstStageMask=&ws; si.signalSemaphoreCount=1; si.pSignalSemaphores=&G.RenderDone[G.SwapIndex]; }
  { std::lock_guard<std::mutex> l(G.Mutex); CHECK(vkQueueSubmit(G.Queue,1,&si,G.Fence)); }
  if(present){
    VkPresentInfoKHR pi={VK_STRUCTURE_TYPE_PRESENT_INFO_KHR}; pi.waitSemaphoreCount=1; pi.pWaitSemaphores=&G.RenderDone[G.SwapIndex]; pi.swapchainCount=1; pi.pSwapchains=&G.Swap; pi.pImageIndices=&G.SwapIndex;
    std::lock_guard<std::mutex> l(G.Mutex); VkResult r=vkQueuePresentKHR(G.Queue,&pi); if(r==VK_ERROR_OUT_OF_DATE_KHR||r==VK_SUBOPTIMAL_KHR) G.Swap?createSwap():(void)0;
  }
  CHECK(vkWaitForFences(G.Dev,1,&G.Fence,VK_TRUE,~0ull));
  if(G.DumpThisFrame){ char path[1024]; snprintf(path,sizeof(path),"%s/frame_%05u.png",G.Cfg.DumpDir,G.Frame); if(writePNG(path,(const uint8_t*)G.ReadbackMap,G.Screen.W,G.Screen.H)) fprintf(stderr,"[vk] wrote %s\n",path); else fprintf(stderr,"[vk] cannot write %s (does the directory exist?)\n",path); }
  if(VkDebug()&&G.QN){ uint64_t r[64]; vkGetQueryPoolResults(G.Dev,G.QP,0,G.QN,sizeof(r),r,8,VK_QUERY_RESULT_64_BIT|VK_QUERY_RESULT_WAIT_BIT); for(int i=0;i<G.QN;i++) fprintf(stderr,"[vk] draw %d samples %llu\n",i,(unsigned long long)r[i]); }
  if(VkDebug()) fprintf(stderr,"[vk] frame %u: %u draws, %zu pipelines\n",G.Frame,G.Draws,G.Pipes.size());
  G.Screen.Layout=VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  G.Frame++; G.InFrame=false;
}

void vkc_set_target(VkcImg *target,int clearflags,uint32_t clearcolor,const int *rect)
{
  if(!G.InFrame) return;
  if(VkDebug()) fprintf(stderr,"[vkc_set_target] clear %d\n",clearflags);
  endRendering();
  G.Target=target; G.TargetIsScreen=(target==0);
  int w,h; targetSize(w,h);
  if(rect){ G.RectX0=std::max(0,rect[0]); G.RectY0=std::max(0,rect[1]); G.RectX1=std::min(w,rect[2]); G.RectY1=std::min(h,rect[3]); }
  else { G.RectX0=0; G.RectY0=0; G.RectX1=w; G.RectY1=h; }
  G.PendingClear=clearflags&3; G.PendingColor=clearcolor; G.StateDirty=true;
  if(G.PendingClear) ensureRendering();
}

void vkc_grab(VkcImg *dst,int filter)
{
  if(!G.InFrame||!dst) return;
  endRendering();
  imgBarrier(G.Cmd,&G.Screen,VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  imgBarrier(G.Cmd,dst,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  VkImageBlit b={}; b.srcSubresource.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT; b.srcSubresource.layerCount=1; b.srcOffsets[1]={G.Screen.W,G.Screen.H,1};
  b.dstSubresource=b.srcSubresource; b.dstOffsets[1]={dst->W,dst->H,1};
  vkCmdBlitImage(G.Cmd,G.Screen.Image,VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,dst->Image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1,&b,filter?VK_FILTER_LINEAR:VK_FILTER_NEAREST);
  imgBarrier(G.Cmd,dst,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  G.StateDirty=true;
}

void vkc_draw(const VkcDraw &d)
{
  if(!G.InFrame) return;
  // textures must be in shader-read layout; transitions happen outside of a render pass
  VkcImg *tex[VKC_MAXTEX];
  for(int i=0;i<VKC_MAXTEX;i++){ tex[i]=d.Tex[i]?d.Tex[i]:G.Black; }
  bool need=false;
  for(int i=0;i<VKC_MAXTEX;i++) if(tex[i]->Layout!=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && tex[i]!=G.Target) need=true;
  if(need){ endRendering(); for(int i=0;i<VKC_MAXTEX;i++) if(tex[i]->Layout!=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && tex[i]!=G.Target) imgBarrier(G.Cmd,tex[i],VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL); }
  ensureRendering();
  VkcPipeKey k=d.Key; k.HasDepth=G.TargetIsScreen;
  VkPipeline p=getPipeline(k);
  if(VkDebug()) fprintf(stderr,"[vkc_draw] rendering %d rect %d %d %d %d idx %d first %d count %d quads %d pipe %p\n",G.Rendering,G.RectX0,G.RectY0,G.RectX1,G.RectY1,d.IndexSize,d.First,d.Count,d.Quads,(void*)p);
  if(p!=G.BoundPipe){ vkCmdBindPipeline(G.Cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,p); G.BoundPipe=p; }
  if(G.StateDirty){ setDynamic(d.BlendConst); G.StateDirty=false; } else { vkCmdSetBlendConstants(G.Cmd,d.BlendConst); }
  // descriptors
  VkImageView views[VKC_MAXTEX]; VkSampler smp[VKC_MAXTEX];
  for(int i=0;i<VKC_MAXTEX;i++){ views[i]=tex[i]->View; smp[i]=getSampler(d.Smp[i]); }
  if(!G.SetValid||memcmp(views,G.SetViews,sizeof(views))||memcmp(smp,G.SetSamplers,sizeof(smp))){
    VkDescriptorSet s=allocSet(); VkDescriptorImageInfo ii[VKC_MAXTEX]; VkWriteDescriptorSet w[VKC_MAXTEX];
    for(int i=0;i<VKC_MAXTEX;i++){ ii[i].sampler=smp[i]; ii[i].imageView=views[i]; ii[i].imageLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      w[i]={VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET}; w[i].dstSet=s; w[i].dstBinding=i; w[i].descriptorCount=1; w[i].descriptorType=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; w[i].pImageInfo=&ii[i]; }
    vkUpdateDescriptorSets(G.Dev,VKC_MAXTEX,w,0,0);
    vkCmdBindDescriptorSets(G.Cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,G.PL,0,1,&s,0,0);
    memcpy(G.SetViews,views,sizeof(views)); memcpy(G.SetSamplers,smp,sizeof(smp)); G.SetValid=true;
  }
  if(d.PushSize) vkCmdPushConstants(G.Cmd,G.PL,VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT,0,d.PushSize,d.Push);
  VkBuffer vb=((Chunk*)d.Vertex.Chunk)->Buf; uint64_t vo=d.Vertex.Offset;
  if(vb!=G.BoundVB||vo!=G.BoundVBOff){ vkCmdBindVertexBuffers(G.Cmd,0,1,&vb,&vo); G.BoundVB=vb; G.BoundVBOff=vo; }
  bool q=G.QP&&G.QN<64&&VkDebug(); if(q) vkCmdBeginQuery(G.Cmd,G.QP,G.QN,0);
  if(d.Quads){
    if(G.BoundIB!=G.QuadIB||G.BoundIBType!=2||G.BoundIBOff!=0){ vkCmdBindIndexBuffer(G.Cmd,G.QuadIB,0,VK_INDEX_TYPE_UINT16); G.BoundIB=G.QuadIB; G.BoundIBType=2; G.BoundIBOff=0; }
    vkCmdDrawIndexed(G.Cmd,d.Count/4*6,1,d.First/4*6,d.BaseVertex,0);
  } else if(d.IndexSize){
    VkBuffer ib=((Chunk*)d.Index.Chunk)->Buf; uint64_t io=d.Index.Offset;
    if(ib!=G.BoundIB||io!=G.BoundIBOff||G.BoundIBType!=d.IndexSize){ vkCmdBindIndexBuffer(G.Cmd,ib,io,d.IndexSize==2?VK_INDEX_TYPE_UINT16:VK_INDEX_TYPE_UINT32); G.BoundIB=ib; G.BoundIBOff=io; G.BoundIBType=d.IndexSize; }
    vkCmdDrawIndexed(G.Cmd,d.Count,1,d.First,d.BaseVertex,0);
  } else vkCmdDraw(G.Cmd,d.Count,1,d.First,0);
  if(q){ vkCmdEndQuery(G.Cmd,G.QP,G.QN); G.QN++; }
  G.Draws++;
}
