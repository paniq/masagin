/****************************************************************************/
/***                                                                      ***/
/***   (C) 2005 Dierk Ohlerich, all rights reserved                       ***/
/***                                                                      ***/
/****************************************************************************/

#include "base/serialize.hpp"
#include "base/system.hpp"

static const sInt sSerMaxBytes = 0x10000;
#if sPLATFORM==sPLAT_PS2
static const sInt sSerMaxAlign = 2048;      // whole cd sectors!
#else
static const sInt sSerMaxAlign = 16;        // just good alignment
#endif

/****************************************************************************/
/***                                                                      ***/
/***   Streaming                                                          ***/
/***                                                                      ***/
/****************************************************************************/

static const sInt sSerObjectMax = 0x4000;

struct sWriteLink
{
  const void *Ptr;
  sWriteLink *Next;
};

#define STATICMEM (sPLATFORM!=sPLAT_WINDOWS && sPLATFORM!=sPLAT_LINUX && sPLATFORM!=sPLAT_PS2)

#if STATICMEM

static sWriteLink sSerWOL[sSerObjectMax];
static void *sSerROL[sSerObjectMax];
static const sInt sSerBufferSize = sSerMaxBytes*3+sSerMaxAlign;
static sU8 sSerBuffer[sSerBufferSize];
static sInt sSerBufferUsed = 0;

#endif 

/****************************************************************************/
/****************************************************************************/
/***                                                                      ***/
/***   Serialisation, new endian-safe system                              ***/
/***                                                                      ***/
/****************************************************************************/
/****************************************************************************/

sWriter::sWriter()
{
  CheckEnd = 0;
  File = 0;
  Ok = 1;
  Data = 0;
  Buffer = 0;
  BufferSize = 0;

#if STATICMEM
  WOL = sSerWOL;
#else
  WOL = new sWriteLink[sSerObjectMax];
#endif
}

sWriter::~sWriter()
{
#if !STATICMEM
  delete[] WOL;
#endif
}

void sWriter::Begin(sFile *file)
{
  File = file;
#if !STATICMEM
  BufferSize = sSerMaxBytes*2+sSerMaxAlign;
  Buffer = new sU8[BufferSize];
#else
  sVERIFY(!sSerBufferUsed);   // multiple file reader/writers not supported with STATICMEM
  BufferSize = sSerBufferSize;
  Buffer = sSerBuffer;
  sSerBufferUsed = sTRUE;   
#endif

  Data = Buffer;
  CheckEnd = Buffer+sSerMaxBytes;
  if(!File)
    Ok = 0;
  sVERIFY((sDInt(Buffer)&(sSerMaxAlign-1))==0);

  WOCount = 1;
  sClear(WOH);
}

sBool sWriter::End()
{
  if(!File->Write(Buffer,Data-Buffer))
    Ok = 0;
#if !STATICMEM
  delete[] Buffer;
#else
  sSerBufferUsed = sFALSE;
#endif
  return Ok;
}

void sWriter::Check()
{
  if(!Ok)
  {
    Data = Buffer;
  }
  else
  {
    if(Data>=CheckEnd)
    {
      sDInt bytes = (Data-Buffer) & ~(sSerMaxAlign-1);
      sDInt left = (Data-Buffer) - bytes;
      sVERIFY(bytes>0);
      File->Write(Buffer,bytes);
      sCopyMem(Buffer,Buffer+bytes,left);
      Data -= bytes;
    }
  }
}

/****************************************************************************/

sInt sWriter::Header(sU32 id,sInt currentversion)
{
  Align();
  Check();
  U32(sMAKE4('<','<','<','<'));
  U32(id);
  U32(currentversion);
  return currentversion;
}

void sWriter::Footer()
{
  Check();
  U32(sMAKE4('>','>','>','>'));
}

void sWriter::Skip(sInt bytes)
{
  sInt chunk;
  Check();
  while(bytes>0)
  {
    chunk = sMin(sSerMaxBytes/1,bytes);
    for(sInt i=0;i<chunk;i++)
      U8(0);
    Check();
    bytes -= chunk;
  }
}

