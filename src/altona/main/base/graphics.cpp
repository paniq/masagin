/****************************************************************************/
/***                                                                      ***/
/***   (C) 2005 Dierk Ohlerich, all rights reserved                       ***/
/***                                                                      ***/
/****************************************************************************/

#include "base/types.hpp"
#include "base/system.hpp"
#include "base/math.hpp"
#include "base/graphics.hpp"
#include "base/windows.hpp"
#include "base/serialize.hpp"


sInt sGFXRendertargetX=0;
sInt sGFXRendertargetY=0;
sF32 sGFXRendertargetAspect=1.0;
sRect sGFXViewRect;
sMatrix44 sGFXMatrices[sGM_MAX];
sInt sGFXMtrlIsSet = 0;
sInt sGFXViewIsSet = 0;

/****************************************************************************/
// globals

static sCBufferBase *CurrentCBs[sCBUFFER_MAXSLOT*sCBUFFER_SHADERTYPES];
static sU64 CurrentCBsSlotMask[sCBUFFER_MAXSLOT*sCBUFFER_SHADERTYPES];
static sU64 CurrentCBsMask[3] = { 0, 0, 0 };

/****************************************************************************/
// externs

extern sInt sSystemFlags;
void sCreateShader2(sShader *shader, sShaderBlob *code);
void sDeleteShader2(sShader *shader);

/****************************************************************************/
// forwards

void sInitShaders();
void sExitShaders();

/****************************************************************************/
/***                                                                      ***/
/***   Initialisation                                                     ***/
/***                                                                      ***/
/****************************************************************************/

sVertexFormatHandle *sVertexFormatBasic;
sVertexFormatHandle *sVertexFormatStandard;
sVertexFormatHandle *sVertexFormatSingle;
sVertexFormatHandle *sVertexFormatDouble;
sVertexFormatHandle *sVertexFormatTSpace;
sVertexFormatHandle *sVertexFormatTSpace4;
sVertexFormatHandle *sVertexFormatTSpace4Anim;

sU32 DeclBasic[] = { sVF_POSITION,sVF_COLOR0,0 };
sU32 DeclStandard[] = { sVF_POSITION,sVF_NORMAL,sVF_UV0,0 };
sU32 DeclSingle[] = { sVF_POSITION,sVF_COLOR0,sVF_UV0,0 };
sU32 DeclDouble[] = { sVF_POSITION,sVF_COLOR0,sVF_UV0,sVF_UV1,0 };
sU32 DeclTSpace[] = { sVF_POSITION,sVF_NORMAL,sVF_TANGENT,sVF_COLOR0,sVF_UV0,sVF_UV1,0 };
sU32 DeclTSpace4[] = { sVF_POSITION,sVF_NORMAL,sVF_TANGENT|sVF_F4,sVF_COLOR0,sVF_UV0,sVF_UV1,0 };
sU32 DeclTSpace4Anim[] = { sVF_POSITION,sVF_NORMAL,sVF_TANGENT|sVF_F4,sVF_BONEINDEX|sVF_I4,sVF_BONEWEIGHT|sVF_C4,sVF_COLOR0,sVF_UV0,sVF_UV1,0 };

/****************************************************************************/

void sInitGfxCommon()
{
  sInitShaders();
  sVertexFormatBasic    = sCreateVertexFormat(DeclBasic);
  sVertexFormatStandard = sCreateVertexFormat(DeclStandard);
  sVertexFormatSingle   = sCreateVertexFormat(DeclSingle);
  sVertexFormatDouble   = sCreateVertexFormat(DeclDouble);
  sVertexFormatTSpace   = sCreateVertexFormat(DeclTSpace);
  sVertexFormatTSpace4  = sCreateVertexFormat(DeclTSpace4);
  sVertexFormatTSpace4Anim  = sCreateVertexFormat(DeclTSpace4Anim);
}

void sExitGfxCommon()
{
  sDestroyAllVertexFormats();
  sExitShaders();
}

/****************************************************************************/

sScreenMode::sScreenMode()
{
  Clear();
}

void sScreenMode::Clear()
{
  Flags = 0;
  Display = -1;
  ScreenX = 800;
  ScreenY = 600;
  OverX = 0;
  OverY = 0;
  Aspect = 0;
  MultiLevel = -1;
  OverMultiLevel = -1;
  RTZBufferX = 0;
  RTZBufferY = 0;
}

sScreenInfo::sScreenInfo()
{
  Clear();
}

void sScreenInfo::Clear()
{
  CurrentXSize = 0;
  CurrentYSize = 0;
  CurrentAspect = 0;
  MultisampleLevels = 0;
  Resolutions.Clear();
  RefreshRates.Clear();
  AspectRatios.Clear();
}

/****************************************************************************/
/***                                                                      ***/
/***   old screenmode interface                                           ***/
/***                                                                      ***/
/****************************************************************************/

void sSetScreenResolution(sInt xs,sInt ys)
{
  sScreenMode sm;
  sGetScreenMode(sm);

  sm.ScreenX = xs;
  sm.ScreenY = ys;

  sSetScreenMode(sm);
}

void sSetScreenResolution(sInt xs,sInt ys, sF32 aspectRatio)
{
  sScreenMode sm;
  sGetScreenMode(sm);

  sm.ScreenX = xs;
  sm.ScreenY = ys;
  sm.Aspect  = aspectRatio;

  sSetScreenMode(sm);
}

void sGetScreenSize(sInt &xs,sInt &ys)
{
  sScreenMode sm;
  sGetScreenMode(sm);

  xs = sm.ScreenX;
  ys = sm.ScreenY;
}

sF32 sGetScreenAspect()
{
  sScreenMode sm;
  sGetScreenMode(sm);

  return sm.Aspect;
}

void sEnlargeZBufferRT(sInt x,sInt y)
{
  sScreenMode sm;
  sGetScreenMode(sm);

  if(x>sm.RTZBufferX || y>sm.RTZBufferY)
  {
    sInt nx = sMax(sm.RTZBufferX,x);
    sInt ny = sMax(sm.RTZBufferY,y);
    sDPrintF(L"[gfx] zbufferrt enlarged, this is very bad. %dx%d -> %dx%d",sm.RTZBufferX,sm.RTZBufferY,nx,ny);
    sCreateZBufferRT(nx,ny);
  }
}

void sCreateZBufferRT(sInt xs,sInt ys)
{
  sScreenMode sm;
  sGetScreenMode(sm);

  sm.RTZBufferX = xs;
  sm.RTZBufferY = ys;

  sSetScreenMode(sm);
}

void sDestroyZBufferRT()
{
  sCreateZBufferRT(0,0);
}


/****************************************************************************/

sBool sGetFullscreen()
{
  sScreenMode sm;
  sGetScreenMode(sm);
  return (sm.Flags&sSM_FULLSCREEN)!=0;
}

void sSetFullscreen(sBool enable)
{
  sScreenMode sm;
  sGetScreenMode(sm);

  if (enable)
    sm.Flags = sm.Flags | sSM_FULLSCREEN;
  else
    sm.Flags = sm.Flags & ~sSM_FULLSCREEN;

  sSetScreenMode(sm);
}

