/****************************************************************************/
/***                                                                      ***/
/***   (C) 2005 Dierk Ohlerich, all rights reserved                       ***/
/***                                                                      ***/
/****************************************************************************/


#ifndef HEADER_ALTONA_UTIL_GRAPHICS
#define HEADER_ALTONA_UTIL_GRAPHICS

#ifndef __GNUC__
#pragma once
#endif

#include "base/types.hpp"
#include "base/math.hpp"

/****************************************************************************/
/****************************************************************************/
/***                                                                      ***/
/***   Graph3D - interface to 3d hardware                                 ***/
/***                                                                      ***/
/****************************************************************************/
/****************************************************************************/

void sSetFullscreen(sBool enable);
sBool sGetFullscreen();

void sSetScreenResolution(sInt resX, sInt resY);
void sSetScreenResolution(sInt resX, sInt resY, sF32 aspectRatio);
void sGetScreenResolution(sInt &resX, sInt &resY);

sBool sSetOversizeScreen(sInt xs,sInt ys,sInt fsaa=0,sBool mayfail=0); // should be called BEFORE sInit() xs=ys=0 to disable
void sGetScreenSize(sInt &xs,sInt &ys);
sF32 sGetScreenAspect();
void sGetScreenSafeArea(sF32 &xs, sF32 &ys); // returns values between 0 (center) and 1 (outside)


/****************************************************************************/
/***                                                                      ***/
/***   Forwards...                                                        ***/
/***                                                                      ***/
/****************************************************************************/

struct sGeoBuffer;
struct sGeoBufferPart;
struct sGeoPrim;
class sTextureBase;
class sTexture2D;
class sTexture3D;
class sTextureCube;

enum sTextureFlags 
{
  sTEX_UNKNOWN      =  0,         // choose a format.
  sTEX_ARGB8888     =  1,         // standard 32 bit texture
  sTEX_QWVU8888     =  2,         // interpreted as signed number, for bumpmaps

  sTEX_GR16         =  3,
  sTEX_ARGB16       =  4,

  sTEX_R32F         =  5,        // floating point textures
  sTEX_GR32F        =  6,
  sTEX_ARGB32F      =  7,
  sTEX_R16F         =  8,
  sTEX_GR16F        =  9,
  sTEX_ARGB16F      = 10,

  sTEX_A8           = 11,         // alpha only
  sTEX_I8           = 12,         // Intensity only (red=green=blue= 8bit)

  sTEX_DXT1         = 13,         // dxt1 without alpha
  sTEX_DXT1A        = 14,         // dxt1 with one bit alpha
  sTEX_DXT3         = 15,         // dxt3: direct 4 bit alpha
  sTEX_DXT5         = 16,         // dxt5: interpolated 3 bit alpha

  sTEX_INDEX4       = 17,
  sTEX_INDEX8       = 18,

  sTEX_MRGB8        = 19,         // 8bit scaled: HDR=M*RGB 
  sTEX_MRGB16       = 20,         // 16bit scaled: HDR=M*RGB 

  sTEX_ARGB2101010  = 21,
  sTEX_D24S8        = 22,         // depth stencil texture

  sTEX_I4           = 23,         // intensity only
  sTEX_IA4          = 24,         // intensity+alpha
  sTEX_IA8          = 25,         // intensity+alpha
  sTEX_RGB5A3       = 26,         // ARGB, depending on MSB, either 0:5:5:5 or 3:4:4:4
  sTEX_ARGB1555     = 27,         // 16 bit format
  sTEX_RGB565       = 28,
  sTEX_DXT5N        = 29,         // DXT5 for normal compression: rgba = 0g0r

  sTEX_FORMAT       = 0x000000ff, // mask for format field
  sTEX_DYNAMIC      = 0x00000100, // you want to update the texture regulary by BeginLoad() / EndLoad()
                                  // need DYNAMIC_PARTIAL and DYNAMIC_COMPLETE!
  sTEX_RENDERTARGET = 0x00000200, // you want to use the texture as a rendertarget.
  sTEX_NOMIPMAPS    = 0x00000400, // no mipmaps
  sTEX_MAIN_DEPTH   = 0x00000800, // use main render target depth stencil buffer
  sTEX_NORMALIZE    = 0x00010000, // enable renomalization durng mipmap generation - useful for bumpmaps!
  sTEX_SCREENSIZE   = 0x00020000, // ignore SizeX/SizeY, use size of screen - useful for rendertargets
  sTEX_SOFTWARE     = 0x00040000, // flagging sTextureSoftware type
  sTEX_SWIZZLED     = 0x00080000, // hardware dependent swizzled texture
  sTEX_ENDIAN_NN    = 0x00100000, // non native endianess of texture
  sTEX_AUTOMIPMAP   = 0x00200000, // generate mipmaps for rendertarget

  sTEX_2D           = 0x00001000, // texture type. 
  sTEX_CUBE         = 0x00002000,
  sTEX_3D           = 0x00003000,
  sTEX_KIND_MSK     = 0x0000f000,
  sTEX_TYPE_MASK    = 0x0000f000, // must be within lower 16 bit for sImgBlobExtract::Serialize_
};

sInt sGetBitsPerPixel(sInt format);
sU64 sGetAvailTextureFormats();   // bitmask of available texture formats on this hardware

enum sTexCubeFace
{
  sTCF_POSX   = 0,
  sTCF_NEGX,
  sTCF_POSY,
  sTCF_NEGY,
  sTCF_POSZ,
  sTCF_NEGZ,

  sTCF_NONE   = 0xffffffff,
};

/****************************************************************************/
/***                                                                      ***/
/***   General Engine Interface                                           ***/
/***                                                                      ***/
/****************************************************************************/

// stuff

enum sGetMatrixIndex
{
  sGM_MODEL = 0,
  sGM_MODELVIEW,
  sGM_MODELSCREEN,
  sGM_VIEW,
  sGM_PROJ,
//platform specific
#if sRENDERER == sRENDER_PS2
  sGM_PS2,
#endif
  sGM_MAX,
};

void sGetMatrix(sMatrix44 &mat,sGetMatrixIndex index);
const sMatrix44& sGetMatrix(sGetMatrixIndex index);
void sGetMatrix(sMatrix34 &mat,sGetMatrixIndex index);
void sGetMatrix(sMatrix34CM &mat,sGetMatrixIndex index);

void sRender3DBegin();        // use Render3DBegin(); ..rendercommands .. Render3DEnd();
void sRender3DEnd();          // for rendering without sApp (like simple tools)
                              // otherwise render in sApp::OnPaint3D implementation
                              // don't mix sApp::OnPaint3D and sRender3DBegin/sRender3DEnd
void sRender3DFlush();        // called AFTER sRender3DEnd, makes sure that the DMA-chain finishes
                              // this does not flip the framebuffer, you may need to draw the image multiple times.
void sClearCurrentCBuffers(); // internally called to invalidate constant buffer caching

void sCreateZBufferRT(sInt x, sInt y);
void sEnlargeZBufferRT(sInt x, sInt y);
void sDestroyZBufferRT();

enum sClearRendertargetFlags
{
  sCLEAR_NONE     = 0,
  sCLEAR_COLOR    = 1,
  sCLEAR_ZBUFFER  = 2,
  sCLEAR_ALL      = sCLEAR_COLOR|sCLEAR_ZBUFFER,
  sCLEAR_NOSCISSOR= 4,
};

// config flags which zbuffer to use when setting render targets
enum sRTDepthFlags
{
  sRTZBUF_DEFAULT,
  sRTZBUF_FORCEMAIN,
  sRTZBUF_NONE,
};

void sSetRendertarget(const sRect *vrp, sInt clearflags=sCLEAR_ALL, sU32 clearcolor=0);
void sSetRendertarget(const sRect *vrp,sTexture2D *tex,sInt zbufflags=sRTZBUF_DEFAULT, sInt clearflags=sCLEAR_ALL, sU32 clearcolor=0);
void sSetRendertargetCube(sTextureCube* tex, sTexCubeFace cf, sInt clearflags=sCLEAR_ALL, sU32 clearcolor=0);

enum sGrabFilterFlags
{
  sGFF_NONE,
  sGFF_POINT,
  sGFF_LINEAR,
};

void sGrabScreen(class sTexture2D *, sGrabFilterFlags filter, const sRect *dst, const sRect *src);
sINLINE void sGrabScreen(class sTexture2D *tex) { sGrabScreen(tex,sGFF_NONE,0,0); }

void sSetScreen(const sRect &rect,sU32 *data);
void sGetRendertargetSize(sInt &dx,sInt &dy);
void sSetRenderClipping(sRect *,sInt count);

void sPackDXT(sU8 *d,sU32 *s,sInt xs,sInt ys,sInt format,sBool dither=1);
sBool sIsFormatDXT(sInt format);

// returns ptr to rendertarget data
// very slow: use only if absolute necessary (screenshots)!!!
void sBeginSaveRT(sU8 *&data, sS32 &pitch, enum sTextureFlags &flags);   // returns sTEX_??? bitmap type flag
void sEndSaveRT();

void sSetTexture(sInt stage, class sTextureBase *tex);
void sSetRenderStates(const sU32 *data, sInt count);
sInt sRenderStateTexture(sU32 *data, sInt texstage, sU32 tflags,sF32 lodbias=0);

