// vkcore: Vulkan 1.3 (dynamic rendering) renderer core.
// Deliberately independent from the Altona headers (plain C++17), so it can use the
// standard library. graphics_vk.cpp is the thin Altona-facing glue on top of it.
#ifndef VKCORE_HPP
#define VKCORE_HPP
#include <stdint.h>
#include <stddef.h>

struct SDL_Window;

enum { VKC_MAXTEX = 4, VKC_MAXATTR = 6 };

struct VkcConfig
{
  SDL_Window *Window;        // null: headless
  int Width, Height;         // size of the offscreen "screen"
  bool Vsync;
  const char *ShaderDir;     // directory with *.spv
  const char *DumpDir;       // headless: write PNGs here (may be null)
  int DumpStart, DumpEvery;  // frame numbers
};

struct VkcBuf { void *Chunk; uint64_t Offset; uint8_t *Map; uint64_t Size; };   // sub-allocation
struct VkcImg;                                                                   // texture / rendertarget

struct VkcAttr { uint8_t Location; uint8_t Format; uint16_t Offset; };           // Format: 0 f2,1 f3,2 f4,3 bgra8unorm
struct VkcSampler { uint8_t Filter;  /*0 point,1 linear*/ uint8_t Mip;  /*0 none,1 linear mips*/
                    uint8_t Addr;    /*0 wrap,1 clamp,2 mirror,3 border black,4 border white*/ uint8_t Aniso; float LodBias; };

// everything that makes a pipeline different. Must be memset(0) before filling.
struct VkcPipeKey
{
  uint32_t Kind;             // 0 simple, 1 msg, 2 msg growable, 3 ipp
  uint32_t Perm;             // permutation word (kind specific, see graphics_vk.cpp)
  int32_t  AlphaRef;         // -1 off
  uint32_t Topology;         // 0 tri list, 1 line list, 2 line strip, 3 tri strip
  uint32_t Stride;
  uint32_t AttrCount;
  VkcAttr  Attr[VKC_MAXATTR];
  uint32_t DepthTest, DepthWrite, DepthFunc;
  uint32_t Cull;             // 0 none, 1 cull back-facing, 2 cull front-facing
  uint32_t BlendOn, SrcC, DstC, OpC, SrcA, DstA, OpA;   // decoded from the packed sMB_* flags, see graphics_vk.cpp
  uint32_t ColorMask;        // bit0 r,1 g,2 b,3 a
  uint32_t HasDepth;         // target has a depth buffer
};

bool  vkc_init(const VkcConfig &cfg);
void  vkc_exit();
bool  vkc_headless();
void  vkc_screen_size(int &w,int &h);

VkcBuf vkc_alloc_static(uint64_t bytes);          // persistent (host visible)
VkcBuf vkc_alloc_frame(uint64_t bytes);           // valid until the frame has been executed
void   vkc_buf_free(VkcBuf &b);

VkcImg *vkc_tex_create(int w,int h,int mips,bool rendertarget);
void    vkc_tex_destroy(VkcImg *t);
void    vkc_tex_upload(VkcImg *t,int mip,const void *data,int pitch,int w,int h);

void vkc_begin_frame();
void vkc_end_frame();
bool vkc_in_frame();

// target==0: the screen. clear: bit0 colour, bit1 depth. rect: viewrect (scissor), null = whole target
void vkc_set_target(VkcImg *target,int clearflags,uint32_t clearcolor,const int *rect);
void vkc_grab(VkcImg *dst,int filter /*0 nearest,1 linear*/);

struct VkcDraw
{
  VkcPipeKey Key;
  const void *Push; uint32_t PushSize;
  VkcImg *Tex[VKC_MAXTEX]; VkcSampler Smp[VKC_MAXTEX];
  float BlendConst[4];
  VkcBuf Vertex; VkcBuf Index; int IndexSize;       // IndexSize 0: not indexed, 2/4 bytes
  int First, Count;                                 // in indices if indexed else vertices
  int BaseVertex;
  bool Quads;                                       // vertex stream holds quads, use the shared quad index buffer
};
void vkc_draw(const VkcDraw &d);

uint32_t vkc_stat_draws();
#endif