void sGetScreenResolution(sInt &xs, sInt &ys)
{
  sGetScreenSize(xs,ys);
}

/****************************************************************************/
/***                                                                      ***/
/***   Shaders                                                            ***/
/***                                                                      ***/
/****************************************************************************/

template <class streamer> void Serialize_(streamer &);

void sShaderBlob::SetNext(sInt type)
{
  sVERIFY(Type != sSTF_NONE);
  sShaderBlob *blob=(sShaderBlob*)(Data+sAlign(Size,4));
  blob->Type = type;
}

sShaderBlob *sShaderBlob::Next()
{
  if(!Type)
    return 0;
  sShaderBlob *blob=(sShaderBlob*)(Data+sAlign(Size,4));
  if(blob->Type!=sSTF_NONE)
    return blob;
  return 0;
}

sShaderBlob *sShaderBlob::Get(sInt type)
{
  sShaderBlob *blob = this;
  while(blob && blob->Type!=type)
    blob = blob->Next();
  return blob;
}

sShaderBlob *sShaderBlob::GetAny(sInt kind, sInt platform)
{
  sShaderBlob *blob = this;
  while(blob && (blob->Type&0xff00)!=((kind|platform)&0xff00))
    blob = blob->Next();
  return blob;
}

void sSerializeShaderBlob(sU8 *data, sInt &size, sReader &s)
{
  sInt readsize=0;
  s | readsize;
  sVERIFY(!(size&3));
  sVERIFY(readsize<size);
  size = readsize;

  sShaderBlob *blob = 0;
  if(size)
  {
    blob = (sShaderBlob*)data;
    s | blob->Type;
  }
  while(blob && blob->Type != sSTF_NONE)
  {
    s | blob->Size;
    s.ArrayU8(blob->Data,blob->Size);
    s.Align();
    blob = (sShaderBlob*) (blob->Data+sAlign(blob->Size,4));
    s | blob->Type;
  }
}

void sSerializeShaderBlob(const sU8 *data, sInt size, sWriter &s)
{
  s | size;
  sShaderBlob *blob = 0;
  if(size)
  {
    blob = (sShaderBlob*)data;
    s | blob->Type;
  }
  while(blob && blob->Type != sSTF_NONE)
  {
    s | blob->Size;
    s.ArrayU8(blob->Data,blob->Size);
    s.Align();
    blob = (sShaderBlob*) (blob->Data+sAlign(blob->Size,4));
    s | blob->Type;
  }
}

/****************************************************************************/

static sDList<sShader,&sShader::Node> *Shaders=0;

void sInitShaders()
{
  Shaders = new sDList<sShader,&sShader::Node>;
}

void sExitShaders()
{
  sDelete(Shaders);
}

sShader *sCreateShaderRaw(sInt type,const sU8 *code,sInt bytes)
{
  type |= sGetShaderPlatform();
  sU32 hash;
  sShader *sh;

  sChecksumAdler32Begin();

  sShaderBlob header;
  header.Type = type;
  header.Size = bytes;

  sChecksumAdler32Add((sU8*)&header,8);
  sChecksumAdler32Add(code,bytes);
  sU8 tmp0 = 0;
  sInt tmp1 = sSTF_NONE;
  for(sInt i=bytes&3;(bytes+i)&3;i++)
    sChecksumAdler32Add(&tmp0,1);
  sChecksumAdler32Add((sU8*)&tmp1,4);

  hash = sChecksumAdler32End();

  sInt cmpbytes = ((bytes+3)&~3)+12;
  sFORALL_LIST(*Shaders,sh)
  {
    if(type==sh->Type && hash==sh->Hash && cmpbytes==sh->Size)
    {
      sShaderBlob *blob = (sShaderBlob*)sh->Data;
      if(blob->Type == header.Type && !blob->Next() && sCmpMem(blob->Data,code,bytes)==0)
      {
        sh->AddRef();
        return sh;
      }
    }
  }

  return new sShader(type,code,bytes,hash,sTRUE);
}

sShader *sCreateShader(sInt type,const sU8 *code,sInt bytes)
{
  sU32 hash;
  sShader *sh;

  hash = sChecksumAdler32(code,bytes);

  sFORALL_LIST(*Shaders,sh)
  {
    if(type==sh->Type && hash==sh->Hash && bytes==sh->Size)
    {
      if(sCmpMem(sh->Data,code,bytes)==0)
      {
        sh->AddRef();
        return sh;
      }
    }
  }

  return new sShader(type,code,bytes,hash);
}

sShader::sShader(sInt type,const sU8 *data,sInt length,sU32 hash,sBool raw/*=sFALSE*/)
{
  Temp = -1;
  GLName = 0;
  sVERIFY(Shaders);
  Shaders->AddTail(this);

  if(raw)
  {
    sInt blen = ((length+3)&~3)+12;
    Data = new sU8[blen];
    sShaderBlob *blob = (sShaderBlob*)Data;
    blob->Type = type;
    blob->Size = length;

    Size = blen;
    Type = type;
    Hash = hash;
    UseCount = 1;

    sCopyMem(blob->Data,data,length);
    for(sInt i=length&3;(length+i)&3;i++)
      blob->Data[length+i] = 0;
    blob = (sShaderBlob*)(blob->Data+((length+3)&~3));
    blob->Type = sSTF_NONE;
  }
  else
  {
    Data = new sU8[length];
    Size = length;
    Type = type;
    Hash = hash;
    UseCount = 1;

    sCopyMem(Data,data,length);
  }
  sShaderBlob *blob = Blob->GetAny(type,sGetShaderPlatform());
  if(!blob)
  {
    if(sRENDERER!=sRENDER_PS2)
      sDPrintF(L"warning: shader creation failed, no suitable shader found\n");
  }
  else
  {
    sCreateShader2(this,blob);
  }
}

sShader::~sShader()
{
  sVERIFY(Shaders);
  Shaders->Rem(this);
  sDeleteShader2(this);
  delete[] Data;
}

void sShader::AddRef()
{
  UseCount++;
}

void sShader::Release()
{
  if(this!=0) 
  {
    UseCount--;
    if(UseCount<=0)
      delete this;
  }
}

sBool sShader::CheckKind(sInt type)
{
  return (type&sSTF_KIND)==(Type&sSTF_KIND);
}

const sU8 *sShader::GetCode(sInt &bytes)
{
  bytes = Size;
  return Data;
}

/****************************************************************************/

// obsolete

void sReleaseShader(sShader * sh)
{
  sh->Release();
}

void sAddRefShader(sShader * sh)
{
  sh->AddRef();
}

sBool sCheckShader(sShader * sh,sInt type)
{
  return sh->CheckKind(type);
}

const sU8 *sGetShaderCode(sShader * sh,sInt &bytes)
{
  return sh->GetCode(bytes);
}

// obsolete

void sAddRefVS(sShader * vsh)
{
  vsh->AddRef();
}

void sAddRefPS(sShader * psh)
{
  psh->AddRef();
}