void sWriter::Align(sInt alignment)
{
  while(sDInt(Data)&(alignment-1))
    U8(0);
}

sInt sWriter::RegisterPtr(void *obj)
{
  sVERIFY(WOCount<sSerObjectMax);
  // test if pointer was already serialized
  sDInt hash = sDInt(obj);
  hash = (hash & 0xffff) ^ ((hash>>16)&0xffff);
  hash = (hash & 0x00ff) ^ ((hash>> 8)&0x00ff);

  WOL[WOCount].Ptr = obj;
  WOL[WOCount].Next = WOH[hash];
  WOH[hash] = &WOL[WOCount];

  return WOCount++;
}

sBool sWriter::IsRegistered(void *obj)
{
  sWriteLink *e;

  if(obj==0)
  {
    U32(0);
    return 1;       // zero pointers are always registered!
  }

  sDInt hash = sDInt(obj);
  hash = (hash & 0xffff) ^ ((hash>>16)&0xffff);
  hash = (hash & 0x00ff) ^ ((hash>> 8)&0x00ff);

  e = WOH[hash];
  while(e)
  {
    if(e->Ptr == obj)
    {
      sInt i = sInt(e-WOL);
      sVERIFY(i>=1 && i<WOCount);
      return 1;
    }
    e = e->Next;
  }

  return 0;
}

void sWriter::VoidPtr(const void *obj)
{
  sWriteLink *e;

  if(obj==0)
  {
    U32(0);
    return;
  }

  sDInt hash = sDInt(obj);
  hash = (hash & 0xffff) ^ ((hash>>16)&0xffff);
  hash = (hash & 0x00ff) ^ ((hash>> 8)&0x00ff);

  e = WOH[hash];
  while(e)
  {
    if(e->Ptr == obj)
    {
      sInt i = sInt(e-WOL);
      sVERIFY(i>=1 && i<WOCount);
      U32(i);
      return;
    }
    e = e->Next;
  }
  sFatal(L"tried to write unregistered object %08x",(sDInt)obj);
}

void sWriter::Bits(sInt *a,sInt *b,sInt *c,sInt *d,sInt *e,sInt *f,sInt *g,sInt *h)
{
  sU8 bits = 0;
  if(a) bits |= (*a&0x01)<<0;
  if(b) bits |= (*b&0x01)<<1;
  if(c) bits |= (*c&0x01)<<2;
  if(d) bits |= (*d&0x01)<<3;
  if(e) bits |= (*e&0x01)<<4;
  if(f) bits |= (*f&0x01)<<5;
  if(g) bits |= (*g&0x01)<<6;
  if(h) bits |= (*h&0x01)<<7;
  U8(bits);
}

/****************************************************************************/

void sWriter::ArrayU8(const sU8 *ptr,sInt count)
{
  sInt chunk;
  Check();
  while(count>0)
  {
    chunk = sMin(sSerMaxBytes/1,count);
    for(sInt i=0;i<chunk;i++)
      U8(*ptr++);
    Check();
    count -= chunk;
  }
}

void sWriter::ArrayU16(const sU16 *ptr,sInt count)
{
  sInt chunk;
  Check();
  while(count>0)
  {
    chunk = sMin(sSerMaxBytes/2,count);
    for(sInt i=0;i<chunk;i++)
      U16(*ptr++);
    Check();
    count -= chunk;
  }
}

void sWriter::ArrayU16Align4(const sU16 *ptr,sInt count)
{
  sInt chunk;
  Check();
  while(count>0)
  {
    chunk = sMin(sSerMaxBytes/2,count);
    for(sInt i=0;i<chunk;i++)
      U16(*ptr++);
    if(chunk & 1)
    {
      sU16 pad=0;
      U16(pad);
    }
    Check();
    count -= chunk;
  }
}

void sWriter::ArrayU32(const sU32 *ptr,sInt count)
{
  sInt chunk;
  Check();
  while(count>0)
  {
    chunk = sMin(sSerMaxBytes/4,count);
    for(sInt i=0;i<chunk;i++)
      U32(*ptr++);
    Check();
    count -= chunk;
  }
}

