/****************************************************************************/
/***  Vulkan backend. sMaterial, sGeometry, sTexture2D and render targets  ***/
/***  are mapped onto vkcore; the device-independent Altona code in       ***/
/***  graphics.cpp is used as it is.                                      ***/
/****************************************************************************/

#include "base/types.hpp"
#if sRENDERER == sRENDER_BLANK

#include "base/system.hpp"
#include "base/math.hpp"
#include "base/graphics.hpp"
#include "base/windows.hpp"
#include "util/shaders.hpp"
#include "base/vkcore.hpp"
#include "base/sdl_shim.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void sInitGfxCommon();
void sExitGfxCommon();
extern sRect sGFXViewRect;
extern sF32 sGFXRendertargetAspect;

// set by the platform layer (platform_sdl.cpp)
extern void *sSDLWindow;
extern sInt sVkVsync;
extern const char *sVkShaderDir;
extern const char *sVkDumpDir;
extern sInt sVkDumpStart, sVkDumpEvery;
extern sInt sSDLDesktopW, sSDLDesktopH;

static sScreenMode DXScreenMode;
static bool VkDebug(){ static int v=-1; if(v<0) v=getenv("MSG_VK_DEBUG")?1:0; return v!=0; }

/****************************************************************************/
/***  side tables (pointer -> backend object); tiny, so linear search     ***/
/****************************************************************************/

struct PermEntry { const sMaterial *M; sU32 Perm; };
static PermEntry PermTab[2048];
static sInt PermCount;
static sThreadLock *TabLock;
static sThreadLock &TL() { if(!TabLock) TabLock = new sThreadLock; return *TabLock; }

static sU32 GetPerm(const sMaterial *m)
{
  for(sInt i=PermCount-1;i>=0;i--) if(PermTab[i].M==m) return PermTab[i].Perm;
  return 0;
}
static void SetPerm(const sMaterial *m,sU32 p)
{
  sScopeLock l(&TL());
  for(sInt i=0;i<PermCount;i++) if(PermTab[i].M==m) { PermTab[i].Perm = p; return; }
  sVERIFY(PermCount<2048);
  PermTab[PermCount].M = m; PermTab[PermCount].Perm = p; PermCount++;
}
static void DropPerm(const sMaterial *m)
{
  sScopeLock l(&TL());
  for(sInt i=0;i<PermCount;i++) if(PermTab[i].M==m) { PermTab[i] = PermTab[--PermCount]; return; }
}

struct TexEntry { const sTextureBase *T; VkcImg *Img; };
static TexEntry TexTab[1024];
static sInt TexCount;

static VkcImg *GetImg(const sTextureBase *t)
{
  for(sInt i=0;i<TexCount;i++) if(TexTab[i].T==t) return TexTab[i].Img;
  return 0;
}
static void AddImg(const sTextureBase *t,VkcImg *img)
{
  sScopeLock l(&TL());
  sVERIFY(TexCount<1024);
  TexTab[TexCount].T = t; TexTab[TexCount].Img = img; TexCount++;
}
static VkcImg *DropImg(const sTextureBase *t)
{
  sScopeLock l(&TL());
  for(sInt i=0;i<TexCount;i++) if(TexTab[i].T==t) { VkcImg *r = TexTab[i].Img; TexTab[i] = TexTab[--TexCount]; return r; }
  return 0;
}

/****************************************************************************/
/***  Vertex formats                                                      ***/
/****************************************************************************/

void sVertexFormatHandle::Create()
{
  sInt i,b[sVF_STREAMMAX];
  sInt stream;

  for(i=0;i<sVF_STREAMMAX;i++)
    b[i] = 0;

  i = 0;
  while(Data[i])
  {
    stream = (Data[i]&sVF_STREAMMASK)>>sVF_STREAMSHIFT;
    sVERIFY(i<31);
    switch(Data[i]&sVF_TYPEMASK)
    {
    case sVF_F2:  b[stream]+=2*4;  break;
    case sVF_F3:  b[stream]+=3*4;  break;
    case sVF_F4:  b[stream]+=4*4;  break;
    case sVF_I4:  b[stream]+=1*4;  break;
    case sVF_C4:  b[stream]+=1*4;  break;
    case sVF_S2:  b[stream]+=1*4;  break;
    case sVF_S4:  b[stream]+=2*4;  break;
    case sVF_H2:  b[stream]+=1*4;  break;
    case sVF_H4:  b[stream]+=2*4;  break;
    case sVF_F1:  b[stream]+=1*4;  break;
    default: sVERIFYFALSE;
    }
    AvailMask |= 1 << (Data[i]&sVF_USEMASK);
    Streams = sMax(Streams,stream+1);
    i++;
  }
  for(sInt i=0;i<sVF_STREAMMAX;i++)
    VertexSize[i] = b[i];
}