void sConvertsRGBTex(sBool e);    // to globally switch sRGB convertion on/off (hdr <-> ldr rendering)
                                  // with sConvertsRGBTex(sTRUE), texture are converted depending on sMTF_SRGB flag
sBool sConvertsRGBTex();

/****************************************************************************/

#include "base/types2.hpp"

// new screenmode interface
// * this is PC-Only, not for consoles
// * may be called BEFORE and AFTER sInit()
// * when used, overrides sInit() parameters

enum                              // flags that modify capabilities
{
  sSM_FULLSCREEN        = 0x0001, // fullscreen is default
  sSM_COLOR16BIT        = 0x0002, // use 565 or 1555 instead of 8888
  sSM_DEPTH16BIT        = 0x0004, // use a 16 bit depth format
  sSM_DEPTHREAD         = 0x0008, // use a readable depth format
  sSM_STENCIL           = 0x0010, // stencil buffer required
  sSM_OVERSIZE          = 0x0020, // use oversize buffer. clearing this does not prevent the overbuffer from beeing allocated.
  sSM_REFRAST           = 0x0040, //
  sSM_NOVSYNC           = 0x0080, //

  sSM_VALID             = 0x8000, // always set this flag.
};

struct sScreenMode
{
  sInt Flags;                     // sSM_??? flags
  sInt Display;                   // adapter number 0..n, -1 for default
  // screen
  sInt ScreenX;                   // screen size
  sInt ScreenY;
  sInt OverX;                     // renderbuffer size, may be larger than screen (or 0)
  sInt OverY;
  sF32 Aspect;                    // x/y of whole screen
  sInt MultiLevel;                // level 0..n, NOT number of samples!
  sInt OverMultiLevel;            // multisampling for oversized buffer
  // rendertargets
  sInt RTZBufferX;                // ZBuffer for offscreen rendertarget
  sInt RTZBufferY;

  sScreenMode();
  void Clear();
};

struct sScreenInfoXY
{
  sInt x,y;
};

struct sScreenInfo
{
  sInt Flags;
  sInt Display;

  sInt CurrentXSize;              // resolution of desktop
  sInt CurrentYSize;
  sF32 CurrentAspect;             // aspect of desktop, as x/y

  sStaticArray<sScreenInfoXY> Resolutions;    // Available resolutions (not for windowed)
  sStaticArray<sInt> RefreshRates;            // Available refresh rates (not for windowed)
  sInt MultisampleLevels;               // Available multisampling-levels
  sStaticArray<sScreenInfoXY> AspectRatios;   // commonly used aspect ratios

  sScreenInfo();
  void Clear();
};

sInt sGetDisplayCount();
void sGetScreenInfo(sScreenInfo &si,sInt flags=0,sInt display=-1);
void sGetScreenMode(sScreenMode &sm);
sBool sSetScreenMode(const sScreenMode &sm);

/****************************************************************************/

// statistics / performance-metering

struct sGraphicsStats
{
  sInt Batches;
  sInt Splitter;
  sInt Vertices;
  sInt Indices;
  sInt Primitives;

  sInt DynBytes;                        // bytes on DynBuffer total
  sInt QuadricsBytes;                   // bytes on DynBuffer used for quadrics
  
  sInt TexChanges[16];
  sInt AllTexChanges;
  sInt VSChanges;
  sInt PSChanges;
  sInt RTChanges;
  sInt MtrlChanges;

  sU64 StaticTextureMem;                // bytes in static textures, not counting rendertargets
};

void sGetGraphicsStats(sGraphicsStats &stat);

/****************************************************************************/
/***                                                                      ***/
/***   Vertex Formats                                                     ***/
/***                                                                      ***/
/****************************************************************************/

enum sVertexFlags                 // experimental way of speficying vertex formats
{                                 // (x) <- minimum number of dimensions
  sVF_POSITION      = 0x0001,     // (3) position (always set)
  sVF_NORMAL        = 0x0002,     // (3) normal
  sVF_TANGENT       = 0x0003,     // (3) tangent
  sVF_BONEINDEX     = 0x0004,     // (4) 4 indices to matrices
  sVF_BONEWEIGHT    = 0x0005,     // (4) 4 weighting factors to matrices

  sVF_COLOR0        = 0x0008,     // (4) color
  sVF_COLOR1        = 0x0009,     // (4) color
  sVF_COLOR2        = 0x000a,     // (4) color
  sVF_COLOR3        = 0x000b,     // (4) color
  sVF_UV0           = 0x000c,     // (2) texture uv's
  sVF_UV1           = 0x000d,     // (2) texture uv's
  sVF_UV2           = 0x000e,     // (2) texture uv's
  sVF_UV3           = 0x000f,     // (2) texture uv's

  sVF_USEMASK       = 0x001f,     // these flags are also stored in a bitmask, so it's limited to 32.

  sVF_STREAM0       = 0x0000,     // stream descriptor
  sVF_STREAM1       = 0x0040,
  sVF_STREAM2       = 0x0080,
  sVF_STREAM3       = 0x00c0,
  sVF_STREAMMASK    = 0x00c0,     // reserve for 4 streams 
  sVF_STREAMMAX     = 4,          // max number of streams
  sVF_STREAMSHIFT   = 6,          // shift to extract sream

  // optional: specify datatype. if missing, defaults are provided.
  // always available
  sVF_F2            = 0x0100,     // 2 x sF32
  sVF_F3            = 0x0200,     // 3 x sF32
  sVF_F4            = 0x0300,     // 4 x sF32
  sVF_C4            = 0x0400,     // 4 x sU8, scaled to 0..1 range
  // sometimes available
  sVF_I4            = 0x0500,     // 4 x sU8, not scaled. for bone-indices
  sVF_S2            = 0x0600,     // 2 x sS16, scaled to -1..1 range
  sVF_S4            = 0x0700,     // 4 x sS16, scaled to -1..1 range
  sVF_H2            = 0x0800,     // 2 x sF16
  sVF_H4            = 0x0900,     // 4 x sF16
  sVF_F1            = 0x0a00,     // 1 x sF32

  sVF_TYPEMASK      = 0xff00,

  sVF_END           = 0,          // endmarker.
};

extern const sInt sVertexFormatTypeSizes[]; // contains sizes of datatypes (index=datatype>>8)

#if sCONFIG_SYSTEM_PS3          // some platforms require sU32 vertex colors to be swapped. this has nothing to do with endianess
inline sU32 sSWAPVC(sU32 x)  { return sSwapEndian(x); }
#elif sRENDERER==sRENDER_WII
inline sU32 sSWAPVC(sU32 x)  { return (x>>24)|(x<<8); }
#elif sRENDERER==sRENDER_PS2
inline sU32 sSWAPVC(sU32 x)  { return ((x&0xff000000)>>0)|(x&0x0000ff00)|((x&0x00ff0000)>>16)|((x&0x000000ff)<<16); }
#else
#define sSWAPVC(x) x
#endif

struct sHalfFloat // don't try to make a fully featured halffloat class, you don't want to accidentally use it...
{                 // Seee EEEE EMMM MMMM  MMMm mmmm mmmm mmmm  (32 bit, e-127)
  sU16 Val;       //                      SEEE EEMM MMMM MMMM  (16 bit, e-15)
  void Set(sF32 f) { sU32 i=*(sU32 *)&f; Val=((i&0x80000000)>>16)|(((i&0x7fe00000)>>13)-(127<<10)+(15<<10))|((i&0x007fe000)>>13); } // do not care about special cases :-(
};

struct sVertexFormatHandle
{
  friend sVertexFormatHandle *sCreateVertexFormat(const sU32 *discriptor);
  friend void sDestroyAllVertexFormats();
  friend void sStreamVertexFormat(sWriter &, const sVertexFormatHandle *vhandle);

  struct OGLDecl
  {
    sInt Mode;                    // 0:END, 1:Attr, 2:Stream
    sInt Index;                   // Attr index or stream #
    sInt Size;                    // 1,2,3 or 4 scalars
    sInt Type;                    // GL_FLOAT, GL_UNSIGNED_BYTE, ...
    sInt Normalized;              // remap integers to [0..1]/[-1..1]
    sInt Offset;                  // offset to attribute in stream
  };
private:
  sInt Count;
  sU32 *Data;
  sInt UseCount;
  sInt VertexSize[sVF_STREAMMAX];
  sInt Streams;
  sU32 AvailMask;
  sVertexFormatHandle *Next;
#if sRENDERER==sRENDER_DX9
  struct IDirect3DVertexDeclaration9 *Decl;
#endif
#if sRENDERER==sRENDER_XBOX360
  struct D3DVertexDeclaration *Decl;
#endif
#if sRENDERER==sRENDER_WII
public:
  sInt VertexFmt;
  sU32 AvailMaskW;
  sInt AttrCount;
  sInt Stream[32];
  sInt Offset[32];
  sInt Stride[32];
private:
#endif
#if sRENDERER==sRENDER_OGL2 || sRENDERER==sRENDER_PS3GL
  OGLDecl *Decl;
#endif
#if sRENDERER==sRENDER_PS2
public:
  struct sVertexFormatExtract *Private;
private:
#endif
  void Create();
  void Destroy();
public:
  sInt sOBSOLETE GetSize() const { return VertexSize[0]; }          // size in bytes
  sInt GetSize(sInt stream) const { return VertexSize[stream]; }    // size in bytes
  sInt GetStreams() const { return Streams; }                       // highest used stream + 1. if streams 0 and 2 are used, this is 3
  sBool Has(sInt flag) const { return (AvailMask & (1<<(flag & sVF_USEMASK)))!=0; } // check if the format has a specific attribute
  sU32 GetAvailMask() const { return AvailMask; }                   // get all available attributes