sShader * sCreateVS(const sU32 *data,sInt count)
{
  return sCreateShader(sSTF_VERTEX,(const sU8 *) data,count*4);
}

sShader * sCreatePS(const sU32 *data,sInt count)
{
  return sCreateShader(sSTF_PIXEL,(const sU8 *) data,count*4);
}

void sDeleteVS(sShader * &handle)
{
  handle->Release();
  handle = 0;
}

void sDeletePS(sShader * &handle)
{
  handle->Release();
  handle = 0;
}

sBool sValidVS(sShader * vsh)
{
  return vsh->CheckKind(sSTF_VERTEX);
}

sBool sValidPS(sShader * psh)
{
  return psh->CheckKind(sSTF_PIXEL);
}

/****************************************************************************/
/***                                                                      ***/
/***   sVertexFormatHandle                                                ***/
/***                                                                      ***/
/****************************************************************************/

#define MAXVFHASH 64


const sInt sVertexFormatTypeSizes[]=
{
  0,  // invalid
  8,  // sVF_F2
  12, // sVF_F3
  16, // sVF_F4
  4,  // sVF_C4,
  4,  // sVF_I4,
  4,  // sVF_S2
  8,  // sVF_S4
  4,  // sVF_H2
  8,  // sVF_H4
  4,  // sVF_F1
};

sVertexFormatHandle *VertexFormatHashTable[MAXVFHASH];

sVertexFormatHandle *sCreateVertexFormat(const sU32 *disc)
{
  sScopeMemSeg ms(sMEMSEG_HEAP);

  sInt count;
  sVertexFormatHandle *handle;
  sU32 data[32];
  sInt hash;
  static const sInt defaulttypes[32] = 
  {
    0,sVF_F3,sVF_F3,sVF_F4,
    sVF_C4,sVF_C4,0,0,
    sVF_C4,sVF_C4,sVF_C4,sVF_C4,
    sVF_F2,sVF_F2,sVF_F2,sVF_F2,
  };
  sVERIFY(sVF_USEMASK==31);

  // count elements and fill default values and copy discriptor
  // count includes trailing zero.

  for(count=0;disc[count];count++)
  {
    sVERIFY(count<31);
    data[count] = disc[count];

    if((data[count] & sVF_TYPEMASK)==0)
      data[count] |= defaulttypes[data[count]&sVF_USEMASK];
  }
  data[count++] = 0;

  // find in hashtable

  hash = sChecksumCRC32((sU8 *)data,count*4) & (MAXVFHASH-1);
  handle = VertexFormatHashTable[hash];
  while(handle)
  {
    if(handle->Count==count && sCmpMem(data,handle->Data,count*4)==0)
    {
      handle->UseCount++;
      return handle;
    }
    handle = handle->Next;
  }

  // not found, create new element

  handle = new sVertexFormatHandle;
  handle->Data = new sU32[count];
  handle->Count = count;
  handle->UseCount = 1;
  handle->AvailMask = 0;
  handle->Streams = 1;

  for(sInt i=0;i<sVF_STREAMMAX;i++)
    handle->VertexSize[i] = 0;
  handle->Next = 0;
  sCopyMem(handle->Data,data,count*4);

  // link in hashtable

  handle->Next = VertexFormatHashTable[hash];
  VertexFormatHashTable[hash] = handle;

  // prepare drawing

  handle->Create();

  // done

  return handle;
}

void sStreamVertexFormat(sWriter &s,const sVertexFormatHandle *vhandle)
{
  s | vhandle->Count;
  s.ArrayU32(vhandle->Data,vhandle->Count);
}

void sStreamVertexFormat(sReader &s,sVertexFormatHandle *&vhandle)
{
  sU32 vd[16];
  sU32 count;
  
  s | count;
  sVERIFY(count < 16);
  s.ArrayU32(vd,count);

  vhandle = sCreateVertexFormat(vd);
}

void sDestroyAllVertexFormats()
{
  sVertexFormatHandle *h,*n;
  for(sInt i=0;i<MAXVFHASH;i++)
  {
    h = VertexFormatHashTable[i];
    while(h)
    {
      n = h->Next;

      h->Destroy();

      delete[] h->Data;
      delete h;
      h = n;
    }
  }
  sClear(VertexFormatHashTable);
}

/****************************************************************************/
/***                                                                      ***/
/***   sGeometry                                                          ***/
/***                                                                      ***/
/****************************************************************************/

sGeometry::sGeometry()
{
  Flags = 0;
  Format = 0;
  IndexSize = 0;
  PrimMode = 0;
  FirstPrim = 0;
  CurrentPrim = 0;

  InitPrivate();
}

sGeometry::~sGeometry()
{
  Clear();

  ExitPrivate();
}

#if sRENDERER!=sRENDER_PS2 && sRENDERER!=sRENDER_WII
#if sRENDERER!=sRENDER_XBOX360 

void sGeometry::Clear()
{
  for(sInt i=0;i<sVF_STREAMMAX;i++)
    VertexPart[i].Clear();
  IndexPart.Clear();
}

void sGeometry::InitPrivate() {}
void sGeometry::ExitPrivate() {}
#endif

void sGeometry::Serialize(sReader &s)
{
  sInt version = s.Header(sSerId::sGeometry,1);
  if(version>0)
  {
    sInt vc,ic;
    sInt vs,is;
    sInt flags;
    sU32 *ip;
    sU32 *vp;
    sVertexFormatHandle *vhandle;

    s | vs | is |vc | ic | flags;
    sStreamVertexFormat(s,vhandle);

    flags &= sGF_PRIMMASK|sGF_INDEXMASK;
    if(!(flags & sGF_INDEX32))
      flags |= sGF_INDEX16;

    BeginLoad(vhandle,flags,sGD_STATIC,vc,ic,(void **)&vp,(void **)&ip);
    s.ArrayU8((sU8 *)vp,vs*vc);  // not endian safe
    s.Align(4);
    s.ArrayU8((sU8 *)ip,is*ic);  // not endian safe
    s.Align(4);
    EndLoad();

    s.Footer();
  }
}

/****************************************************************************/

#if sRENDERER==sRENDER_OGL2 || sRENDERER==sRENDER_PS3GL || sRENDERER==sRENDER_BLANK

void sGeometry::Init(sInt flags,sVertexFormatHandle *form)
{
  Flags = flags;
  Format = form;

  switch(Flags & sGF_INDEXMASK)
  {
    case sGF_INDEX16:  IndexSize = 2; break;
    case sGF_INDEX32:  IndexSize = 4; break;
    default: IndexSize = 0;
  }
}

void sGeometry::BeginLoadIB(sInt ic,sGeometryDuration duration,void **ip)
{
  sVERIFY(IndexSize>0)
  IndexPart.Clear();
#if sRENDERER==sRENDER_DX9
  IndexPart.Init(ic,IndexSize,duration,(Flags & sGF_INDEX32)?2:1);
#else
  IndexPart.Init(ic,IndexSize,duration,1);
#endif
  IndexPart.Lock(ip);
}

void sGeometry::EndLoadIB(sInt ic)
{
  IndexPart.Unlock(ic,IndexSize);
}

