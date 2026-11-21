/****************************************************************************/
/***                                                                      ***/
/***   (C) 2005 Dierk Ohlerich, all rights reserved                       ***/
/***                                                                      ***/
/****************************************************************************/

#ifndef HEADER_ALTONA_BASE_SERIALIZE
#define HEADER_ALTONA_BASE_SERIALIZE

#ifndef __GNUC__
#pragma once
#endif

#include "base/types.hpp"
class sFile;

/****************************************************************************/
/***                                                                      ***/
/***   Serialisation, new endian-safe system                              ***/
/***                                                                      ***/
/****************************************************************************/

class sWriter
{
  sU8 *Buffer;
  sInt BufferSize;
  sU8 *CheckEnd;
  sFile *File;
  sBool Ok;
  sU8 *Data;

  // object serialization
  struct sWriteLink *WOH[256]; 
  struct sWriteLink *WOL;
  sInt WOCount;

public:

  sWriter();
  ~sWriter();
  void Begin(sFile *file);
  sBool End();
  void Check();
  sBool IsOk() { return Ok; }
  void Fail() { Ok = 0; }
  sBool IsReading() { return 0; }
  sBool IsWriting() { return 1; }

  sInt Header(sU32 id,sInt currentversion);
  sU32 PeekHeader() { return 0 /*sSerId::Error*/; }
  sBool PeekFooter() { return sFALSE; }
  void Footer();
  void Skip(sInt bytes);
  void Align(sInt alignment=4);
  sInt RegisterPtr(void *);
  sBool IsRegistered(void *);
  void VoidPtr(const void *obj);
  template <typename T> void Ptr(const T *obj) { VoidPtr((const void *)obj); }
  template <typename T> void Enum(const T val) { U32(val); }
  sBool If(sBool c) { U32(c); return c; }
  template <typename T> void Index(T *e,sStaticArray<T *> &a) { U32(e - a[0]); }
  template <typename T> void Once(T *obj) { if(If(obj && !IsRegistered(obj))){RegisterPtr(obj);obj->Serialize(*this);} Ptr(obj); }
  template <typename T> void OnceRef(T *obj) { if(If(obj && !IsRegistered(obj))){RegisterPtr(obj);obj->Serialize(*this);} Ptr(obj); }
  void Bits(sInt *a,sInt *b,sInt *c,sInt *d,sInt *e,sInt *f,sInt *g,sInt *h);

  void U8(sU8 v)     { Data[0] = v;                           Data+=1; }
  void U16(sU16 v)   { sUnalignedLittleEndianStore16(Data,v); Data+=2; }
  void U32(sU8 &v)   { sUnalignedLittleEndianStore32(Data,v); Data+=4; } // useful variant for streaming 8 bit variables as 32 bit stream
  void U32(sU32 v)   { sUnalignedLittleEndianStore32(Data,v); Data+=4; }
  void U64(sU64 v)   { sUnalignedLittleEndianStore64(Data,v); Data+=8; }
  void ArrayU8(const sU8 *ptr,sInt count);
  void ArrayU16(const sU16 *ptr,sInt count);
  void ArrayU16Align4(const sU16 *ptr,sInt count);
  void ArrayU32(const sU32 *ptr,sInt count);
  void ArrayU64(const sU64 *ptr,sInt count);
  void String(const sChar *v);

  void S8(sS8 v)     { U8((sU8) v); }
  void S16(sS16 v)    { U16((sU16) v); }
  void S32(sS32 v)    { U32((sU32) v); }
  void S64(sS64 v)    { U64((sU64) v); }
  void F32(sF32 v)   { U32(*((sU32 *) &v)); }
  void F64(sF64 v)   { U64(*((sU64 *) &v)); }
  void ArrayS8(const sS8 *ptr,sInt count)     { ArrayU8((const sU8 *)ptr,count);  }
  void ArrayS16(const sS16 *ptr,sInt count)   { ArrayU16((const sU16 *)ptr,count); }
  void ArrayS32(const sS32 *ptr,sInt count)   { ArrayU32((const sU32 *)ptr,count); }
  void ArrayS64(const sS64 *ptr,sInt count)   { ArrayU64((const sU64 *)ptr,count); }
  void ArrayF32(const sF32 *ptr,sInt count)   { ArrayU32((const sU32 *)ptr,count); }
  void ArrayF64(const sF64 *ptr,sInt count)   { ArrayU64((const sU64 *)ptr,count); }

  template <class Type> void Array(const sStaticArray<Type> &a) { S32(a.GetCount()); }
  template <class Type> void ArrayNew(const sStaticArray<Type *> &a) { S32(a.GetCount()); }
};

class sReader
{
  sFile *File;
  const sU8 *Map;
  sU8 *Buffer;
  sInt BufferSize;
  const sU8 *Data;
  const sU8 *CheckEnd;
  sU8 *LoadEnd;
  sS64 ReadLeft;
  sBool Ok;