  const sU32 *GetDesc() const { return Data; }                      // if you want to know exactly...
#if sRENDERER==sRENDER_DX9
  struct IDirect3DVertexDeclaration9 *GetDecl() { return Decl; }    // this is only for internal use.
#endif
#if sRENDERER==sRENDER_XBOX360
  struct D3DVertexDeclaration *GetDecl() { return Decl; }    // this is only for internal use.
#endif
#if sRENDERER==sRENDER_OGL2 || sRENDERER==sRENDER_PS3GL
  OGLDecl *GetDecl() { return Decl; }                               // this is only for internal use.
#endif
};


extern sVertexFormatHandle *sVertexFormatBasic;       // pos, color0
extern sVertexFormatHandle *sVertexFormatStandard;    // pos, normal, uv0
extern sVertexFormatHandle *sVertexFormatDouble;      // pos, color0, uv0, uv1
extern sVertexFormatHandle *sVertexFormatSingle;      // pos, color0, uv0
extern sVertexFormatHandle *sVertexFormatTSpace;      // pos, normal, tangent, color0, uv0, uv1
extern sVertexFormatHandle *sVertexFormatTSpace4;     // pos, normal, tangent with bitangent sign, color0, uv0, uv1
extern sVertexFormatHandle *sVertexFormatTSpace4Anim; // pos, normal, tangent with bitangent sign, indices, weights, color0, uv0, uv1

sVertexFormatHandle *sCreateVertexFormat(const sU32 *descriptor); // create a vertex format
void sDestroyAllVertexFormats();                                  // called by system... vertex formats are cheap.

void sStreamVertexFormat(sWriter &, const sVertexFormatHandle *vhandle);
void sStreamVertexFormat(sReader &, sVertexFormatHandle *&vhandle);

/****************************************************************************/

inline sU32 sMakeU32(sF32 x,sF32 y,sF32 z)
{ return ((sInt((x+0.5f)*127)&0xff)<<0)
       + ((sInt((y+0.5f)*127)&0xff)<<8)
       + ((sInt((z+0.5f)*127)&0xff)<<16); }


struct sVertexBasic               // 16 bytes
{
  sF32 px,py,pz;                  // 3 pos
  sU32 c0;                        // 1 color
  void Init(sF32 PX,sF32 PY,sF32 PZ,sU32 C0) volatile
  { px=PX; py=PY; pz=PZ; c0=sSWAPVC(C0); }
  void Init(const sVector31 &p,sU32 C0) volatile
  { px=p.x; py=p.y; pz=p.z; c0=sSWAPVC(C0); }

  static sVertexFormatHandle* VertexFormat() {return sVertexFormatBasic;}
};

struct sVertexStandard            // 32 bytes
{
  sF32 px,py,pz;                  // 3 pos
  sF32 nx,ny,nz;                  // 3 normal
  sF32 u0,v0;                     // 2 uv's
  void Init(sF32 PX,sF32 PY,sF32 PZ,sF32 NX,sF32 NY,sF32 NZ,sF32 U0,sF32 V0) volatile
  { px=PX; py=PY; pz=PZ; nx=NX; ny=NY; nz=NZ; u0=U0; v0=V0; }
  void Init(const sVector31 &p,const sVector30 &n,sF32 U0,sF32 V0) volatile
  { px=p.x; py=p.y; pz=p.z; nx=n.x; ny=n.y; nz=n.z; u0=U0; v0=V0; }

  static sVertexFormatHandle* VertexFormat() {return sVertexFormatStandard;}
};

struct sVertexDouble              // 32 bytes
{
  sF32 px,py,pz;                  // 3 pos
  sU32 c0;                        // 1 color
  sF32 u0,v0;                     // 4 uv's
  sF32 u1,v1;

  void Init(sF32 PX,sF32 PY,sF32 PZ,sU32 C0,sF32 U0,sF32 V0,sF32 U1,sF32 V1) volatile
  { px=PX; py=PY; pz=PZ; c0=sSWAPVC(C0); u0=U0; v0=V0; u1=U1; v1=V1; }
  void Init(const sVector31 &p,sU32 C0,sF32 U0,sF32 V0,sF32 U1,sF32 V1) volatile
  { px=p.x; py=p.y; pz=p.z; c0=sSWAPVC(C0); u0=U0; v0=V0; u1=U1; v1=V1; }

  static sVertexFormatHandle* VertexFormat() {return sVertexFormatDouble;}
};

struct sVertexSingle              // 24 bytes
{
  sF32 px,py,pz;                  // 3 pos
  sU32 c0;                        // 1 color
  sF32 u0,v0;                     // 2 uv's

  void Init(sF32 PX,sF32 PY,sF32 PZ,sU32 C0,sF32 U0,sF32 V0) volatile
  { px=PX; py=PY; pz=PZ; c0=sSWAPVC(C0); u0=U0; v0=V0; }
  void Init(const sVector31 &p,sU32 C0,sF32 U0,sF32 V0) volatile
  { px=p.x; py=p.y; pz=p.z; c0=sSWAPVC(C0); u0=U0; v0=V0; }

  static sVertexFormatHandle* VertexFormat() {return sVertexFormatSingle;}
};

struct sVertexTSpace              // 56 bytes, this will change a lot....
{
  sF32 px,py,pz;                  // 3 position
  sF32 nx,ny,nz;                  // 3 normal
  sF32 tx,ty,tz;                  // 3 tangent
  sU32 c0;                        // 1 color
  sF32 u0,v0;                     // 4 uv's (we WILL want a lightmap!)
  sF32 u1,v1;
  void Init(sF32 PX,sF32 PY,sF32 PZ,sF32 NX,sF32 NY,sF32 NZ,sF32 U0,sF32 V0) volatile
  { px=PX; py=PY; pz=PZ; nx=NX; ny=NY; nz=NZ; u0=U0; v0=V0; tx=ty=tz=u1=v1=0; c0=0;}
  void Init(const sVector31 &p,const sVector30 &n,sF32 U0,sF32 V0) volatile
  { px=p.x; py=p.y; pz=p.z; nx=n.x; ny=n.y; nz=n.z; u0=U0; v0=V0; tx=ty=tz=u1=v1=0; c0=0; }

  static sVertexFormatHandle* VertexFormat() {return sVertexFormatTSpace;}
};


struct sVertexTSpace4             // 56 bytes, this will change a lot....
{
  sF32 px,py,pz;                  // 3 position
  sF32 nx,ny,nz;                  // 3 normal
  sF32 tx,ty,tz,tsign;            // 4 tangent
  sU32 c0;                        // 1 color
  sF32 u0,v0;                     // 4 uv's (we WILL want a lightmap!)
  sF32 u1,v1;
/*
  void Init(sF32 PX,sF32 PY,sF32 PZ,sF32 NX,sF32 NY,sF32 NZ,sF32 U0,sF32 V0)
  { px=PX; py=PY; pz=PZ; nx=NX; ny=NY; nz=NZ; u0=U0; v0=V0; tx=ty=tz=u1=v1=0; c0=0;}
  void Init(const sVector31 &p,const sVector30 &n,sF32 U0,sF32 V0)
  { px=p.x; py=p.y; pz=p.z; nx=n.x; ny=n.y; nz=n.z; u0=U0; v0=V0; tx=ty=tz=u1=v1=0; c0=0; }
*/
  static sVertexFormatHandle* VertexFormat() {return sVertexFormatTSpace4;}
};
/****************************************************************************/
/***                                                                      ***/
/***   Shader Interface                                                   ***/
/***                                                                      ***/
/****************************************************************************/
  
enum sShaderTypeFlag
{
  // platform
  sSTF_NONE     = 0x0000,
  sSTF_DX       = 0x0100,
  sSTF_GLSL     = 0x0200,           // vertex_shader / fragment_shader
  sSTF_NVIDIA   = 0x0300,           // vertex_program / fragment_progran
  sSTF_PS2      = 0x0400,
  sSTF_PS3      = 0x0500,
  sSTF_XBOX360  = 0x0600,
  sSTF_WII      = 0x0700,
  sSTF_HLSL     = 0x0800,
  sSTF_CG       = 0x0900,

  // kind
  sSTF_VERTEX   = 0x1000,
  sSTF_PIXEL    = 0x2000,
  sSTF_GEOMETRY = 0x3000,

  // dx profiles
  sSTFP_DX_00   = 0x0000,
  sSTFP_DX_13   = 0x0001,
  sSTFP_DX_20   = 0x0002,
  sSTFP_DX_30   = 0x0003,
  sSTFP_DX_40   = 0x0004,
  sSTFP_DX_11   = 0x0005,
  sSTFP_DX_30X  = 0x0006,