void sWriter::ArrayU64(const sU64 *ptr,sInt count)
{
  sInt chunk;
  Check();
  while(count>0)
  {
    chunk = sMin(sSerMaxBytes/8,count);
    for(sInt i=0;i<chunk;i++)
      U64(*ptr++);
    Check();
    count -= chunk;
  }
}

void sWriter::String(const sChar *v)
{
  sInt len = sGetStringLen(v);
  U32(len);
  ArrayU16((sU16 *)v,len);
  Align();
}

/****************************************************************************/
/****************************************************************************/

sReader::sReader()
{
  File = 0;
  Map = 0;
  Buffer = 0;
  BufferSize = 0;
  ReadLeft = 0;
  Ok = 0;
  Data = 0;
  CheckEnd = 0;
  LoadEnd = 0;
  DontMap = 1;

  ROCount = 0;

#if STATICMEM
  ROL = sSerROL;
#else
  ROL = new void *[sSerObjectMax];
#endif
}

sReader::~sReader()
{
#if !STATICMEM
  sDeleteArray(Buffer);
  delete[] ROL;
#endif
}

void sReader::Begin(sFile *file)
{
  File = file;
  ReadLeft = File->GetSize();
  Map = 0;
  Ok = 1;     // Ok = 1 before first check, otherwise check fails
#if !STATICMEM
  if(!DontMap)
    Map = File->MapAll();
  if(Map==0)
  {
    BufferSize = sSerMaxBytes*3+sSerMaxAlign;
    Buffer = (sU8 *)sAllocMem(BufferSize,0,64,sGetConstructorMemSegment());
    Data = CheckEnd = LoadEnd = Buffer;
    Check();
  }
  else
  {
    Data = Map;
  }
#else
  sVERIFY(!sSerBufferUsed);   // multiple file reader/writers not supported with STATICMEM
  sSerBufferUsed = sTRUE;
  BufferSize = sSerBufferSize;
  Buffer = sSerBuffer;
  Data = CheckEnd = LoadEnd = Buffer;
  Check();
#endif

  ROCount = 1;
  ROL[0] = 0;
}

sBool sReader::End()
{
#if !STATICMEM
  sDeleteArray(Buffer);
#else
  sSerBufferUsed = sFALSE;
#endif
  File = 0;
  Map = 0;
  Buffer = 0;
  BufferSize = 0;
  ReadLeft = 0;

  return Ok;
}

void sReader::Check()
{
  if(Map)
  {
  }
  else if(!Ok)
  {
    Data = Buffer;
  }
  else if(Data>=CheckEnd)
  {
    sDInt remove = (Data-Buffer) & ~(sSerMaxAlign-1);
    sDInt copy = (LoadEnd-Buffer) - remove;
    sCopyMem(Buffer,Buffer+remove,copy);
    Data -= remove;
    LoadEnd -= remove;
    sDInt load = Buffer+BufferSize-LoadEnd;
    if(load>ReadLeft) 
      load = ReadLeft;
    if(load>0)
    {
      if(!File->Read(LoadEnd,load))
        Ok = 0;
      LoadEnd += load;
      ReadLeft -= load;
    }
    CheckEnd = Buffer+BufferSize-sSerMaxBytes-sSerMaxAlign;
  }
}

void sReader::DebugPeek(sInt count)
{
  const sU32 *ptr = (const sU32 *)Data;
  for(sInt i=0;i<count;i++)
    sDPrintF(L"%08x ",ptr[i]);
  sDPrintF(L"\n");
}

/****************************************************************************/

sInt sReader::Header(sU32 id,sInt currentversion)
{
  sU32 value;
  sInt version;
  Check();
  Align();
  U32(value); if(value!=sMAKE4('<','<','<','<')) Ok  = 0;
  U32(value); if(value!=id) Ok = 0;
  S32(version); if(version>currentversion) Ok = 0;
  return Ok ? version : 0;
}