  void **ROL;
  sInt ROCount;

public:
  sBool DontMap;          // for debug purposes
  sReader();
  ~sReader();
  void Begin(sFile *file);
  sBool End();
  void Check();
  sBool IsOk() { return Ok; }
  void Fail() { Ok = 0; }
  const sU8 *GetPtr(sInt bytes); // get acces to data and advance pointer
  sBool IsReading() { return 1; }
  sBool IsWriting() { return 0; }
  void DebugPeek(sInt count);

  sInt Header(sU32 id,sInt currentversion);
  sU32 PeekHeader();
  sBool PeekFooter();
  void Footer();
  void Skip(sInt bytes);
  void Align(sInt alignment=4);
  sInt RegisterPtr(void *);
  void VoidPtr(void *&obj);
  template <typename T> void Ptr(T *&obj) { VoidPtr((void *&)obj); }
  template <typename T> void Enum(T &val) { sU32 v; U32(v); val=(T)v; }
  sBool If(sBool c) { sU32 c1; U32(c1); return c1; }
  template <typename T> void Index(T *&e,sStaticArray<T *> &a) { sU32 n; U32(n); e = a[n]; }
  template <typename T> void Once(T *&obj) { if(If(0)){obj=new T;RegisterPtr(obj);obj->Serialize(*this); } Ptr(obj); }
  template <typename T> void OnceRef(T *&obj) { if(If(0)){obj=new T;RegisterPtr(obj);obj->Serialize(*this);Ptr(obj);} else {Ptr(obj);obj->AddRef();} }
  void Bits(sInt *a,sInt *b,sInt *c,sInt *d,sInt *e,sInt *f,sInt *g,sInt *h);

  void U8(sU8 &v)    { v = Data[0];                          Data+=1; }
  void U8(sInt &v)   { v = Data[0];                          Data+=1; }
  void U16(sU16 &v)  { sUnalignedLittleEndianLoad16(Data,v); Data+=2; }
  void U16(sInt &v)  { v = Data[0]|(Data[1]<<8);             Data+=2; }
  void U32(sU8 &v)   { v = Data[0];                          Data+=4; } // useful variant for streaming 8 bit variables as 32 bit stream
  void U32(sU32 &v)  { sUnalignedLittleEndianLoad32(Data,v); Data+=4; }
  void U64(sU64 &v)  { sUnalignedLittleEndianLoad64(Data,v); Data+=8; }
  void ArrayU8(sU8 *ptr,sInt count);
  void ArrayU16(sU16 *ptr,sInt count);
  void ArrayU32(sU32 *ptr,sInt count);
  void ArrayU64(sU64 *ptr,sInt count);
  void String(sChar *v,sInt maxsize);

  void S8(sS8 &v)     { U8((sU8 &) v); }
  void S8(sInt &v)     { U8((sInt &) v); }
  void S16(sS16 &v)    { U16((sU16 &) v); }
  void S16(sInt &v)    { U16((sInt &) v); }
  void S32(sS32 &v)    { U32((sU32 &) v); }
  void S64(sS64 &v)    { U64((sU64 &) v); }
  void F32(sF32 &v)   { U32(*((sU32 *) &v)); }
  void F64(sF64 &v)   { U64(*((sU64 *) &v)); }
  void ArrayS8(sS8 *ptr,sInt count)     { ArrayU8((sU8 *)ptr,count);  }
  void ArrayS16(sS16 *ptr,sInt count)   { ArrayU16((sU16 *)ptr,count); }
  void ArrayS32(sS32 *ptr,sInt count)   { ArrayU32((sU32 *)ptr,count); }
  void ArrayS64(sS64 *ptr,sInt count)   { ArrayU64((sU64 *)ptr,count); }
  void ArrayF32(sF32 *ptr,sInt count)   { ArrayU32((sU32 *)ptr,count); }
  void ArrayF64(sF64 *ptr,sInt count)   { ArrayU64((sU64 *)ptr,count); }

  template <class Type> void Array(sStaticArray<Type> &a) 
  { sInt max; S32(max); sVERIFY(a.GetCount()==0); a.Resize(max); } 
  template <class Type> void ArrayNew(sStaticArray<Type *> &a) 
  { sInt max; S32(max); sVERIFY(a.GetCount()==0); a.Resize(max); 
    for(sInt i=0;i<max;i++) { a[i]=new Type; } }
};