  sSTF_DX_00    = sSTF_DX|sSTFP_DX_00,
  sSTF_DX_11    = sSTF_DX|sSTFP_DX_11,    // vs 
  sSTF_DX_13    = sSTF_DX|sSTFP_DX_13,    // ps
  sSTF_DX_20    = sSTF_DX|sSTFP_DX_20,    // vs & ps
  sSTF_DX_30    = sSTF_DX|sSTFP_DX_30,    // vs & ps
  sSTF_DX_40    = sSTF_DX|sSTFP_DX_40,    // vs & ps

  // masks
  sSTF_PROFILE  = 0x00ff,
  sSTF_PLATFORM = 0x0f00,
  sSTF_KIND     = 0xf000,
};


enum sShaderCompileFlag
{
  sSCF_DEBUG        = 0x00010000,   // generating debug infos
  sSCF_AVOID_CFLOW  = 0x00020000,   // avoiding control flow
  sSCF_PREFER_CFLOW = 0x00040000,   // prefer control flow
};

class sShader;                              // some kind of shader (pixel, vertex, geometry)
class sLinkedShader;                        // not yet implemented, for OpenGL ShaderObject. optional parameter to sSetShaders! 


enum
{
  sCBUFFER_MAXSLOT = 32,
  sCBUFFER_SHADERTYPES = 3,
  sCBUFFER_SHADERMASK = sCBUFFER_MAXSLOT*3,
  sCBUFFER_VS = sCBUFFER_MAXSLOT*0,
  sCBUFFER_PS = sCBUFFER_MAXSLOT*1,
  sCBUFFER_GS = sCBUFFER_MAXSLOT*2,
};

class sCBufferBase                           // do not use this class directly!
{
  friend class sMaterial;                   // the Vulkan backend reads DataPersist in sMaterial::Set
  friend void sSetCBuffers(sCBufferBase **,sInt);
  friend void sSetShaders(sShader *vs,sShader *ps,sShader *gs,sLinkedShader **link,sCBufferBase **cbuffers,sInt cbcount);

public:
  sCBufferBase();                           // create data and lock
  void SetPtr(void **dataptr,void *data);   // used internally during initialization
  void Modify();                            // relock new copy of buffer
  void OverridePtr(void *);                 // reload cb with data from here
  ~sCBufferBase();                          // destroy data

  sInt RegStart;                            // starting register
  sInt RegCount;                            // register count
  sU64 Mask;                                // used register mask
  sInt Slot;                                // dedicated slot & shader type

  void SetCfg(sInt slot, sInt start, sInt count);
protected:
  void SetRegs();                           // unlock and actually set constant registers
                                            // this have to be private/protected or current cbuffers have to be managed within SetRegs!
  void *DataPersist;
  void **DataPtr;
private:
};

// helper for compile time mask generation
template <sInt Start, sInt Count> struct CBMask
{
  template <sInt val, sInt MIN, sInt MAX> struct Clamp
  {
    enum { Val = val>MAX?MAX:(val<MIN?MIN:val) };
  };
  static const sU64 Mask =
    ( (((sU64(1)<<Clamp<(Count+3)/4-32,0,32>::Val)-1)<<32) |
      ((sU64(1)<<Clamp<(Count+3)/4,0,32>::Val)-1)
    )<<(Start/4);
};

template <class Type>                       // you are supposed to create this as member variable or directly on stack
class sCBuffer : public sCBufferBase
{
  Type Storage;                             // sCBuffer always copy to command buffer and is valid until destruction
                                            // need other cbuffer type for direct command buffer access without copy
public:
  sCBuffer()
  {
    // compile time checks for register alignment
    typedef sInt check0[(Type::RegStart&3)?-1:1];
    // compile time mask generation
    Mask = CBMask<Type::RegStart,Type::RegCount>::Mask;
    ////// runtime mask generation
    //Mask = ((sU64(1)<<(sClamp((Type::RegCount+3)/4-32,0,32)))-1)<<32;
    //Mask |= (sU64(1)<<(sClamp((Type::RegCount+3)/4,0,32)))-1;
    //Mask = Mask<<(Type::RegStart/4);
    RegStart=Type::RegStart; RegCount=Type::RegCount; Slot=Type::Slot; SetPtr((void**)&Data,(void*)&Storage);
  }
  Type *Data;
};


// shader data helper
struct sShaderBlob
{
  sInt Type;
  sInt Size;
  sU8 Data[1];                              // dummy size, real size given by Bytes

  void SetNext(sInt type);
  sShaderBlob *Next();
  sShaderBlob *Get(sInt type);
  sShaderBlob *GetAny(sInt kind, sInt platform);
};

void sSerializeShaderBlob(sU8 *data, sInt &size, sReader &stream);
void sSerializeShaderBlob(const sU8 *data, sInt size, sWriter &stream);

// global functions

sShaderTypeFlag sGetShaderPlatform();
sInt sGetShaderProfile();

sShader *sCreateShader(sInt type,const sU8 *code,sInt bytes);      // create the shader from shader blob
sShader *sCreateShaderRaw(sInt type,const sU8 *code,sInt bytes);   // create the shader from "raw" data
void sSetShaders(sShader *vs,sShader *ps,sShader *gs=0,sLinkedShader **link=0,sCBufferBase **cbuffers=0,sInt cbcount=0);
void sSetCBuffers(sCBufferBase **cbuffers,sInt cbcount);
sINLINE void sSetCBuffers(sCBufferBase *cbuffers)                 { sSetCBuffers(&cbuffers,1); }

// the way shader parameters are set is not final. need a good idea here.

void sSetVSParam(sInt o, sInt count, const sVector4* vsf);    // VS consts are persistent between shaders
void sSetPSParam(sInt o, sInt count, const sVector4* psf);    // reset all PS constants every time you switch shaders
void sSetVSBool(sU32 bits,sU32 mask);                         // update predication
void sSetPSBool(sU32 bits,sU32 mask);

// the shader class

class sShader
{
  union
  {
  sU8 *Data;                      // shader code
  sShaderBlob *Blob;
  };
  sInt Size;                      // size of code in bytes
  sInt Type;                      // PS/VS/GS
  sInt UseCount;                  // refcounting
  sU32 Hash;
  sShader *Link;

  ~sShader();
  sShader(sInt type,const sU8 *data,sInt length,sU32 hash,sBool raw=sFALSE);
  friend sShader *sCreateShader(sInt type,const sU8 *code,sInt bytes);
  friend sShader *sCreateShaderRaw(sInt type,const sU8 *code,sInt bytes);
  friend void sSetShaders(sShader *vs,sShader *ps,sShader *gs,sLinkedShader **link,sCBufferBase **cbuffers,sInt cbcount);
  friend void sCreateShader2(sShader *shader,sShaderBlob *blob);
  friend void sDeleteShader2(sShader *shader);
  friend void sSetShader2(sShader *shader);

  union                           // various platform dependent representations
  {
#if sRENDERER == sRENDER_XBOX360
    struct D3DVertexShader *vs;
    struct D3DPixelShader *ps;
#else
    struct IDirect3DVertexShader9 *vs;  // sRENDER_DX9
    struct IDirect3DPixelShader9 *ps;   // sRENDER_DX9
#endif
    sInt GLName;                        // sRENDER_OGL2, also misused for Wii
    struct _CGprogram *cg;              // sRENDER_PS3GL, old version
    struct CGBShader *cgb;              // sRENDER_PS3GL, new version
  };
public:
  void AddRef();
  void Release();
  sBool CheckKind(sInt);
  const sU8 *GetCode(sInt &bytes);

  sDNode Node;                    // privatE: list of all shaders
  sInt Temp;
};

// constant buffers
/*
class sCBuffer
{
  friend void sSetShaders(sShader *vs,sShader *ps,sShader *gs,sLinkedShader **link,sCBuffer **cbuffers,sInt cbcount);
  sInt RegStart;
  sInt RegCount;
  sInt Slot;
  sU32 *Copy;
  sU32 *Source;
public:
  sCBuffer(sInt start,sInt count,sInt slot,void *source);
  ~sCBuffer();
  void Update();
};
*/

/****************************************************************************/
/****************************************************************************/

// first obsolete shader interface

typedef sOBSOLETE class sShader *sShaderHandle; 
                                                              // always first set shader, then set constants
void sOBSOLETE sAddRefVS(sShader * vsh);
void sOBSOLETE sAddRefPS(sShader * psh);
sShader sOBSOLETE *sCreateVS(const sU32 *data,sInt count, sInt level=sSTF_DX_20);
sShader sOBSOLETE *sCreatePS(const sU32 *data,sInt count, sInt level=sSTF_DX_20);
void sOBSOLETE sDeleteVS(sShader *&);
void sOBSOLETE sDeletePS(sShader *&);
sBool sOBSOLETE sValidVS(sShader * vsh);
sBool sOBSOLETE sValidPS(sShader * psh);

// second obsolete shader interface

void sOBSOLETE sReleaseShader(sShader * sh);                      // shaders are refcounted
void sOBSOLETE sAddRefShader(sShader * sh);                       // shaders are refcounted
sBool sOBSOLETE sCheckShader(sShader * sh,sInt type);      // checks if the shader handle is a valid shader of the given type
const sOBSOLETE sU8 *sGetShaderCode(sShader * sh,sInt &bytes);    // get code from which the shader was created