void sGeometry::BeginLoadVB(sInt vc,sGeometryDuration duration,void **vp,sInt stream)
{
  VertexPart[stream].Clear();
  VertexPart[stream].Init(vc,Format->GetSize(stream),duration,0);
  VertexPart[stream].Lock(vp);
}

void sGeometry::EndLoadVB(sInt vc,sInt stream)
{
  VertexPart[stream].Unlock(vc,Format->GetSize(stream));
}

void sGeometry::BeginLoad(sVertexFormatHandle *vf,sInt flags,sGeometryDuration duration,sInt vc,sInt ic,void **vp,void **ip)
{
  Init(flags,vf);
  BeginLoadVB(vc,duration,vp,0);
  if(ic)
    BeginLoadIB(ic,duration,ip);
  else
    IndexPart.Clear();
}

void sGeometry::EndLoad(sInt vc,sInt ic)
{
  VertexPart[0].Unlock(vc,Format->GetSize(0));
  if(IndexPart.Buffer)
    IndexPart.Unlock(ic,IndexSize);
}

// obsolete

void sGeometry::BeginLoad(sInt vc,sInt ic,sInt flags,sVertexFormatHandle *vf,void **vp,void **ip)
{
  if(!(flags & sGF_INDEX32))
    flags |= sGF_INDEX16;

  Init(flags&(sGF_PRIMMASK|sGF_INDEXMASK|sGF_INSTANCES),vf);
  BeginLoadVB(vc,sGeometryDuration(flags&3),vp,0);
  if(ic)
    BeginLoadIB(ic,sGeometryDuration((flags>>4)&3),ip);
  else
    IndexPart.Clear();
}
#endif //  renderer
#endif // sPLATFIRM!=sPLAT_PS2

/****************************************************************************/
/***                                                                      ***/
/***   sTexture                                                           ***/
/***                                                                      ***/
/****************************************************************************/

sInt sGetBitsPerPixel(sInt format)
{
  sInt BitsPerPixel = 0;

  switch(format&sTEX_FORMAT)
  {
    case sTEX_ARGB8888: BitsPerPixel = 32;  break;
    case sTEX_QWVU8888: BitsPerPixel = 32;  break;
    
    case sTEX_GR16:     BitsPerPixel = 32;  break;
    case sTEX_ARGB16:   BitsPerPixel = 64;  break;

    case sTEX_R32F:     BitsPerPixel = 32;  break;
    case sTEX_GR32F:    BitsPerPixel = 64;  break;
    case sTEX_ARGB32F:  BitsPerPixel = 128; break;
    case sTEX_R16F:     BitsPerPixel = 16;  break;
    case sTEX_GR16F:    BitsPerPixel = 32;  break;
    case sTEX_ARGB16F:  BitsPerPixel = 64;  break;

    case sTEX_A8:       BitsPerPixel =  8;  break;
    case sTEX_I8:       BitsPerPixel =  8;  break;
    case sTEX_DXT1:     BitsPerPixel =  4;  break;
    case sTEX_DXT1A:    BitsPerPixel =  4;  break;
    case sTEX_DXT3:     BitsPerPixel =  8;  break;
    case sTEX_DXT5:     BitsPerPixel =  8;  break;
    case sTEX_DXT5N:    BitsPerPixel =  8;  break;

    case sTEX_MRGB8:    BitsPerPixel = 32;  break;
    case sTEX_MRGB16:   BitsPerPixel = 64;  break;

    case sTEX_ARGB2101010: BitsPerPixel=32; break;
    case sTEX_D24S8:    BitsPerPixel = 32;  break;

//    case sTEX_A4:       BitsPerPixel =  4; break;
    case sTEX_I4:       BitsPerPixel =  4; break;
    case sTEX_IA4:      BitsPerPixel =  8; break;
    case sTEX_IA8:      BitsPerPixel = 16; break;
    case sTEX_RGB5A3:   BitsPerPixel = 16; break;

    case sTEX_INDEX4:   BitsPerPixel =  4; break;
    case sTEX_INDEX8:   BitsPerPixel =  8; break;

    case sTEX_ARGB1555: BitsPerPixel = 16; break;
    case sTEX_RGB565:   BitsPerPixel = 16; break;

    default: sVERIFYFALSE;
  }
  return BitsPerPixel;
}

/****************************************************************************/

sTextureBase::sTextureBase()
{
  Loading = -1;
  LockFlags = -1;


  Temp = 0;
  LoadBuffer = 0;

  Flags = 0;
  Mipmaps = 0;
  BitsPerPixel = 0;

  FrameRT = 0xffff;
  SceneRT = 0xffff;
#if sRENDERER==sRENDER_OGL2
  GLName = 0;
#endif
#if sRENDERER==sRENDER_DX9
  Tex2D = 0;
#endif

  NameId = 0;
  
  InitPrivate();
}

sTextureBase::~sTextureBase()
{
  sVERIFY(LoadBuffer==0);     // if this is not 0, then EndLoad() has not been called!
  ExitPrivate();
}

#if sRENDERER!=sRENDER_XBOX360 && sRENDERER!=sRENDER_WII
void sTextureBase::InitPrivate() {}
void sTextureBase::ExitPrivate() {}
#endif

void sTextureBase::Set(sInt stage)
{
  sFatal(L"can not set this texture");
}

/****************************************************************************/

sTexture2D::sTexture2D()
{
  Flags = sTEX_2D;
#if sRENDERER==sRENDER_PS3GL
  GLName = 0;
  GLFBName = 0;
#endif
#if sRENDERER==sRENDER_PS2
  Private = 0;
#endif
  SizeX = 0;
  SizeY = 0;
  Flags = 0;
  Mipmaps = 0;
  Loading = -1;
  BitsPerPixel = 0;
}

sTexture2D::sTexture2D(sInt xs,sInt ys,sU32 flags,sInt mipmaps)
{
  Flags = sTEX_2D;
#if sRENDERER==sRENDER_PS3GL
  GLName = 0;
  GLFBName = 0;
#endif
#if sRENDERER==sRENDER_PS2
  Private = 0;
#endif
  Init(xs,ys,flags|sTEX_2D,mipmaps);
}

sTexture2D::~sTexture2D()
{
  Clear();
}

void sTexture2D::Clear()
{
  sVERIFY(LoadBuffer==0);
  Destroy2();
  SizeX = 0;
  SizeY = 0;
  Flags = 0;
  Mipmaps = 0;
  Loading = -1;
  BitsPerPixel = 0;
}

void sTexture2D::Init(sInt xs, sInt ys, sInt flags,sInt mipmaps, sBool force/*=sFALSE*/)
{
  sInt realmip;
  sVERIFY(LoadBuffer==0);

  // use screen size

  if(flags & sTEX_SCREENSIZE)
  {
    sGetScreenSize(xs,ys);
  }

  // calculate "real" mipmap count

  realmip = 0;
  if(mipmaps==0) mipmaps=32;
  while(realmip<mipmaps && xs>=(1<<realmip) && ys>=(1<<realmip))
    realmip++;
  if(flags & sTEX_NOMIPMAPS)
    realmip = 1;

  // early out: nothing to do!

  if(xs==SizeX && ys==SizeY && flags==Flags && realmip==Mipmaps && !force) 
    return;

  // clear (possible) old texture

  Clear();

  // set new parameters

  SizeX = xs;
  SizeY = ys;
  Flags = (flags&~sTEX_KIND_MSK)|sTEX_2D;
  Mipmaps = realmip;
  Loading = -1;
  BitsPerPixel = sGetBitsPerPixel(Flags);

  // create resource

  Create2(flags);
}