void sVertexFormatHandle::Destroy() {}

// translate a vertex format into vkcore attributes (stream 0 only; the demo uses nothing else)
static void BuildAttrs(sVertexFormatHandle *f,VkcPipeKey &k)
{
  const sU32 *d = f->GetDesc();
  sInt off = 0;
  k.AttrCount = 0;
  k.Stride = f->GetSize(0);
  for(sInt i=0;d[i];i++)
  {
    sInt stream = (d[i]&sVF_STREAMMASK)>>sVF_STREAMSHIFT;
    sInt use = d[i]&sVF_USEMASK;
    sInt size = 0, fmt = -1;
    switch(d[i]&sVF_TYPEMASK)
    {
    case sVF_F2: size = 8;  fmt = 0; break;
    case sVF_F3: size = 12; fmt = 1; break;
    case sVF_F4: size = 16; fmt = 2; break;
    case sVF_C4: size = 4;  fmt = 3; break;
    case sVF_I4: case sVF_S2: case sVF_H2: case sVF_F1: size = 4; break;
    case sVF_S4: case sVF_H4: size = 8; break;
    }
    sInt loc = -1;
    switch(use) { case sVF_POSITION: loc = 0; break; case sVF_COLOR0: loc = 1; break; case sVF_UV0: loc = 2; break; case sVF_UV1: loc = 3; break; }
    if(stream==0 && loc>=0 && fmt>=0 && k.AttrCount<VKC_MAXATTR)
    {
      k.Attr[k.AttrCount].Location = loc; k.Attr[k.AttrCount].Format = fmt; k.Attr[k.AttrCount].Offset = off;
      k.AttrCount++;
    }
    if(stream==0) off += size;
  }
}

/****************************************************************************/
/***  Geometry buffers                                                    ***/
/****************************************************************************/

struct sGeoBuffer { VkcBuf B; };

sGeoBufferPart::sGeoBufferPart() { Start = 0; Count = 0; Buffer = 0; }
sGeoBufferPart::~sGeoBufferPart() { Clear(); }

void sGeoBufferPart::Clear()
{
  if(Buffer) { vkc_buf_free(Buffer->B); delete Buffer; }
  Buffer = 0; Start = 0; Count = 0;
}

void sGeoBufferPart::Init(sInt count,sInt size,sGeometryDuration duration,sInt buffertype)
{
  Start = 0;
  Count = count;
  Buffer = new sGeoBuffer;
  sU64 bytes = sU64(count)*size; if(bytes==0) bytes = 16;
  Buffer->B = (duration==sGD_STATIC) ? vkc_alloc_static(bytes) : vkc_alloc_frame(bytes);
}

void sGeoBufferPart::Lock(void **ptr) { *ptr = Buffer->B.Map; }
void sGeoBufferPart::Unlock(sInt count,sInt size) { if(count>=0) Count = count; }

/****************************************************************************/

static sU8 *DummyBuffer; static sDInt DummySize;
static sU8 *GrowDummyBuffer(sInt newsize)
{
  if(DummySize<newsize) { delete[] DummyBuffer; DummyBuffer = new sU8[newsize]; DummySize = newsize; }
  return DummyBuffer;
}

void sGeometry::BeginQuadrics() {}
void sGeometry::EndQuadrics() {}
void sGeometry::BeginQuad(void **data,sInt count) { sFatal(L"sGeometry quadrics are not supported by the vulkan backend"); *data = GrowDummyBuffer(Format->GetSize(0)*count*4); }
void sGeometry::EndQuad() {}
void sGeometry::BeginGrid(void **data,sInt xs,sInt ys) { sFatal(L"sGeometry grids are not supported by the vulkan backend"); *data = GrowDummyBuffer(Format->GetSize(0)*xs*ys); }
void sGeometry::EndGrid() {}

/****************************************************************************/
/***  Material state snapshot used by the next draw                       ***/
/****************************************************************************/

static VkcDraw CurDraw;
static sBool CurValid;
static sU8 PushData[128];