/****************************************************************************/
/***                                                                      ***/
/***   Viewport                                                           ***/
/***                                                                      ***/
/****************************************************************************/

enum sViewportUpdateFlags
{
  sVUF_MODEL      = 0x0001,       // update model matrix
  sVUF_VIEW       = 0x0002,       // update view matrix
  sVUF_PROJ       = 0x0004,       // update projection matrix
  sVUF_WINDOW     = 0x0008,       // update screen rectangle

  sVUF_ALL        = 0x000f,       // update all
  sVUF_TRANSFORM  = 0x0007,       // update all transformation (not sVUF_WINDOW)
};

enum sViewportOrthogonal
{
  sVO_PROJECTIVE  = 0,            // perspective projection
  sVO_PIXELS      = 1,            // pixel addresssing
  sVO_SCREEN      = 2,            // 0..1 
};

class sViewport
{
public:
  sMatrix44 Proj;                 // cache for projection matrix
  sMatrix34 View;
  sMatrix34 ModelView;            // Model * View
  sMatrix44 ModelScreen;          // Model * View * Proj

public:
  sViewport();
  void SetTarget(sTexture2D *tex);        // set target to whole texture. (if texture is 0, use whole screen)
  void SetTarget(sTextureCube *tex, sTexCubeFace cf);
  void SetTargetCurrent();                // takes whatever was set with sSetRendertarget()
  void SetZoom(sF32 aspect,sF32 zoom);  // zoom specifies the fov of the smaller axis. see getaspect.
  void SetZoom(sF32 zoom);        // as above, aspect is calculated from TargetSizeXY
  void Prepare(sInt update=sVUF_ALL);
  void UpdateModelMatrix(const sMatrix34 &mat); // just change model matrix and call prepare.
  void Set() const;

  void PMatrix(const sMatrix44 &mat)  { Proj = mat; }
  const sMatrix44& PMatrix() const    { return Proj; }
  const sMatrix34& VMatrix() const    { return View; }
  const sMatrix34& MVMatrix() const   { return ModelView; }
  const sMatrix44& MVPMatrix() const  { return ModelScreen; }

  sMatrix34 Model;                // matrix for model
  sMatrix34 Camera;               // matrix for camera. View = Camera^-1

  sInt TargetSizeX;               // size of the rendertarget you want to use.
  sInt TargetSizeY;               // size of the rendertarget you want to use.
  sF32 TargetAspect;              // aspect ratio of the rendertarget you want to use.
  sRect Target;                   // portion inside the rendertarget you want to use.

  sF32 ClipNear;                  // z-clipping
  sF32 ClipFar;
//  sF32 Zoom,Aspect;
  sF32 ZoomX;                     // cos(fov_x)
  sF32 ZoomY;                     // cos(fov_x)*ys/xs

  sF32 CenterX;                   // center (set to 0.5 for real center), inside Window
  sF32 CenterY;
  sInt Orthogonal;                // completly changes meaning of everything: render orthogonally. Please also set CenterX and CenterY to 0

  sF32 GetAspect() { return ZoomX/ZoomY; }  // aspect is X/Y, like 16/9. Previously it was defined the other way around!

  // using the frustum

  void MakeRayPixel(sInt mx,sInt my,sRay &ray) const;    // make ray, mx and my normalized to -1..1
  void MakeRay(sF32 mx,sF32 my,sRay &ray) const;    // make ray, mx and my normalized to -1..1
  sBool Transform(const sVector31 &p,sInt &ix,sInt &iy) const;
  sBool Transform(const sVector31 &p,sF32 &ix,sF32 &iy) const;
  sInt Visible(const sAABBox &box) const { return Visible(box,ClipNear,ClipFar); }    // 0=total out, 1=clip, 2=total in
  sInt Visible(const sAABBox &box, sF32 clipnear, sF32 clipfar) const;    // 0=total out, 1=clip, 2=total in
  sInt VisibleDist(const sAABBox &box,sF32 &dist) const;    // if not total out, return distance of closest point
};

/****************************************************************************/
/***                                                                      ***/
/***   Geometry                                                           ***/
/***                                                                      ***/
/****************************************************************************/

// old flags, dont use any more!
// remove?

//enum sGeometryFlagsOld             // obsolete names
//{
//  sGFV_STATIC            = 0x0001, // static buffer, never change once written
//  sGFV_FRAME             = 0x0002, // dynamic buffer, rewrite each frame
//  sGFV_STREAM            = 0x0003, // stremed buffer, draw immediatly
//  sGFV_MASK              = 0x0003,
//
//  sGFI_STATIC            = 0x0010, // see above
//  sGFI_FRAME             = 0x0020,
//  sGFI_STREAM            = 0x0030,
//  sGFI_MASK              = 0x0030,
//
//  sGF_STATIC             = 0x0011, // see above
//  sGF_FRAME              = 0x0022,
//  sGF_STREAM             = 0x0033,
//
//  sGFT_TRILIST           = 0x1000, // geometry types
//  sGFT_TRISTRIP          = 0x2000,
//  sGFT_LINELIST          = 0x3000,
//  sGFT_LINESTRIP         = 0x4000,
//  sGFT_QUAD              = 0x5000,
//  sGFT_SPRITE            = 0x6000,
//  sGFT_MASK              = 0xf000,
//
////  sGF_INDEX32        = 0x00010000, // use 32 bit index buffers (avoid)
//};

struct sDrawRange
{
  sInt Start;
  sInt End;
};

enum sGeomtryFlags                // flags for initializing geometries
{
  sGF_TRILIST           = 0x1000, // geometry types
  sGF_TRISTRIP          = 0x2000,
  sGF_LINELIST          = 0x3000,
  sGF_LINESTRIP         = 0x4000,
  sGF_QUADLIST          = 0x5000,
  sGF_SPRITELIST        = 0x6000,
  sGF_QUADRICS          = 0xf000, // special primitive mode (quad/grid)
  sGF_PRIMMASK          = 0xf000,

  sGF_INDEXOFF      = 0x00000000, // no index buffer (avoid)
  sGF_INDEX32       = 0x00010000, // use 32 bit index buffers (avoid)
  sGF_INDEX16       = 0x00020000, // use 16 bit index buffers (do!)
  sGF_INDEXMASK     = 0x00030000, // mask index buffer bits

  sGF_INSTANCES     = 0x00040000, // use instancing API
};

enum sGeometryDuration            // state of a buffer
{
  sGD_NONE = 0,                   // uninitialized slot
  sGD_STATIC,                     // forever until changed
  sGD_FRAME,                      // flushed after end of frame
  sGD_STREAM,                     // flushed immediatly
};

enum sGeometryPrimitiveMode
{
  sGP_GRID = 1,
  sGP_QUAD = 2,
};

#define sMAX_VBCOUNT 0x8000       // prefered size for dynamic vertex and index buffers
#define sMAX_IBCOUNT 0x20000      // if you request more, you will get a "custom" buffer.
#define sMAX_QUADCOUNT 0x2000

struct sGeoBufferPart             // This geometry owns a part of a VB/IB
{
  sInt Start;                     // range start (in elements)
  sInt Count;                     // range size (in elements)
  sGeoBuffer *Buffer;             // buffer
  
  sGeoBufferPart();
  ~sGeoBufferPart();
#if sRENDERER==sRENDER_OGL2 || sRENDERER==sRENDER_PS3GL || sRENDERER==sRENDER_BLANK
  void Init(sInt count,sInt size,sGeometryDuration duration,sInt buffertype);
  void Lock(void **);
  void Unlock(sInt count,sInt size);
  void Clear();
#endif
#if sRENDERER==sRENDER_DX9
  void *Init(sInt count,sInt size,sGeometryDuration duration,sInt buffertype,sInt advance=0);
  void Advance(sInt count,sInt size);
  void Clear();
#endif
};

struct sVertexOffset
{
  sInt VOff[sVF_STREAMMAX];       // vertex offsets in stream
};

class sGeometry                   // the geometry itself
{
private:
//  struct IDirect3DVertexDeclaration9 *VertexDecl;

#if sRENDERER==sRENDER_DX9
  void DrawPrim();                // specialized draw routine for prim mode.
  void DrawPrim(sGeoPrim *start,sGeoPrim *end,sInt ic,sInt vc);
#endif

#if sRENDERER==sRENDER_DX9 || sRENDERER==sRENDER_OGL2 || sRENDERER==sRENDER_PS3GL || sRENDERER==sRENDER_BLANK
  sGeoBufferPart VertexPart[sVF_STREAMMAX];
  sGeoBufferPart IndexPart;
#endif
#if sRENDERER==sRENDER_XBOX360
  void DrawPrim();                // specialized draw routine for prim mode.
  void DrawPrim(const sGeoPrim *start,const sGeoPrim *end,sInt ic,sInt vc);
  struct sGeometryXBox360 *Private;
  sInt PrimAppend;
#endif
#if sRENDERER==sRENDER_PS2
  struct sGeometryPS2 *Private;
  void FinalizeLoad();
  void EndGrid(sU8 *data,sInt tx,sInt ty,sInt ts);
public:
  void ReadPrivate(sReader &s);
  void HintSplitter(sInt start,sInt end);
  void DrawSplitters(sInt start,sInt end,struct sVifBuffer &vif);
  void DrawSplittersBone(sInt start,sInt end,struct sVifBuffer &vif);
private:
#endif
#if sRENDERER==sRENDER_WII
public:  
  struct sGeometryWii *Private;
  sBool ReadPrivate(sReader &s);
  void HintSplitter(sInt start,sInt end);
private:
#endif