void sTexture2D::LoadAllMipmaps(sU8 *source)
{
  sU8 *dest;
  sU8 *p;
  sInt pitch_source,pitch_dest;
  sInt xs,ys;
  sInt level;

  xs = SizeX;
  ys = SizeY;
  pitch_source = BitsPerPixel*xs/8;
  switch(Flags & sTEX_FORMAT)
  {
  case sTEX_DXT1:
  case sTEX_DXT1A:
    xs /= 4;
    ys /= 4;
    pitch_source = 8*xs;
    break;
  case sTEX_DXT3:
  case sTEX_DXT5:
  case sTEX_DXT5N:
    xs /= 4;
    ys /= 4;
    pitch_source = 16*xs;
    break;
  }
  level = 0;
  p = source;

  for(;;)
  {
    // copy mipmap

    BeginLoad(dest,pitch_dest,level);
    for(sInt y=0;y<ys;y++)
    {
      sCopyMem(dest,p,pitch_source);
      dest += pitch_dest;
      p += pitch_source;
    }
    EndLoad();

    // check if next;

    level++;
    if(level>=Mipmaps) break;
    xs = xs/2;
    ys = ys/2;
    pitch_source = pitch_source/2;
  }
}

/****************************************************************************/
/***                                                                      ***/
/***   sTextureCube                                                       ***/
/***                                                                      ***/
/****************************************************************************/

sTextureCube::sTextureCube()
{
  Flags = sTEX_CUBE;
  SizeXY = 0;
}

sTextureCube::sTextureCube(sInt dim, sInt flags,sInt mipmaps/*=0*/)
{
  Flags = sTEX_CUBE;
  SizeXY = 0;
  Init(dim,flags,mipmaps);
}

sTextureCube::~sTextureCube()
{
  Clear();
}

/****************************************************************************/

void sTextureCube::Init(sInt dim, sInt flags, sInt mipmaps/*=0*/, sBool force/*=sFALSE*/)
{
  sInt realmip;
  sVERIFY(LoadBuffer==0);

  // calculate "real" mipmap count
  realmip = 0;
  if(mipmaps==0) mipmaps=32;
  while(realmip<mipmaps && dim>=(1<<realmip))
    realmip++;
  if(flags & sTEX_NOMIPMAPS)
    realmip = 1;

  // early out: nothing to do!
  if(dim==SizeXY && flags==Flags && realmip==Mipmaps && !force) 
    return;

  // clear (possible) old texture
  Clear();

  // set new parameters
  SizeXY = dim;
  Flags = (flags&~sTEX_KIND_MSK)|sTEX_CUBE;
  Mipmaps = realmip;
  Loading = -1;
  BitsPerPixel = sGetBitsPerPixel(Flags);
  LockedFace = sTCF_NONE;

  // create resource
  Create2(flags);
}

void sTextureCube::Clear()
{
  sVERIFY(LoadBuffer==0);
  Destroy2();
  SizeXY = 0;
  Flags = 0;
  Mipmaps = 0;
  Loading = -1;
  BitsPerPixel = 0;
  LockedFace = sTCF_NONE;
}

/****************************************************************************/

void sTextureCube::LoadAllMipmaps(sU8 *data)
{
  sU8 *dest;
  sU8 *p;
  sInt pitch_source,pitch_dest;
  sInt level;

  sInt initsize = SizeXY;
  pitch_source = BitsPerPixel*initsize/8;
  switch(Flags & sTEX_FORMAT)
  {
  case sTEX_DXT1:
  case sTEX_DXT1A:
    initsize /= 4;
    pitch_source = 8*initsize;
    break;
  case sTEX_DXT3:
  case sTEX_DXT5:
  case sTEX_DXT5N:
    initsize /= 4;
    pitch_source = 16*initsize;
    break;
  }
  p = data;

  sInt pitch_init = pitch_source;

  for(sInt f=0;f<6;f++)
  {
    level = 0;
    sInt size = initsize;
    pitch_source = pitch_init;

    for(;;)
    {
      // copy mipmap
      BeginLoad(sTexCubeFace(f),dest,pitch_dest,level);
      for(sInt y=0;y<size;y++)
      {
        sCopyMem(dest,p,pitch_source);
        dest += pitch_dest;
        p += pitch_source;
      }
      EndLoad();

      // check if next;

      level++;
      if(level>=Mipmaps) break;
      size = size/2;
      pitch_source = pitch_source/2;
    }
  }
}

/****************************************************************************/


/****************************************************************************/
/***                                                                      ***/
/***   sMaterial                                                          ***/
/***                                                                      ***/
/****************************************************************************/

sMaterialEnv::sMaterialEnv()
{
  for(sInt i=0;i<sMTRLENV_LIGHTS;i++)
  {
    LightDir[i].Init(0,0,0);
    LightColor[i] = 0;
  }
  AmbientColor = 0xffffffff;

  FogMax = 1.0f;
  FogStart = 75.0f;
  FogEnd = 1024.0f;
  FogDensity = 1.0f;
  FogColor = 0xffffffff;

  DOFBlurNear = 0.0f;
  DOFFocusNear = 1.0f;
  DOFFocusFar = 20.0f;
  DOFBlurFar = 40.0f;
  DOFBlurMax = 0.75f;
}

void sMaterialEnv::Fix()
{
}

/****************************************************************************/

sMaterial::sMaterial()
{
  NameId = 0;
  Flags = sMTRL_ZON|sMTRL_CULLON;
  FuncFlags[0] = sMFF_LESSEQUAL;
  FuncFlags[1] = sMFF_ALWAYS;
  FuncFlags[2] = sMFF_LESSEQUAL;
  FuncFlags[3] = 0;
  BlendColor = sMB_OFF;
  BlendAlpha = sMB_SAMEASCOLOR;
  BlendFactor = 0xffffffff;
  StencilOps[0] = sMSO_KEEP;
  StencilOps[1] = sMSO_KEEP;
  StencilOps[2] = sMSO_KEEP;
  StencilOps[3] = sMSO_KEEP;
  StencilOps[4] = sMSO_KEEP;
  StencilOps[5] = sMSO_KEEP;
  StencilRef = 0;
  AlphaRef = 10;
  StencilMask = 0xFF;
  VertexShader = 0;
  PixelShader = 0;
#if sRENDERER==sRENDER_DX9 || sRENDERER==sRENDER_BLANK || sRENDERER==sRENDER_XBOX360
  VariantFlags = 0;
  StateVariants = 0;
  StateCount = 0;
  States = 0;
#endif
  for(sInt i=0;i<sMTRL_MAXTEX;i++)
  {
    Texture[i] = 0;
    TFlags[i] = 0;
    LodBias[i] = 0.0f;
  }

  Create2();
}