sU32 sReader::PeekHeader()
{
  sU32 value;
  Check();
  const sU8 *tmp = Data;
  Align();
  U32(value); if(value!=sMAKE4('<','<','<','<')) Ok  = 0;
  U32(value);
  Data = tmp;
  return value;
}

void sReader::Footer()
{
  sU32 v;
  Check();
  U32(v); if(v!=sMAKE4('>','>','>','>')) Ok = 0;
}

sBool sReader::PeekFooter()
{
  const sU8 *tmp = Data;
  sU32 v;
  Check();
  sBool result = sFALSE;
  U32(v);
  if(v==sMAKE4('>','>','>','>'))
    result = sTRUE;
  Data = tmp;
  return result;
}

void sReader::Skip(sInt bytes)
{
  sInt chunk;
  Check();
  while(bytes>0)
  {
    chunk = sMin(sSerMaxBytes/1,bytes);
    Data += chunk;
    Check();
    bytes -= chunk;
  }
}

const sU8 *sReader::GetPtr(sInt bytes)
{
  const sU8 *result;
  Check();
  result = Data;
  Data += bytes;
  Check();
  return result;
}

void sReader::Align(sInt alignment)
{
  sDInt misalign = sDInt(Data)&(alignment-1);
  if(misalign>0)
    Data += alignment-misalign;
  sVERIFY((sDInt(Data) & (alignment-1)) == 0);
}

sInt sReader::RegisterPtr(void *obj)
{
  if(obj==0)
    return 0;
  sVERIFY(ROCount<sSerObjectMax);
  ROL[ROCount] = obj;
  return ROCount++;
}

void sReader::VoidPtr(void *&obj)
{
  sInt i;
  S32(i);
  sVERIFY(i>=0 && i<ROCount);
  obj = ROL[i];
}

void sReader::Bits(sInt *a,sInt *b,sInt *c,sInt *d,sInt *e,sInt *f,sInt *g,sInt *h)
{
  sU8 bits;
  U8(bits);
  if(a) *a = (bits>>0)&0x01;
  if(b) *b = (bits>>1)&0x01;
  if(c) *c = (bits>>2)&0x01;
  if(d) *d = (bits>>3)&0x01;
  if(e) *e = (bits>>4)&0x01;
  if(f) *f = (bits>>5)&0x01;
  if(g) *g = (bits>>6)&0x01;
  if(h) *h = (bits>>7)&0x01;
}

/****************************************************************************/

void sReader::ArrayU8(sU8 *ptr,sInt count)
{
  sInt chunk;
  Check();
  while(count>0)
  {
    chunk = sMin(sSerMaxBytes/1,count);
    for(sInt i=0;i<chunk;i++)
      U8(*ptr++);
    Check();
    count -= chunk;
  }
}

void sReader::ArrayU16(sU16 *ptr,sInt count)
{
  sInt chunk;
  Check();
  while(count>0)
  {
    chunk = sMin(sSerMaxBytes/2,count);
    for(sInt i=0;i<chunk;i++)
      U16(*ptr++);
    Check();
    count -= chunk;
  }
}

void sReader::ArrayU32(sU32 *ptr,sInt count)
{
  sInt chunk;
  Check();
  while(count>0)
  {
    chunk = sMin(sSerMaxBytes/4,count);
    for(sInt i=0;i<chunk;i++)
      U32(*ptr++);
    Check();
    count -= chunk;
  }
}

void sReader::ArrayU64(sU64 *ptr,sInt count)
{
  sInt chunk;
  Check();
  while(count>0)
  {
    chunk = sMin(sSerMaxBytes/8,count);
    for(sInt i=0;i<chunk;i++)
      U64(*ptr++);
    Check();
    count -= chunk;
  }
}

void sReader::String(sChar *v,sInt maxsize)
{
  sInt len;
  maxsize--;
  S32(len);
  if(len<=maxsize)
  {
    ArrayU16((sU16 *)v,len);
    v[len] = 0;
  }
  else
  {
    ArrayU16((sU16 *)v,maxsize);
    v[maxsize] = 0;
    Skip((len-maxsize)*sizeof(sU16));
  }
  Align();
}

/****************************************************************************/
/****************************************************************************/