  sInt Flags;                     // flags at creation
  sInt IndexSize;                 // 2 or 4, derived from flags
  sBool PrimMode;                 // this buffer is loaded in prim mode (Quad/Grid)
  sGeoPrim *FirstPrim;            // first primitive in single linked list
  sGeoPrim **LastPrimPtr;         // used to append to single linked list
  sGeoPrim *CurrentPrim;          // used to unlock vertex buffer
  
  sVertexFormatHandle *Format;

  void InitPrivate();             // create private implementation
  void ExitPrivate();             // destroys private implementation
public:
  sGeometry();
  ~sGeometry();
  void Clear();
  void Serialize(sReader &s);

  // old interface
  void sOBSOLETE BeginLoad(sInt vc,sInt ic,sInt flags,sVertexFormatHandle *,void **vp,void **ip);

  // initialization and drawing

  void Init(sInt flags,sVertexFormatHandle *);
  sVertexFormatHandle *GetFormat() { return Format; }
  void Draw(sDrawRange *ir=0,sInt irc=0,sInt instancecount=0, sVertexOffset *off=0);  // draw many small splitters

  // traditional buffer loading

  void BeginLoadIB(sInt ic,sGeometryDuration duration,void **ip);
  void EndLoadIB(sInt ic=-1);
  void BeginLoadVB(sInt vc,sGeometryDuration duration,void **vp,sInt stream=0);
  void EndLoadVB(sInt vc=-1,sInt stream=0);

  void BeginLoad(sVertexFormatHandle *,sInt flags,sGeometryDuration duration,sInt vc,sInt ic,void **vp,void **ip);
  void EndLoad(sInt vc=-1,sInt ic=-1);

  // quadrics interface (quads, grids and sprites)
  // the advantage: all quadrics are drawn in one or few GPU-Batches,
  // even when mixing multiple geometries. good for huds and 2d-stuff.

  void BeginQuadrics();
  void EndQuadrics();
  void BeginQuad(void **data,sInt count);
  void EndQuad();
  void BeginGrid(void **data,sInt xs,sInt ys);
  void EndGrid();

};


// macros for generating quads:
// don't template this, only sU16 & sU32 are allowed!

inline void sQuad(sU16 *&ip,sInt o,sInt a,sInt b,sInt c,sInt d)
{
  *ip++ = o+a;
  *ip++ = o+b;
  *ip++ = o+c;
  *ip++ = o+a;
  *ip++ = o+c;
  *ip++ = o+d;
}

inline void sQuadI(sU16 *&ip,sInt o,sInt d,sInt c,sInt b,sInt a)
{
  *ip++ = o+a;
  *ip++ = o+b;
  *ip++ = o+c;
  *ip++ = o+a;
  *ip++ = o+c;
  *ip++ = o+d;
}

inline void sQuad(sU32 *&ip,sInt o,sInt a,sInt b,sInt c,sInt d)
{
  *ip++ = o+a;
  *ip++ = o+b;
  *ip++ = o+c;
  *ip++ = o+a;
  *ip++ = o+c;
  *ip++ = o+d;
}

inline void sQuadI(sU32 *&ip,sInt o,sInt d,sInt c,sInt b,sInt a)
{
  *ip++ = o+a;
  *ip++ = o+b;
  *ip++ = o+c;
  *ip++ = o+a;
  *ip++ = o+c;
  *ip++ = o+d;
}

/****************************************************************************/
/***                                                                      ***/
/***   Textures                                                           ***/
/***                                                                      ***/
/****************************************************************************/

// slow! only for debugging
void sCopyCubeFace(sTexture2D *dest, sTextureCube *src, sTexCubeFace cf);

class sTextureBase
{
protected:
  friend class sMaterial;
  friend void sSetTexture(sInt, class sTextureBase*);
  virtual void Set(sInt stage);

  sInt Loading;
  sInt LockFlags;
  sU8 *LoadBuffer;                // some implementations require a buffer for loading textures

  void InitPrivate();             // create private implementation
  void ExitPrivate();             // destroys private implementation

#if sRENDERER==sRENDER_DX9
  union
  {
    struct IDirect3DTexture9 *Tex2D;
    struct IDirect3DCubeTexture9 *TexCube;
    struct IDirect3DVolumeTexture9 *Tex3D;
  };
#endif
#if sRENDERER==sRENDER_XBOX360
public:
  struct sTextureXBox360 *Private;
protected:
#endif
#if sRENDERER==sRENDER_PS2
  struct sTexturePS2 *Private;
#endif
#if sRENDERER==sRENDER_WII
public:  
  struct sTextureWii *Private;
protected:
#endif
public:

#if sRENDERER==sRENDER_DX9
  virtual struct IDirect3DBaseTexture9* DeviceTexture();
  virtual void OnLostDevice(sBool reinit=sFALSE);
#endif
#if sRENDERER==sRENDER_PS3GL
  sInt GLBind;
#endif
#if sRENDERER==sRENDER_OGL2
  sInt GLName;
#endif

  sTextureBase();
  virtual ~sTextureBase();

  // casting
  class sTexture2D *CastTex2D()       { if((Flags&(sTEX_TYPE_MASK|sTEX_SOFTWARE))==sTEX_2D) return (class sTexture2D*) this; return 0; }
  class sTextureCube *CastTexCube()   { if((Flags&(sTEX_TYPE_MASK|sTEX_SOFTWARE))==sTEX_CUBE) return (class sTextureCube*) this; return 0; }
  class sTexture3D *CastTex3D()       { if((Flags&(sTEX_TYPE_MASK|sTEX_SOFTWARE))==sTEX_3D) return (class sTexture3D*) this; return 0; }

  sInt Temp;                      // for free use...
  sInt NameId;

  // read only!
  sInt Flags;                     // creation flags
  sInt Mipmaps;                   // number of valid mipmaps
  sInt BitsPerPixel;              // bits per pixel in this format

  //!
  sU16 FrameRT;                   // last frame rendered into rendertarget
  sU16 SceneRT;                   // last scene rendered into rendertarget
};

// shortcut implementation for loading hardware dependant texture data
// returns NULL if not implemented or not applicable
sTextureBase *sReadTexture(sReader &s);


/****************************************************************************/
/***                                                                      ***/
/***   sTexture2D                                                         ***/
/***                                                                      ***/
/****************************************************************************/

class sTexture2D :
  public sTextureBase
{
private:
  friend void sCopyCubeFace(sTexture2D *, sTextureCube *, sTexCubeFace);
  friend void sSetRendertarget(const sRect *vrp,sTexture2D *tex,sInt zbufferflags, sInt clearflags, sU32 clearcolor);
  friend void sGrabScreen(class sTexture2D *tex, sGrabFilterFlags filter, const sRect *dst, const sRect *src);
  void Set(sInt stage);

#if sRENDERER==sRENDER_DX9
  virtual struct IDirect3DBaseTexture9* DeviceTexture();
  virtual void OnLostDevice(sBool reinit=sFALSE);
#endif
#if sRENDERER==sRENDER_OGL2 || sRENDER_PS3GL
  sInt GLName;
  sInt GLFBName;
  sInt LoadMipmap;
#endif

  void Create2(sInt flags);
  void Destroy2();

public:
  sTexture2D();
  sTexture2D(sInt xs,sInt ys,sU32 flags,sInt mipmaps=0);
  virtual ~sTexture2D();

  void Init(sInt xs, sInt ys, sInt flags,sInt mipmaps=0, sBool force=sFALSE);
  void Clear();

  void BeginLoad(sU8 *&data,sInt &pitch,sInt mipmap=0);
  void EndLoad();
  void LoadAllMipmaps(sU8 *data);
  void *BeginLoadPalette();       // most platforms don't support palettes
  void EndLoadPalette();

#if sPLATFORM==sPLAT_WII
  void ReadPrivate(sReader &s);
#endif

  // read only!
  sInt SizeX;                     // size of texture
  sInt SizeY;
};

/****************************************************************************/
/***                                                                      ***/
/***   sTextureCube                                                       ***/
/***                                                                      ***/
/****************************************************************************/