sMaterial::~sMaterial()
{
  Destroy2();
  if(VertexShader) 
    VertexShader->Release();
  if(PixelShader) 
    PixelShader->Release();
#if sRENDERER==sRENDER_DX9 || sRENDERER==sRENDER_BLANK || sRENDERER==sRENDER_XBOX360
  if(States)
  {
    for(sInt i=0;i<StateVariants;i++)
      delete[] States[i];
    delete[] VariantFlags;
    delete[] States;
    delete[] StateCount;
  }
#endif

}

void sMaterial::SetShader(sShader * vs,sShader * ps)
{
  sSetShaders(vs,ps,0,0);
}

void sMaterial::CopyBaseFrom(sMaterial *src)
{
  for(sInt i=0;i<sMTRL_MAXTEX;i++)
    Texture[i] = src->Texture[i];
  NameId = src->NameId;
  Flags = src->Flags;
  for(sInt i=0;i<sCOUNTOF(FuncFlags);i++)
    FuncFlags[i] = src->FuncFlags[i];
  for(sInt i=0;i<sMTRL_MAXTEX;i++)
    TFlags[i] = src->TFlags[i];
  BlendColor = src->BlendColor;
  BlendAlpha = src->BlendAlpha;
  BlendFactor = src->BlendFactor;
  for(sInt i=0;i<sCOUNTOF(StencilOps);i++)
    StencilOps[i] = src->StencilOps[i];
  StencilRef = src->StencilRef;
  StencilMask = src->StencilMask;
  AlphaRef = src->AlphaRef;
}

/****************************************************************************/

#if sRENDERER==sRENDER_DX9 || sRENDERER==sRENDER_BLANK || sRENDERER==sRENDER_XBOX360

void sMaterial::InitVariants(sInt max)
{
  if(StateVariants==max && States)
    return;

  //sVERIFY(StateVariants==0);
  //sVERIFY(States == 0);
    
  StateVariants = max;
  VariantFlags = new sMaterialRS[max];
  States = new sU32 *[max];
  StateCount = new sInt[max];

  for(sInt i=0;i<max;i++)
  {
    States[i] = 0;
    StateCount[i] = 0;
  }
}

void sMaterial::AllocStates(const sU32 *data,sInt count,sInt var)
{
  sVERIFY(var<StateVariants);
  sVERIFY(States[var]==0);

  States[var] = new sU32[count*2];
  StateCount[var] = count;
  sCopyMem(States[var],data,count*8);
}

void sMaterial::SetVariant(sInt var)
{
  sVERIFY(var<StateVariants);

  sU32 buffer[512];
  sU32 *data=buffer;

  VariantFlags[var] = *(sMaterialRS*)&Flags;
  AddMtrlFlags(data);
  AllocStates(buffer,(data-buffer)/2,var);
}

#endif

/****************************************************************************/
/***                                                                      ***/
/***   Viewport                                                           ***/
/***                                                                      ***/
/****************************************************************************/

sViewport::sViewport()
{
  ClipNear = 0.125f;
  ClipFar = 1024.0f;
  ZoomX = 1.0f;
  ZoomY = 1.0f;
  CenterX = 0.5f;
  CenterY = 0.5f;
  Orthogonal = sVO_PROJECTIVE;
  SetTarget(0);
};

void sViewport::SetTarget(sTexture2D *tex)
{
  if(tex)
  {
    TargetSizeX = tex->SizeX;
    TargetSizeY = tex->SizeY;
    TargetAspect = sF32(tex->SizeY)/sF32(tex->SizeX);
    Target.Init(0,0,TargetSizeX,TargetSizeY);
  }
  else
    SetTargetCurrent();
}

void sViewport::SetTarget(sTextureCube *tex, sTexCubeFace cf)
{
  if(tex)
  {
    TargetSizeX = tex->SizeXY;
    TargetSizeY = tex->SizeXY;
    TargetAspect = 1.0f;
    Target.Init(0,0,TargetSizeX,TargetSizeY);
  }
  else
    SetTargetCurrent();
}

void sViewport::SetTargetCurrent()
{
  TargetSizeX = sGFXRendertargetX;
  TargetSizeY = sGFXRendertargetY;
  TargetAspect = sGFXRendertargetAspect*sGFXRendertargetY*sF32(sGFXViewRect.SizeX())/(sF32(sGFXViewRect.SizeY())*sGFXRendertargetX);
  Target = sGFXViewRect;
}

void sViewport::SetZoom(sF32 aspect,sF32 zoom)
{
  ZoomX = zoom;
  ZoomY = zoom;
  if (aspect<1.0)
    ZoomY *= aspect;
  else
    ZoomX /= aspect;
}

void sViewport::SetZoom(sF32 zoom)
{
  SetZoom(TargetAspect,zoom);
}

void sViewport::UpdateModelMatrix(const sMatrix34 &mat)
{
  Model = mat;
  Prepare(sVUF_MODEL);
}

void sViewport::Prepare(sInt update)
{
  sMatrix44 mat;

  if(update&(sVUF_PROJ|sVUF_WINDOW))
  {
    sF32 xs = sF32(Target.SizeX())/TargetSizeX;
    sF32 ys = sF32(Target.SizeY())/TargetSizeY;
    sF32 xo = sF32(Target.CenterX()-0.5f)/TargetSizeX*2-1.0f;
    sF32 yo = sF32(Target.CenterY()-0.5f)/TargetSizeY*2-1.0f;
    switch(Orthogonal)
    {
    case sVO_PROJECTIVE:
      {
        sF32 q = 1/(ClipFar-ClipNear);
        
        mat.i.x = xs * ZoomX;
        mat.i.y = 0;
        mat.i.z = 0;
        mat.i.w = 0;

        mat.j.x = 0;
        mat.j.y = ys * ZoomY;
        mat.j.z = 0;
        mat.j.w = 0;

        mat.k.x =   xo + (CenterX-0.5f)*2*xs;
        mat.k.y = - yo + (CenterY-0.5f)*2*ys;
        mat.k.z = q*ClipFar;
        mat.k.w = 1;

        mat.l.x = 0;
        mat.l.y = 0;
        mat.l.z = -q*ClipFar*ClipNear;
        mat.l.w = 0;
      }
      break;

    case sVO_PIXELS:      // this does not respect the Target rectangle. is that a good idea? no.
      mat.i.x = 2.0f / TargetSizeX;
      mat.i.y = 0;
      mat.i.z = 0;
      mat.i.w = 0;

      mat.j.x = 0;
      mat.j.y = -2.0f / TargetSizeY;
      mat.j.z = 0;
      mat.j.w = 0;

      mat.k.x = 0;
      mat.k.y = 0;
      mat.k.z = 1;
      mat.k.w = 0;
#if sRENDERER==sRENDER_XBOX360
      mat.l.x = -1.0 + 2.0f*Target.x0 / TargetSizeX + 0.5f/TargetSizeX;
      mat.l.y =  1.0 - 2.0f*Target.y0 / TargetSizeY + 0.5f/TargetSizeX;
#else
      mat.l.x = -1.0 + 2.0f*Target.x0 / TargetSizeX;
      mat.l.y =  1.0 - 2.0f*Target.y0 / TargetSizeY;
#endif
      mat.l.z = 0;
      mat.l.w = 1;
      break;

    case sVO_SCREEN:
      mat.i.x = 2.0f * xs;
      mat.i.y = 0;
      mat.i.z = 0;
      mat.i.w = 0;

      mat.j.x = 0;
      mat.j.y = -2.0f * ys;
      mat.j.z = 0;
      mat.j.w = 0;

      mat.k.x = 0;
      mat.k.y = 0;
      mat.k.z = 1;
      mat.k.w = 0;

      mat.l.x = -sF32(Target.CenterX()*2+0.25f)/TargetSizeX;    // CHAOS: this is correct. do not change. tested for rendering exact pixels under FSAA
      mat.l.y =  sF32(Target.CenterY()*2+0.25f)/TargetSizeY;    // CHAOS: i do not understand WHY this is correct... should be +1.00f instead of +0.25f
      mat.l.z = 0;
      mat.l.w = 1;
      break;

    default:
      sVERIFYFALSE;
    }
    /*
#if sRENDERER==sRENDER_OGL2
    mat.k.z = 2*mat.k.z - 1;
    mat.l.z = 2*mat.l.z;
#endif
    */

    Proj = mat;
  }
  if(update)
  {
    View = Camera;
    View.TransR();

    ModelView = Model*View;
    ModelScreen = ModelView*Proj;
  }
}