static VkcSampler MakeSampler(sU32 tf,sF32 lodbias)
{
  VkcSampler s; memset(&s,0,sizeof(s));
  switch(tf&sMTF_LEVELMASK)
  {
  case sMTF_LEVEL0: s.Filter = 0; s.Mip = 0; break;
  case sMTF_LEVEL1: s.Filter = 1; s.Mip = 0; break;
  case sMTF_LEVEL3: s.Filter = 1; s.Mip = 1; s.Aniso = 1; break;
  default:          s.Filter = 1; s.Mip = 1; break;     // LEVEL2: trilinear
  }
  switch(tf&sMTF_ADDRMASK)
  {
  case sMTF_CLAMP:  s.Addr = 1; break;
  case sMTF_MIRROR: s.Addr = 2; break;
  case sMTF_BORDER_BLACK: s.Addr = 3; break;
  case sMTF_BORDER_WHITE: s.Addr = 4; break;
  default: s.Addr = 0;
  }
  s.LodBias = lodbias;
  return s;
}

void sVkSetMaterialShaders(sMaterial *mtrl, sInt kind, sU32 permutation)
{
  mtrl->NameId = kind;
  SetPerm(mtrl,permutation);
}

void sMaterial::Create2() {}
void sMaterial::Destroy2() { DropPerm(this); }
void sMaterial::SetStates(sInt variant) {}
void sMaterial::AddMtrlFlags(sU32 *&data) {}

void sMaterial::Prepare(sVertexFormatHandle *format)
{
  SelectShaders(format);
  if(StateVariants==0)
  {
    InitVariants(1);
    SetVariant(0);
  }
}

void sMaterial::Set(sCBufferBase **cbuffers,sInt cbcount,sInt variant)
{
  sVERIFY(variant<StateVariants);
  const sMaterialRS &rs = VariantFlags[variant];
  VkcDraw &d = CurDraw;
  memset(&d.Key,0,sizeof(d.Key));
  VkcPipeKey &k = d.Key;

  switch(NameId)
  {
  case sVKM_SIMPLE:  k.Kind = 0; break;
  case sVKM_MSG:     k.Kind = 1; break;
  case sVKM_MSGGROW: k.Kind = 2; break;
  case sVKM_IPP:     k.Kind = 3; break;
  default: sFatal(L"sMaterial::Set: material has no backend shader kind");
  }
  k.Perm = GetPerm(this);
  k.AlphaRef = (rs.FuncFlags[sMFT_ALPHA]==sMFF_GREATEREQUAL) ? rs.AlphaRef : -1;

  // depth
  sU32 z = rs.Flags & sMTRL_ZMASK;
  k.DepthTest  = (z & sMTRL_ZREAD) ? 1 : 0;
  k.DepthWrite = (z & sMTRL_ZWRITE) ? 1 : 0;
  k.DepthFunc  = rs.FuncFlags[sMFT_DEPTH];
  // culling
#ifdef MSG_VK_CULL_INVERT
  const sU32 cullon = 2, cullinv = 1;
#else
  const sU32 cullon = 1, cullinv = 2;
#endif
  switch(rs.Flags & sMTRL_CULLMASK) { case sMTRL_CULLON: k.Cull = cullon; break; case sMTRL_CULLINV: k.Cull = cullinv; break; default: k.Cull = 0; }
  // blending: the packed sMB_* flags are decoded into source / op / destination
  if(rs.BlendColor!=sMB_OFF)
  {
    k.BlendOn = 1;
    k.SrcC = (rs.BlendColor>>0)&15; k.OpC = (rs.BlendColor>>8)&7; k.DstC = (rs.BlendColor>>16)&15;
    sU32 ba = (rs.BlendAlpha==sMB_SAMEASCOLOR) ? rs.BlendColor : rs.BlendAlpha;
    k.SrcA = (ba>>0)&15; k.OpA = (ba>>8)&7; k.DstA = (ba>>16)&15;
  }
  // colour write mask (a set sMTRL_MSK_* bit DISABLES the channel)
  k.ColorMask = ((rs.Flags&sMTRL_MSK_RED)?0:1) | ((rs.Flags&sMTRL_MSK_GREEN)?0:2) | ((rs.Flags&sMTRL_MSK_BLUE)?0:4) | ((rs.Flags&sMTRL_MSK_ALPHA)?0:8);

  sU32 bfc = rs.BlendFactor;
  d.BlendConst[0] = ((bfc>>16)&255)/255.0f; d.BlendConst[1] = ((bfc>>8)&255)/255.0f; d.BlendConst[2] = (bfc&255)/255.0f; d.BlendConst[3] = ((bfc>>24)&255)/255.0f;

  for(sInt i=0;i<VKC_MAXTEX;i++)
  {
    d.Tex[i] = Texture[i] ? GetImg(Texture[i]) : 0;      // unbound slots read opaque black (D3D9), see vkcore
    d.Smp[i] = MakeSampler(rs.TFlags[i],rs.LodBias[i]);
  }

  // constant buffers are laid out back to back in the push constant block
  sU32 off = 0;
  for(sInt i=0;i<cbcount;i++)
  {
    sCBufferBase *cb = cbuffers[i];
    if(!cb) continue;
    sU32 n = cb->RegCount*16;
    sVERIFY(off+n<=128);
    sCopyMem(PushData+off,cb->DataPersist,n);
    off += n;
  }
#ifdef MSG_VK_MATRIX_TRANSPOSE
  if(off>=64) { sF32 *m = (sF32*)PushData; for(int a=0;a<4;a++) for(int b=a+1;b<4;b++) { sF32 t = m[a*4+b]; m[a*4+b] = m[b*4+a]; m[b*4+a] = t; } }
#endif
  d.Push = PushData; d.PushSize = off;
  CurValid = sTRUE;
  sGFXMtrlIsSet = 1;
}