class sTextureCube :
  public sTextureBase
{
private:
  friend void sCopyCubeFace(sTexture2D *, sTextureCube *, sTexCubeFace);
  friend void sSetRendertargetCube(sTextureCube* tex, sTexCubeFace cf, sInt clearflags, sU32 clearcolor);
  void Set(sInt stage);

  sTexCubeFace  LockedFace;
#if sRENDERER==sRENDER_DX9
  virtual IDirect3DBaseTexture9* DeviceTexture();
  virtual void OnLostDevice(sBool reinit=sFALSE);
#endif
#if sRENDERER==sRENDER_OGL2 || sRENDER_PS3GL
  sInt GLName;
  sU32 GLFBName[6];
  sInt LoadMipmap;
  sInt LoadFace;
#endif

  void Create2(sInt flags);
  void Destroy2();

public:
  sTextureCube();
  sTextureCube(sInt dim, sInt flags, sInt mipmaps=0);
  virtual ~sTextureCube();

  void Init(sInt dim, sInt flags, sInt mipmaps=0, sBool force=sFALSE);
  void Clear();

  void BeginLoad(sTexCubeFace cf, sU8*& data, sInt& pitch, sInt mipmap=0);
  void EndLoad();
  void LoadAllMipmaps(sU8 *data);

  // read only!
  sInt SizeXY;                    // size of texture
};

/****************************************************************************/
/***                                                                      ***/
/***   sTextur3D                                                          ***/
/***                                                                      ***/
/****************************************************************************/

class sTexture3D :
  public sTextureBase
{
private:
#if sRENDERER==sRENDER_DX9
  virtual struct IDirect3DBaseTexture9* DeviceTexture();
  virtual void OnLostDevice(sBool reinit=sFALSE) {}
#endif
  void Set(sInt stage);

public:
  sTexture3D(sInt xs, sInt ys, sInt zs, sU32 flags);
  virtual ~sTexture3D();

  void BeginLoad(sU8*& data, sInt& rpitch, sInt& spitch, sInt mipmap=0);
  void EndLoad();
  void Load(sU8 *src);

  // read only!
  sInt SizeX;                     // size of texture
  sInt SizeY;
  sInt SizeZ;
};


/****************************************************************************/
/***                                                                      ***/
/***   Material                                                           ***/
/***                                                                      ***/
/****************************************************************************/

enum sMaterialConsts
{
  sMTRLENV_LIGHTS  = 4,
  sMTRLENV_MATRICES = 4,
  sMTRLENV_COLORS = 4,

  sMTRL_MAXTEX = 8,
};

sOBSOLETE class sMaterialEnv                          // additional variables for material
{
public:
  sMaterialEnv();                                     // initialize with meaningful defaults
  void Fix();                                         // check and repear ranges

  sVector30 LightDir[sMTRLENV_LIGHTS];                // 4 directional lights
  sU32 LightColor[sMTRLENV_LIGHTS];                   // color 0x00000000 disables light
  sU32 AmbientColor;                                  // ambient color.

  sMatrix44 Matrix[sMTRLENV_MATRICES];                // general purpose matrices
  sVector4 Color[sMTRLENV_COLORS];                    // general purpose pixel shader colors

  // view effect flags

  // fog parameters
  sF32 FogMax;
  sF32 FogStart;
  sF32 FogEnd;
  sF32 FogDensity;
  sU32 FogColor;

  // depth of field ipp parameters
  sF32 DOFBlurNear;                                   // near plane with maximal dof blur
  sF32 DOFFocusNear;                                  // near plane start with zero dof blur
  sF32 DOFFocusFar;                                   // far plane ending zero dof blur
  sF32 DOFBlurFar;                                    // far plane with maximal dof blur
  sF32 DOFBlurMax;                                    // maximal dof blur factor [0..1]
};

// material render state flags
struct sMaterialRS
{
  sU32 Flags;                               // sMTRL_?? flags
  sU8 FuncFlags[4];                         // FuncFlags[sMFT_??] = sMFF_?? flags for depth test [0], alpha test [1] and stencil test [2]
  sU32 TFlags[sMTRL_MAXTEX];                // sMTF_?? flags
  sU32 BlendColor;                          // sMB?_?? flags for color
  sU32 BlendAlpha;                          // sMB?_?? flags for alpha or sMB_SAMEASCOLOR
  sU32 BlendFactor;                         // blendfactor 
  sU8 StencilOps[6];                        // sMSO_??? two sided stencil op flags for FAIL, ZFAIL, PASS (CW, CCW)
  sU8 StencilRef;														// the stencil reference value
  sU8 StencilMask;                          //
  sU8 AlphaRef;                             // reference for alpha test
  sF32 LodBias[sMTRL_MAXTEX];               // mipmap lod bias. this interface is not final, it does not get saved!
};

class sMaterial
{
public:
  sShader *VertexShader;
  sShader *PixelShader;

protected:
  friend class sToolPlatform;

  void SetFixedEnv(sMaterialEnv *env);
  void SetShader(sShader * vs,sShader * ps);
  void Create2();
  void Destroy2();

  enum {STATES_2PASS = 8};

#if sRENDERER==sRENDER_DX9 || sRENDERER==sRENDER_BLANK || sRENDERER==sRENDER_XBOX360
  sMaterialRS *VariantFlags;
  sU32 **States;
  sInt *StateCount;
  sInt StateVariants;
  void SetStates(sInt variant=0);
  void AddMtrlFlags(sU32 *&data);                     // add Flags and Blend 
  void AllocStates(const sU32 *data,sInt count,sInt var);  // optimise and copy state array from static buffer to material
#endif


#if sRENDERER==sRENDER_OGL2
  void SetStates(sInt variant=0);
#endif

#if sRENDERER==sRENDER_WII
public:
  sMaterialRS *VariantFlags;
  sInt *States;
  sInt StateVariants;
  void SetStates(sInt variant=0);
protected:
#endif

#if sRENDERER==sRENDER_PS2
  struct sMaterialPS2 *Private;
  void SetStates(sInt variant=0);
#endif

#if sRENDERER==sRENDER_PS3GL
  void SetStates();
#endif

public:
  sMaterial();
  virtual ~sMaterial();
  void Prepare(sVertexFormatHandle *vf);              // prepare the material
  virtual void SelectShaders(sVertexFormatHandle *vf)=0; // overload this to set VertexShader and PixelShader member
  virtual void Set(sMaterialEnv *env) { Set(env,0); }               // OBSOLETE set material parameters depending on MaterialEnv
  virtual void Set(sMaterialEnv *env, sInt variant) {}              // OBSOLETE set material parameters depending on MaterialEnv
  virtual void Set(const struct sEngLightBundle *, sInt pass) {}    // OBSOLETE set engine material parameters depending on lighting

  void Set(sInt variant=0)                                                  { Set(0,0,variant); }
  void Set(sCBufferBase **cbuffers,sInt cbcount,sInt variant=0);
  void Set(sCBufferBase *a)                                                 { Set(&a,1); }
  void Set(sCBufferBase *a,sCBufferBase *b)                                 { sCBufferBase *A[2] = {a,b};     Set(A,2); }
  void Set(sCBufferBase *a,sCBufferBase *b,sCBufferBase *c)                 { sCBufferBase *A[3] = {a,b,c};   Set(A,3);  }
  void Set(sCBufferBase *a,sCBufferBase *b,sCBufferBase *c,sCBufferBase *d) { sCBufferBase *A[4] = {a,b,c,d}; Set(A,4);  }
  void CopyBaseFrom(sMaterial *src);

  void InitVariants(sInt max);
  void SetVariant(sInt var);

  sTextureBase *Texture[sMTRL_MAXTEX];

  sInt NameId;

  // sMaterialRS:
  sU32 Flags;                               // sMTRL_?? flags
  sU8 FuncFlags[4];                         // FuncFlags[sMFT_??] = sMFF_?? flags for depth test [0], alpha test [1] and stencil test [2]
  sU32 TFlags[sMTRL_MAXTEX];                // sMTF_?? flags
  sU32 BlendColor;                          // sMB?_?? flags for color
  sU32 BlendAlpha;                          // sMB?_?? flags for alpha or sMB_SAMEASCOLOR
  sU32 BlendFactor;                         // blendfactor 
  sU8 StencilOps[6];                        // sMSO_??? two sided stencil op flags for FAIL, ZFAIL, PASS (CW, CCW)
  sU8 StencilRef;														// the stencil reference value
  sU8 StencilMask;                          //
  sU8 AlphaRef;                             // reference for alpha test
  sF32 LodBias[sMTRL_MAXTEX];               // mipmap lod bias. this interface is not final, it does not get saved!
};

enum sMaterialFlags
{
  // these flags are converted to renderstates by graphics.cpp

  sMTRL_ZOFF          = 0x0000,             // zbuffer control
  sMTRL_ZREAD         = 0x0001,
  sMTRL_ZWRITE        = 0x0002,
  sMTRL_ZON           = 0x0003,
  sMTRL_ZMASK         = 0x0003,

  sMTRL_CULLOFF       = 0x0000,             // no culling, doublesided
  sMTRL_CULLON        = 0x0010,             // normal culling
  sMTRL_CULLINV       = 0x0020,             // inverted culling
  sMTRL_CULLMASK      = 0x0030,

  sMTRL_MSK_ALPHA     = 0x0100,             // color write mask
  sMTRL_MSK_RED       = 0x0200, 
  sMTRL_MSK_GREEN     = 0x0400,
  sMTRL_MSK_BLUE      = 0x0800,
  sMTRL_MSK_RGBA      = 0x0f00,
  sMTRL_MSK_MASK      = 0x0f00,

  sMTRL_STENCILSS     = 0x00010000,         // single sided stencil operation
  sMTRL_STENCILDS     = 0x00020000,         // double sided stencil operation