sBool sViewport::Transform(const sVector31 &p,sInt &ix,sInt &iy) const
{
  sVector4 s = p * ModelScreen;
  if(s.z>0)
  {
    ix = sInt(((s.x/s.w)*0.5f+0.5f) * TargetSizeX) - Target.x0;
    iy = sInt((0.5f-(s.y/s.w)*0.5f) * TargetSizeY) - Target.y0; 
    return 1;
  }
  else
  {
    ix = 0;
    iy = 0;
    return 0;
  }
}


sBool sViewport::Transform(const sVector31 &p,sF32 &ix,sF32 &iy) const
{
  sVector4 s = p * ModelScreen;
  if(s.z>0)
  {
    ix = ((s.x/s.w)*0.5f+0.5f) - (sF32)Target.x0 / TargetSizeX;
    iy = (0.5f-(s.y/s.w)*0.5f) - (sF32)Target.y0 / TargetSizeY; 
    return 1;
  }
  else
  {
    ix = 0;
    iy = 0;
    return 0;
  }
}
void sViewport::MakeRayPixel(sInt ix,sInt iy,sRay &ray) const
{
  sF32 mx = (ix-Target.x0) / sF32(Target.SizeX());
  sF32 my = (iy-Target.y0) / sF32(Target.SizeY());
  mx = mx*2-1;
  my = 1-my*2;
  ray.Dir = Camera.k + Camera.i*mx/ZoomX + Camera.j*my/ZoomY;
  ray.Dir.Unit();
  ray.Start = Camera.l + ray.Dir*ClipNear;
}

void sViewport::MakeRay(sF32 mx,sF32 my,sRay &ray) const
{
  sMatrix34 invcam(Camera);
  invcam.Invert34();
  ray.Dir = Camera.k + invcam.i*mx/ZoomX + invcam.j*my/ZoomY;    // this might be wrong!
  ray.Dir.Unit();
  ray.Start = Camera.l + ray.Dir*ClipNear;
}

sInt sViewport::Visible(const sAABBox &box, sF32 clipnear, sF32 clipfar) const
{
  sMatrix34 r[2];
  sVector30 v[8];
  sVector30 t;
  sInt i;
  sU32 clip,amask,omask;

  r[0] = ModelView;
  r[1] = ModelView;
  r[0].i *= box.Min.x; r[0].i.x += ModelView.l.x;
  r[0].j *= box.Min.y; r[0].j.y += ModelView.l.y;
  r[0].k *= box.Min.z; r[0].k.z += ModelView.l.z;
  r[1].i *= box.Max.x; r[1].i.x += ModelView.l.x;
  r[1].j *= box.Max.y; r[1].j.y += ModelView.l.y;
  r[1].k *= box.Max.z; r[1].k.z += ModelView.l.z;

  v[0] = r[0].i+r[0].j+r[0].k;
  v[1] = r[0].i+r[1].j+r[0].k;
  v[2] = r[1].i+r[0].j+r[0].k;
  v[3] = r[1].i+r[1].j+r[0].k;
  v[4] = r[0].i+r[0].j+r[1].k;
  v[5] = r[0].i+r[1].j+r[1].k;
  v[6] = r[1].i+r[0].j+r[1].k;
  v[7] = r[1].i+r[1].j+r[1].k;

  amask = ~0;
  omask = 0;
  for(i=0;i<8;i++)
  {
    t = v[i];
    t.x = -t.x*ZoomX;
    t.y = -t.y*ZoomY;
    t.z = -t.z;

    clip = 0;
    if(t.z<0)
    {

      if(t.x< t.z) clip |= 0x01;
      if(t.y< t.z) clip |= 0x02;
      if(t.x>-t.z) clip |= 0x04;
      if(t.y>-t.z) clip |= 0x08;
    }
    else
    {
      if(t.x<0) clip |= 0x01;
      if(t.y<0) clip |= 0x02;
      if(t.x>0) clip |= 0x04;
      if(t.y>0) clip |= 0x08;
    }
    if(t.z>-clipnear)
      clip |= 0x10;
    if(t.z<-clipfar)
      clip |= 0x20;

    amask &= clip;
    omask |= clip;
  }
  if(amask!=0) return 0;          // total out
  if(omask==0) return 2;          // total in
  return 1;                       // part in
}

sInt sViewport::VisibleDist(const sAABBox &box,sF32 &dist) const
{
  sMatrix34 r[2];
  sVector30 v[8];
  sVector30 t;
  sInt i;
  sU32 clip,amask,omask;

  r[0] = ModelView;
  r[1] = ModelView;
  r[0].i *= box.Min.x; r[0].i.x += ModelView.l.x;
  r[0].j *= box.Min.y; r[0].j.y += ModelView.l.y;
  r[0].k *= box.Min.z; r[0].k.z += ModelView.l.z;
  r[1].i *= box.Max.x; r[1].i.x += ModelView.l.x;
  r[1].j *= box.Max.y; r[1].j.y += ModelView.l.y;
  r[1].k *= box.Max.z; r[1].k.z += ModelView.l.z;

  v[0] = r[0].i+r[0].j+r[0].k;
  v[1] = r[0].i+r[1].j+r[0].k;
  v[2] = r[1].i+r[0].j+r[0].k;
  v[3] = r[1].i+r[1].j+r[0].k;
  v[4] = r[0].i+r[0].j+r[1].k;
  v[5] = r[0].i+r[1].j+r[1].k;
  v[6] = r[1].i+r[0].j+r[1].k;
  v[7] = r[1].i+r[1].j+r[1].k;

  amask = ~0;
  omask = 0;
  dist = ClipFar;

  for(i=0;i<8;i++)
  {
    t = v[i];
    t.x = -t.x*ZoomX;
    t.y = -t.y*ZoomY;

    if(dist>t.z)
      dist = t.z;
    
    clip = 0;
    if(t.z>0)
    {

      if(t.x<-t.z) clip |= 0x01;
      if(t.y<-t.z) clip |= 0x02;
      if(t.x> t.z) clip |= 0x04;
      if(t.y> t.z) clip |= 0x08;
    }
    else
    {
      if(t.x<0) clip |= 0x01;
      if(t.y<0) clip |= 0x02;
      if(t.x>0) clip |= 0x04;
      if(t.y>0) clip |= 0x08;
    }
    if(t.z<ClipNear)
      clip |= 0x10;
    if(t.z>ClipFar)
      clip |= 0x20;

    amask &= clip;
    omask |= clip;
  }

  if(amask!=0) return 0;          // total out
  if(omask==0) return 2;          // total in
  return 1;                       // part in
}