inline sWriter& operator| (sWriter &s,sS32  v)  { s.S32(v); return s; } 
inline sReader& operator| (sReader &s,sS32 &v)  { s.S32(v); return s; }
inline sWriter& operator| (sWriter &s,sU32  v)  { s.U32(v); return s; }
inline sReader& operator| (sReader &s,sU32 &v)  { s.U32(v); return s; }
inline sWriter& operator| (sWriter &s,sF32  v)  { s.F32(v); return s; }
inline sReader& operator| (sReader &s,sF32 &v)  { s.F32(v); return s; }
inline sWriter& operator| (sWriter &s,sS64  v)  { s.S64(v); return s; }
inline sReader& operator| (sReader &s,sS64 &v)  { s.S64(v); return s; }
inline sWriter& operator| (sWriter &s,sU64  v)  { s.U64(v); return s; }
inline sReader& operator| (sReader &s,sU64 &v)  { s.U64(v); return s; }
inline sWriter& operator| (sWriter &s,sF64  v)  { s.F64(v); return s; }
inline sReader& operator| (sReader &s,sF64 &v)  { s.F64(v); return s; }
inline sWriter& operator| (sWriter &s,const sChar *v) { s.String(v); return s; }
inline sReader& operator| (sReader &s,const sStringDesc &desc) { s.String(desc.Buffer,desc.Size); return s; }
inline sWriter& operator| (sWriter &s,const sRect &v) { s | v.x0 | v.y0 | v.x1 | v.y1; return s; }
inline sReader& operator| (sReader &s,      sRect &v) { s | v.x0 | v.y0 | v.x1 | v.y1; return s; }
inline sWriter& operator| (sWriter &s,const sFRect &v) { s | v.x0 | v.y0 | v.x1 | v.y1; return s; }
inline sReader& operator| (sReader &s,      sFRect &v) { s | v.x0 | v.y0 | v.x1 | v.y1; return s; }

template <class Type> inline sWriter& operator| (sWriter &s,Type *a) { a->Serialize(s); return s; }
template <class Type> inline sReader& operator| (sReader &s,Type *a) { a->Serialize(s); return s; }

/****************************************************************************/
/***                                                                      ***/
/***   official registry for serialization ids                            ***/
/***                                                                      ***/
/****************************************************************************/
/***                                                                      ***/
/***  - always state the enum-values explicitly!                          ***/
/***  - don't leave gaps.                                                 ***/
/***  - if you remove a class, don't delete it, comment it out!           ***/
/***  - mark id's which are reserved but not yet used, so they can be     ***/
/***    safely reassigned.                                                ***/
/***                                                                      ***/
/****************************************************************************/

namespace sSerId
{
  enum Users_
  {
    Altona                = 0x00010000,
    TC3                   = 0x00020000,
    Farbrausch            = 0x00030000,
    Chaos                 = 0x00040000,
    Werkkzeug4            = 0x00050000,
  };

  enum Altona_
  {
    Error                 = 0x00000000,
    sImage                = 0x00010001,
    sTexture              = 0x00010002,
    sGeometry             = 0x00010003,
    sImageData            = 0x00010004,
    sColorGradient        = 0x00010005,
  };

  enum TC3_
  {
    sToolMaterial         = 0x00020001,
    sToolMesh             = 0x00020002,
    sToolSkeleton         = 0x00020003,
    sToolLibrary          = 0x00020004,
    sToolTexture          = 0x00020005,
    sToolLight            = 0x00020006,
    sEngMaterial          = 0x00020007,
    sEngLibrary           = 0x00020009,
    SandboxDocument       = 0x00020009,
    SandboxPainter        = 0x0002000a,
    SandboxMaterials      = 0x0002000b, // unused???
    sKDTree               = 0x0002000c,
    SandboxPlacer         = 0x0002000d,
    EntityPlacer          = 0x0002000e,
    EntityMeshGen         = 0x0002000f,
    sEngSkeleton          = 0x00020010,
    EntityLibrary         = 0x00020011,
    EntityLight           = 0x00020012,
    sEngLight             = 0x00020013,
    SandboxParticles      = 0x00020014,
    sEngCubeLights        = 0x00020015,
    sKDToolLink           = 0x00020016,
    sToolLightmaps        = 0x00020017,
    sToolPackage          = 0x00020018,
    sEngPainterFont       = 0x00020019,
    sEngPackage           = 0x0002001a,
    sGeometryWii          = 0x0002001b,
    sCubeEffect           = 0x0002001c, // example effect
    sEffectLoader         = 0x0002001d,
    Wz4EditOptions        = 0x0002001e,
    Wz4DocOptions         = 0x0002001f,
    sAudioSoundBank       = 0x00020020,
    sLineEffect           = 0x00020021, // example effect
    sTextureWii           = 0x00020022,
    BuildCache            = 0x00020023,
    sAudienceEffect       = 0x00020024,
    sIppEffect            = 0x00020025,
    sBystanderEffect      = 0x00020026,
    sImgBlobExtract       = 0x00020027,
    sSplinedObjectEffect  = 0x00020028,
    sAudience2Effect      = 0x00020029,
    sParticle2System      = 0x0002002a,
    sParticle2Emitter     = 0x0002002b,
    sParticle2Trigger     = 0x0002002c,
    sEffectLightBundle    = 0x0002002d,
    sLensFlareEffect      = 0x0002002e,

    sBodyMixer            = 0x00020030,
  };

  // 0x80000000 .. 0xffffffff -> free for random numbers...
};

/****************************************************************************/


// HEADER_ALTONA_BASE_SERIALIZE
#endif