  // these flags can be used to permuate vertex shaders

  sMTRL_LIGHTING      = 0x0080,             // enable light calculation!

  sMTRL_VC_NOLIGHT    = 0x0000,
  sMTRL_VC_LIGHT0     = 0x1000,             // vertex color0 used as vertex light
  sMTRL_VC_LIGHT1     = 0x2000,             // vertex color1 used as vertex light
  sMTRL_VC_LIGHT_MSK  = 0x3000,
  sMTRL_VC_LSHIFT     = 12,

  sMTRL_VC_NOCOLOR    = 0x0000,
  sMTRL_VC_COLOR0     = 0x4000,             // vertex color0 used as material color
  sMTRL_VC_COLOR1     = 0x8000,             // vertex color1 used as material color
  sMTRL_VC_COLOR_MSK  = 0xc000,
  sMTRL_VC_CSHIFT     = 14,

  sMTRL_VC_MSK        = 0xf000,
  sMTRL_VC_OFF        = 0x0000,
};


enum sMaterialFunctionType
{
  sMFT_DEPTH = 0,
  sMFT_ALPHA = 1,
  sMFT_STENCIL = 2,
};

enum sMaterialFunctionFlag
{
  sMFF_NEVER          = 0,
  sMFF_LESS           = 1,
  sMFF_EQUAL          = 2,
  sMFF_LESSEQUAL      = 3,
  sMFF_GREATER        = 4,
  sMFF_NOTEQUAL       = 5,
  sMFF_GREATEREQUAL   = 6,
  sMFF_ALWAYS         = 7,
};

enum sMaterialTextureFlags
{
  // these flags are converted to renderstates by graphics.cpp

  sMTF_LEVEL0         = 0x0002,                       // off
  sMTF_LEVEL1         = 0x0003,                       // filter
  sMTF_LEVEL2         = 0x0000,                       // filter+mipmap (normal)
  sMTF_LEVEL3         = 0x0001,                       // extended quality
  sMTF_LEVELMASK      = 0x0003,

  sMTF_TILE           = 0x0000,                       // texture addressing
  sMTF_CLAMP          = 0x0010,
  sMTF_MIRROR         = 0x0020,
  sMTF_BORDER         = 0x0030,
  sMTF_BORDER_BLACK   = 0x0030,                       // border color 0x00000000
  sMTF_BORDER_WHITE   = 0x0040,                       // border color 0xffffffff
  sMTF_ADDRMASK       = 0x00f0,

  sMTF_SRGB           = 0x1000,                       // gamma corrected sRGB or linear texture, enable sMTF_SRGB for albedo colormaps,
                                                      // disable for lightmaps lookup textures or multiplicative blended textures
  sMTF_EXTERN         = 0x2000,                       // extern texture set by application (eg. render target)

  // these flags can be used to permuate vertex shaders

  sMTF_UVMASK         = 0x0f00,                       // these bits control uv transformation
  sMTR_UV3DMASK       = 0x0800,                       // if this is set, we need 3d calculations!
  sMTF_UV0            = 0x0000,
  sMTF_UV1            = 0x0100,
  sMTF_UV2            = 0x0200,
  sMTF_UV3            = 0x0300,
  sMTF_WORLDSPACE     = 0x0800,
  sMTF_MODELSPACE     = 0x0900,
  sMTF_CAMERASPACE    = 0x0a00,
  sMTF_REFLECTION     = 0x0b00,                       // lookup texture with reflection vector
  sMTF_SPHERICAL      = 0x0c00,                       // lookup texture with normal vector

  sMTF_SCALE          = 0x4000,
  sMTF_TRANSFORM      = 0x8000,                       // duplicated, remove and only use matrix selector
  sMTF_MTRL_MAT       = 0x00010000,                   // use sMTR_MTRL_MAT*id to select mtrl transformation matrix [1..4]
  sMTF_MTRL_MAT_MSK   = 0x000f0000,
  sMTF_ENV_MAT        = 0x00100000,                   // use sMTR_ENV_MAT*id to select environment transformation matrix [1..4]
  sMTF_ENV_MAT_MSK    = 0x00f00000,
};

enum sMaterialStencilOp                     // stencil operations, is like DirectX D3DSTENCILOP_??? -1
{
  sMSO_KEEP = 0,                            // no operation
  sMSO_ZERO = 1,                            // set to zero
  sMSO_REPLACE = 2,                         // replace with ref
  sMSO_INCSAT = 3,                          // increase by one and clamp
  sMSO_DECSAT = 4,                          // decrease by one and clamp
  sMSO_INVERT = 5,                          // bit-invert stencil buffer
  sMSO_INC = 6,                             // increase by one
  sMSO_DEC = 7,                             // decrease by one
};
 
enum sMaterialStencilOpIndex
{
  sMSI_CWFAIL = 0,
  sMSI_CWZFAIL = 1,
  sMSI_CWPASS = 2,
  sMSI_CCWFAIL = 3,
  sMSI_CCWZFAIL = 4,
  sMSI_CCWPASS = 5,

  sMSI_FAIL = sMSI_CWFAIL,
  sMSI_ZFAIL = sMSI_CWZFAIL,
  sMSI_PASS = sMSI_CWPASS,
};

enum sMaterialBlend                         // define your blendmode in 11 bits...
{
  // source factor

  sMBS_0              = 0x00000001,         // 0
  sMBS_1              = 0x00000002,         // 1
  sMBS_SC             = 0x00000003,         //     source color
  sMBS_SCI            = 0x00000004,         // 1 - source color
  sMBS_SA             = 0x00000005,         //     source alpha
  sMBS_SAI            = 0x00000006,         // 1 - source alpha
  sMBS_DA             = 0x00000007,         //     destination alpha
  sMBS_DAI            = 0x00000008,         // 1 - destination alpha
  sMBS_DC             = 0x00000009,         //     destination color
  sMBS_DCI            = 0x0000000a,         // 1 - destination color
  sMBS_SA_SAT         = 0x0000000b,         // min(SA,DAI)
  sMBS_F              = 0x0000000e,         //     blend factor
  sMBS_FI             = 0x0000000f,         // 1 - blend factor
  sMBS_MASK           = 0x0000000f,

  // operation (usually add)

  sMBO_ADD            = 0x00000100,         //     sMBS_??? * Source + sMBD_??? * Dest
  sMBO_SUB            = 0x00000200,         //     sMBS_??? * Source - sMBD_??? * Dest
  sMBO_SUBR           = 0x00000300,         //   - sMBS_??? * Source + sMBD_??? * Dest
  sMBO_MIN            = 0x00000400,         // min(sMBS_??? * Source , sMBD_??? * Dest)
  sMBO_MAX            = 0x00000500,         // max(sMBS_??? * Source , sMBD_??? * Dest)
  sMBO_MASK           = 0x00000f00,

  // destination factor (same as source)

  sMBD_0              = 0x00010000,       
  sMBD_1              = 0x00020000,
  sMBD_SC             = 0x00030000,
  sMBD_SCI            = 0x00040000, 
  sMBD_SA             = 0x00050000,
  sMBD_SAI            = 0x00060000,
  sMBD_DA             = 0x00070000,
  sMBD_DAI            = 0x00080000,
  sMBD_DC             = 0x00090000,
  sMBD_DCI            = 0x000a0000,
  sMBD_SA_SAT         = 0x000b0000,         // for alpha channel always 1
  sMBD_F              = 0x000e0000,
  sMBD_FI             = 0x000f0000,
  sMBD_MASK           = 0x000f0000,

  // special values

  sMB_SAMEASCOLOR     = 0xffffffff,         // BlendAlpha is same as BlendColor
  sMB_OFF             = 0x00000000,         // Alphablending disabled

  // common blending equations

  sMB_ALPHA           = (sMBS_SA |sMBO_ADD |sMBD_SAI),
  sMB_ADD             = (sMBS_1  |sMBO_ADD |sMBD_1  ),
  sMB_SUB             = (sMBS_1  |sMBO_SUBR|sMBD_1  ),
  sMB_MUL             = (sMBS_DC |sMBO_ADD |sMBD_0  ),
  sMB_MUL2            = (sMBS_DC |sMBO_ADD |sMBD_SC ),
  sMB_ADDSMOOTH       = (sMBS_1  |sMBO_ADD |sMBD_SCI),
  sMB_PMALPHA         = (sMBS_1  |sMBO_ADD |sMBD_SAI),
};

/****************************************************************************/
/***                                                                      ***/
/***   internal use only                                                  ***/
/***                                                                      ***/
/****************************************************************************/

extern sInt sGFXRendertargetX;
extern sInt sGFXRendertargetY;
extern sF32 sGFXRendertargetAspect;
extern sRect sGFXViewRect;
extern sMatrix44 sGFXMatrices[sGM_MAX];
extern sInt sGFXMtrlIsSet;
extern sInt sGFXViewIsSet;

/*
extern sInt sGFXScreenX;
extern sInt sGFXScreenY;
extern sInt sGFXCurrentFrame;
extern sInt sGFXResetScreen;
extern sInt sGFXResetScreenX;
extern sInt sGFXResetScreenY;
*/
/****************************************************************************/

// HEADER_ALTONA_UTIL_GRAPHICS
#endif