void sViewport::Set() const
{
  sGFXMatrices[sGM_MODEL] = Model;
  sGFXMatrices[sGM_MODELVIEW] = ModelView;
  sGFXMatrices[sGM_MODELSCREEN] = ModelScreen;
  sGFXMatrices[sGM_VIEW] = View;
  sGFXMatrices[sGM_PROJ] = Proj;

  sGFXViewIsSet = 1;
  sGFXMtrlIsSet = 0;
}

/****************************************************************************/

void sGetMatrix(sMatrix44 &mat,sGetMatrixIndex index)
{
  sVERIFY(index>=0 && index<sGM_MAX);
  
  mat = sGFXMatrices[index];
}

const sMatrix44& sGetMatrix(sGetMatrixIndex index)
{
  sVERIFY(index>=0 && index<sGM_MAX);
  
  return sGFXMatrices[index];
}

void sGetMatrix(sMatrix34 &mat,sGetMatrixIndex index)
{
  sVERIFY(index>=0 && index<sGM_MAX);
  
  mat.i.x = sGFXMatrices[index].i.x;
  mat.i.y = sGFXMatrices[index].i.y;
  mat.i.z = sGFXMatrices[index].i.z;
  mat.j.x = sGFXMatrices[index].j.x;
  mat.j.y = sGFXMatrices[index].j.y;
  mat.j.z = sGFXMatrices[index].j.z;
  mat.k.x = sGFXMatrices[index].k.x;
  mat.k.y = sGFXMatrices[index].k.y;
  mat.k.z = sGFXMatrices[index].k.z;
  mat.l.x = sGFXMatrices[index].l.x;
  mat.l.y = sGFXMatrices[index].l.y;
  mat.l.z = sGFXMatrices[index].l.z;
}

void sGetMatrix(sMatrix34CM &mat,sGetMatrixIndex index)
{
  sVERIFY(index>=0 && index<sGM_MAX);

  const sMatrix44 &m=sGFXMatrices[index];
  mat.x.Init(m.i.x,m.j.x,m.k.x,m.l.x);
  mat.y.Init(m.i.y,m.j.y,m.k.y,m.l.y);
  mat.z.Init(m.i.z,m.j.z,m.k.z,m.l.z);
}

/****************************************************************************/

void sGetRendertargetSize(sInt &dx,sInt &dy)
{
  dx = sGFXViewRect.SizeX();//DXRendertargetX;
  dy = sGFXViewRect.SizeY();//DXRendertargetY;
}

sBool sIsFormatDXT(sInt format)
{
  switch(format & sTEX_FORMAT)
  {
  case sTEX_DXT1:
  case sTEX_DXT1A:
  case sTEX_DXT3:
  case sTEX_DXT5:
  case sTEX_DXT5N:
    return 1;
  default:
    return 0;
  }
}


void Render3D()
{
  sRender3DBegin();
  
  if(sGetApp())
    sGetApp()->OnPaint3D();
  
  sRender3DEnd();
}

/****************************************************************************/

void sClearCurrentCBuffers()
{
  sClear(CurrentCBs);
  sClear(CurrentCBsMask);
  sClear(CurrentCBsSlotMask);
}

void sSetCBuffers(sCBufferBase **cbuffers,sInt cbcount)
{
  static const sChar *typenam[] = { L"VS",L"PS",L"GS" };

  // set constant buffers
  for(sInt i=0;i<cbcount;i++)
  {
    sCBufferBase *cb = cbuffers[i];
    if(cb)
    {
      sVERIFY(cb->Slot<sCBUFFER_MAXSLOT*sCBUFFER_SHADERTYPES);
      if(cb != CurrentCBs[cb->Slot])
      {
        sInt type = cb->Slot>>5;
        CurrentCBsMask[type] &= ~CurrentCBsSlotMask[cb->Slot];
        if(CurrentCBsMask[type]&cb->Mask)
        {
          // handle register collision
          CurrentCBsSlotMask[cb->Slot] = 0;    // prevent detection with itself
          sBool unhandled = 1;
          for(sInt i=type*sCBUFFER_MAXSLOT;i<(type+1)*sCBUFFER_MAXSLOT;i++)
          {
            if(cb->Mask & CurrentCBsSlotMask[i])
            {
              // handled cbuffer collisions are ok (in most cases)
              //sDPrintF(L"register collision %s, %02d: %03d..%03d vs %02d: %016x\n",
              //  typenam[type],
              //  cb->Slot,cb->RegStart,cb->RegStart+cb->RegCount-1,
              //  i,CurrentCBsSlotMask[i]);
              CurrentCBsMask[type] &= ~CurrentCBsSlotMask[i];
              CurrentCBsSlotMask[i] = 0;
              CurrentCBs[i] = 0;
              unhandled = 0;
            }
          }
          if(unhandled)
          {
            sDPrintF(L"register collision %s, %02d: %03d..%03d\n",
              typenam[type],
              cb->Slot,cb->RegStart,cb->RegStart+cb->RegCount-1);
          }
        }
        cb->SetRegs();
        CurrentCBs[cb->Slot] = cb;
        CurrentCBsMask[type] |= cb->Mask;
        CurrentCBsSlotMask[cb->Slot] = cb->Mask;
      }
    }
  }
}

void sCBufferBase::Modify()
{
  if(DataPtr)
    *DataPtr = DataPersist;
  if(CurrentCBs[Slot]==this)
    CurrentCBs[Slot] = 0;
}

void sCBufferBase::SetCfg(sInt slot, sInt start, sInt count)
{
  Slot = slot;
  RegStart = start;
  RegCount = count;
  Mask = ((sU64(1)<<(sClamp((RegCount+3)/4-32,0,32)))-1)<<32;
  Mask |= (sU64(1)<<(sClamp((RegCount+3)/4,0,32)))-1;
  Mask = Mask<<(RegStart/4);
}

/****************************************************************************/
/***                                                                      ***/
/***   Misc                                                               ***/
/***                                                                      ***/
/****************************************************************************/

/****************************************************************************/