/****************************************************************************/

void sGeometry::Draw(sDrawRange *ir,sInt irc,sInt instancecount,sVertexOffset *)
{
  if(!CurValid || !vkc_in_frame()) return;
  VkcDraw d = CurDraw;
  VkcPipeKey &k = d.Key;
  BuildAttrs(Format,k);
  d.Quads = sFALSE;
  switch(Flags & sGF_PRIMMASK)
  {
  case sGF_TRILIST:   k.Topology = 0; break;
  case sGF_TRISTRIP:  k.Topology = 3; break;
  case sGF_LINELIST:  k.Topology = 1; break;
  case sGF_LINESTRIP: k.Topology = 2; break;
  case sGF_QUADLIST:  k.Topology = 0; d.Quads = sTRUE; break;
  default: sFatal(L"sGeometry::Draw: unsupported primitive type");
  }
  if(VkDebug()) fprintf(stderr,"[draw] kind %u attrs %u stride %u vb %p ib %p vcount %d icount %d istart %d isize %d\n",k.Kind,k.AttrCount,k.Stride,(void*)VertexPart[0].Buffer,(void*)IndexPart.Buffer,VertexPart[0].Count,IndexPart.Count,IndexPart.Start,IndexSize);
  if(!VertexPart[0].Buffer) return;
  if(VkDebug()) { const float *pf=(const float*)d.Push; const float *vf=(const float*)VertexPart[0].Buffer->B.Map; fprintf(stderr,"[key] blend %u mask %u cull %u dt %u dw %u df %u aref %d perm %u topo %u\n",k.BlendOn,k.ColorMask,k.Cull,k.DepthTest,k.DepthWrite,k.DepthFunc,k.AlphaRef,k.Perm,k.Topology); fprintf(stderr,"[push %u] %g %g %g %g | %g %g %g %g ... col %g %g %g %g ; v0 %g %g %g\n",d.PushSize,pf[0],pf[1],pf[2],pf[3],pf[4],pf[5],pf[6],pf[7],pf[16],pf[17],pf[18],pf[19],vf[0],vf[1],vf[2]); }
  d.Vertex = VertexPart[0].Buffer->B;
  d.BaseVertex = VertexPart[0].Start;
  d.IndexSize = 0;
  if(IndexSize && IndexPart.Buffer) { d.Index = IndexPart.Buffer->B; d.IndexSize = IndexSize; }

  sInt total = d.IndexSize ? IndexPart.Count : VertexPart[0].Count;
  if(ir==0 || irc==0)
  {
    d.First = d.IndexSize ? IndexPart.Start : 0; d.Count = total;
    if(d.Count>0) vkc_draw(d);
  }
  else
  {
    for(sInt i=0;i<irc;i++)
    {
      d.First = ir[i].Start; d.Count = ir[i].End-ir[i].Start;
      if(d.Count>0) vkc_draw(d);
    }
  }
}

/****************************************************************************/
/***  Constant buffers                                                    ***/
/****************************************************************************/

sCBufferBase::sCBufferBase() { DataPtr = 0; DataPersist = 0; }
void sCBufferBase::SetPtr(void **dataptr,void *data) { DataPtr = dataptr; DataPersist = data; *DataPtr = DataPersist; }
void sCBufferBase::SetRegs() {}
sCBufferBase::~sCBufferBase() {}

/****************************************************************************/
/***  Textures                                                            ***/
/****************************************************************************/

sU64 sGetAvailTextureFormats() { return sU64(1)<<sTEX_ARGB8888; }
sTextureBase *sReadTexture(sReader &s) { return 0; }

void sTexture2D::Create2(sInt flags)
{
  sInt fmt = flags & sTEX_FORMAT;
  if(fmt!=sTEX_UNKNOWN && fmt!=sTEX_ARGB8888)
    sFatal(L"vulkan backend: only sTEX_ARGB8888 textures are implemented");
  BitsPerPixel = 32;
  VkcImg *img = vkc_tex_create(SizeX,SizeY,Mipmaps,(flags&sTEX_RENDERTARGET)!=0);
  AddImg(this,img);
}

void sTexture2D::Destroy2()
{
  VkcImg *img = DropImg(this);
  if(img) vkc_tex_destroy(img);
}

void sTexture2D::BeginLoad(sU8 *&data,sInt &pitch,sInt mipmap)
{
  sVERIFY(LoadBuffer==0);
  sInt w = sMax(1,SizeX>>mipmap), h = sMax(1,SizeY>>mipmap);
  LoadBuffer = new sU8[w*h*4];
  Loading = mipmap;
  data = LoadBuffer;
  pitch = w*4;
}

void sTexture2D::EndLoad()
{
  sVERIFY(LoadBuffer);
  sInt w = sMax(1,SizeX>>Loading), h = sMax(1,SizeY>>Loading);
  VkcImg *img = GetImg(this);
  if(img) vkc_tex_upload(img,Loading,LoadBuffer,w*4,w,h);
  delete[] LoadBuffer;
  LoadBuffer = 0;
  Loading = -1;
}

void sTexture2D::Set(sInt stage) {}
void *sTexture2D::BeginLoadPalette() { sFatal(L"paletted textures not supported"); return 0; }
void sTexture2D::EndLoadPalette() {}

void sTextureCube::Create2(sInt flags) { sFatal(L"cube textures are not supported by the vulkan backend"); }
void sTextureCube::Destroy2() {}
void sTextureCube::BeginLoad(sTexCubeFace cf, sU8*& data, sInt& pitch, sInt mipmap) { data = GrowDummyBuffer(4*SizeXY*SizeXY); pitch = 0; }
void sTextureCube::EndLoad() {}
void sTextureCube::Set(sInt stage) {}

void sPackDXT(sU8 *d,sU32 *bmp,sInt xs,sInt ys,sInt format,sBool dither) { sFatal(L"sPackDXT not supported"); }

/****************************************************************************/
/***  Shader plumbing that is irrelevant with SPIR-V                      ***/
/****************************************************************************/

sShaderTypeFlag sGetShaderPlatform() { return sSTF_NONE; }
sInt sGetShaderProfile() { return sSTF_NONE; }
void sSetVSParam(sInt o, sInt count, const sVector4* vsf) {}
void sSetPSParam(sInt o, sInt count, const sVector4* psf) {}
void sSetVSBool(sU32 bits,sU32 mask) {}
void sSetPSBool(sU32 bits,sU32 mask) {}
void sCreateShader2(sShader *shader, sShaderBlob *blob) {}
void sDeleteShader2(sShader *shader) {}
void sSetShaders(sShader *,sShader *,sShader *,sLinkedShader **,sCBufferBase **,sInt) {}

/****************************************************************************/
/***  Initialisation / screen                                             ***/
/****************************************************************************/

sBool sSetOversizeScreen(sInt xs,sInt ys,sInt fsaa,sBool mayfail) { return sFALSE; }
void PreInitGFX(sInt flags,sInt &xs,sInt &ys) {}

void InitGFX(sInt flags,sInt xs,sInt ys)
{
  VkcConfig c; memset(&c,0,sizeof(c));
  c.Window = (struct SDL_Window *)sSDLWindow;
  c.Width = xs; c.Height = ys;
  c.Vsync = sVkVsync;
  c.ShaderDir = sVkShaderDir;
  c.DumpDir = sVkDumpDir; c.DumpStart = sVkDumpStart; c.DumpEvery = sVkDumpEvery;
  if(!vkc_init(c)) sFatal(L"vulkan initialisation failed");
  DXScreenMode.Flags |= sSM_VALID;
  DXScreenMode.ScreenX = xs;
  DXScreenMode.ScreenY = ys;
  DXScreenMode.Aspect = sF32(xs)/sF32(ys);
  sGFXRendertargetX = xs; sGFXRendertargetY = ys; sGFXRendertargetAspect = DXScreenMode.Aspect;
  sGFXViewRect.Init(0,0,xs,ys);
  sInitGfxCommon();
}

void ExitGFX()
{
  sExitGfxCommon();
  vkc_exit();
}

void ResizeGFX(sInt xs,sInt ys) {}   // the offscreen screen keeps its size; the swapchain blit scales

sInt sGetDisplayCount() { return 1; }

void sGetScreenInfo(sScreenInfo &si,sInt flags,sInt display)
{
  si.Clear();
  if(!sSDLDesktopW && !sGetShellSwitch(L"headless") && !getenv("MSG_HEADLESS"))
    shim_desktop_size(&sSDLDesktopW,&sSDLDesktopH);   // main() asks before sInit()
  si.Resolutions.HintSize(1); si.RefreshRates.HintSize(1); si.AspectRatios.HintSize(1);
  si.Resolutions.AddMany(1); si.RefreshRates.AddMany(1); si.AspectRatios.AddMany(1);
  si.Resolutions[0].x = sSDLDesktopW ? sSDLDesktopW : 1920;
  si.Resolutions[0].y = sSDLDesktopH ? sSDLDesktopH : 1080;
  si.RefreshRates[0] = 60;
  si.AspectRatios[0].x = 16; si.AspectRatios[0].y = 9;
  si.CurrentXSize = si.Resolutions[0].x;
  si.CurrentYSize = si.Resolutions[0].y;
  si.CurrentAspect = sF32(si.CurrentXSize)/si.CurrentYSize;
}

void sGetScreenMode(sScreenMode &sm) { sm = DXScreenMode; }
sBool sSetScreenMode(const sScreenMode &sm) { DXScreenMode = sm; return 1; }   // size changes are not supported at runtime

/****************************************************************************/
/***  Rendering                                                           ***/
/****************************************************************************/

static void SetTarget(sTexture2D *tex,const sRect *vrp,sInt clearflags,sU32 clearcolor)
{
  sInt xs,ys;
  VkcImg *img = 0;
  if(tex) { img = GetImg(tex); xs = tex->SizeX; ys = tex->SizeY; sGFXRendertargetAspect = sF32(xs)/sF32(ys); }
  else    { xs = DXScreenMode.ScreenX; ys = DXScreenMode.ScreenY; sGFXRendertargetAspect = DXScreenMode.Aspect; }
  sGFXRendertargetX = xs; sGFXRendertargetY = ys;
  sRect vr;
  if(vrp==0) { vr.Init(0,0,xs,ys); vrp = &vr; }
  sGFXViewRect = *vrp;
  int rect[4] = { vrp->x0, vrp->y0, vrp->x1, vrp->y1 };
  vkc_set_target(img,clearflags&3,clearcolor,rect);
}

void sSetRendertarget(const sRect *vrp, sInt clearflags, sU32 clearcolor) { SetTarget(0,vrp,clearflags,clearcolor); }
void sSetRendertarget(const sRect *vrp,sTexture2D *tex,sInt zbufflags, sInt clearflags, sU32 clearcolor) { SetTarget(tex,vrp,clearflags,clearcolor); }
void sSetRendertargetCube(sTextureCube*,sTexCubeFace,sInt cf, sU32 cc) { sFatal(L"cube rendertargets are not supported"); }

void sGrabScreen(class sTexture2D *tex, sGrabFilterFlags filter, const sRect *dst, const sRect *src)
{
  VkcImg *img = GetImg(tex);
  vkc_grab(img,filter==sGFF_LINEAR);
}

void sBeginSaveRT(sU8 *&data, sS32 &pitch, enum sTextureFlags &flags) { data = 0; pitch = 0; }
void sEndSaveRT() {}
void sRender3DFlush() {}

void sRender3DBegin()
{
  vkc_begin_frame();
  CurValid = sFALSE;
  sSetRendertarget(0,sCLEAR_NONE,0);
}

void sRender3DEnd() { vkc_end_frame(); }

void sSetRenderClipping(sRect *r,sInt count) {}
void sSetTexture(sInt stage,class sTextureBase *tex) {}
void sSetRenderStates(const sU32 *data, sInt count) {}
sInt sRenderStateTexture(sU32 *data, sInt texstage, sU32 tflags,sF32 lodbias) { return 0; }
void sGetGraphicsStats(sGraphicsStats &stat) { sClear(stat); }

#endif // sRENDERER == sRENDER_BLANK
