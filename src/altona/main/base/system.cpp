#include "base/system.hpp"

/****************************************************************************/

sInt sSystemFlags = 0;
sInt sExitFlag = 0;

sApp *sAppPtr = 0;
sHooks *sFrameHook = 0;
sHooks *sNewDeviceHook = 0;

void (*sNetPrint)(const sChar *text) = sNULL;
void (*sRedirectStdOut)(const sChar *text) = sNULL;

sBool sAllocMemEnabled = sTRUE;

/****************************************************************************/
/****************************************************************************/
/***                                                                      ***/
/***   Basic Stuff                                                        ***/
/***                                                                      ***/
/****************************************************************************/
/****************************************************************************/

sApp::~sApp()
{
}

void sApp::OnInput(const sInputEvent &)
{
}

void sApp::OnPaint2D(const sRect &,const sRect &)
{
}

void sApp::OnPaint3D()
{
}

void sApp::OnPrepareFrame()
{
}

void sApp::OnEvent(sInt)
{
}

void sApp::OnInit()
{
}

/****************************************************************************/

void sVerifyFalse(const sChar *file,sInt line)
{
  sFatal(sTXT("%s(%d) : assertion"),file,line);
}

void sExit()
{
  if(!sExitFlag)
    sExitFlag = 1;
}

void sRestart()
{
  sExitFlag = 2;
}

sInt sGetSystemFlags()
{
  return sSystemFlags;
}

void sSetApp(sApp *app)
{
  sAppPtr = app;
  if (sAppPtr)
    sAppPtr->OnInit();
}

sApp *sGetApp()
{
  return sAppPtr;
}

/****************************************************************************/

static struct sSubsystem
{
  sInt Priority;
  const sChar *Name;
  void (*Init)();
  void (*Exit)();
  sBool Running;
} Subsystems[256];
static sInt SubsystemCount;
static sInt SubsystemPriority;

void sAddSubsystem(const sChar *name,sInt priority,void (*init)(),void (*exit)())
{
  sVERIFY(SubsystemCount<sCOUNTOF(Subsystems));
  Subsystems[SubsystemCount].Name = name;
  Subsystems[SubsystemCount].Priority = priority;
  Subsystems[SubsystemCount].Init = init;
  Subsystems[SubsystemCount].Exit = exit;
  Subsystems[SubsystemCount].Running = 0;
  if(Subsystems[SubsystemCount].Priority < SubsystemPriority)
  {
    if(Subsystems[SubsystemCount].Init)
      (*Subsystems[SubsystemCount].Init)();
    Subsystems[SubsystemCount].Running = 1;
  }
  SubsystemCount++;
}

void sSetRunlevel(sInt priority)
{
  // sort runlevels

  for(sInt i=0;i<SubsystemCount-1;i++)
    for(sInt j=i+1;j<SubsystemCount;j++)
      if(Subsystems[i].Priority > Subsystems[j].Priority)
        sSwap(Subsystems[i],Subsystems[j]);

  // switch on what needs switching on

  for(sInt i=0;i<SubsystemCount;i++)
  {
    if(Subsystems[i].Priority<priority && !Subsystems[i].Running)
    {
      sDPrintF(L"[sys] init 0x%02x %s\n",Subsystems[i].Priority,Subsystems[i].Name);
      if(Subsystems[i].Init)
        (*Subsystems[i].Init)();
      Subsystems[i].Running = 1;
    }
  }

  // switch off what needs switching off

  for(sInt i=SubsystemCount-1;i>=0;i--)
  {
    if(Subsystems[i].Priority>=priority && Subsystems[i].Running)
    {
      sDPrintF(L"[sys] exit 0x%02x %s\n",Subsystems[i].Priority,Subsystems[i].Name);
      if(Subsystems[i].Exit)
        (*Subsystems[i].Exit)();
      Subsystems[i].Running = 0;
    }
  }

  SubsystemPriority = priority;
}

/****************************************************************************/
/***                                                                      ***/
/***   Misc                                                               ***/
/***                                                                      ***/
/****************************************************************************/

#if sPLATFORM == sPLAT_WINDOWS || sPLATFORM == sPLAT_LINUX 
static sString<8> sPlatInfix = L".pch";
#endif
void sSetPlatformInfix(const sChar *infix)
{
#if sPLATFORM == sPLAT_WINDOWS || sPLATFORM == sPLAT_LINUX 
  sVERIFY(infix);
  sVERIFY(infix[0] == '.');
  sPlatInfix = infix;
  sVERIFY(sPlatInfix.Count()==4);
#endif
}

const sChar* sGetPlatformInfix()
{
#if sPLATFORM == sPLAT_WINDOWS || sPLATFORM == sPLAT_LINUX 
  return sPlatInfix;
#else
  return sPLATFORM_INFIX;
#endif
}

/****************************************************************************/
/****************************************************************************/
/***                                                                      ***/
/***   Joypads                                                            ***/
/***                                                                      ***/
/****************************************************************************/
/****************************************************************************/

sJoypad::sJoypad()
{
  Size = 0;
  Data = 0;
  Kind = 0;
  Id = 0;
}

sJoypad::~sJoypad()
{
}

sBool sJoypad::IsConnected()
{
  return 0;
}

void sJoypad::GetData(sJoypadData &data)
{
  sClear(data);
}

void sJoypad::GetName(const sStringDesc &name)
{
  sCopyString(name.Buffer,Name,name.Size);
}

void sJoypad::SetMotor(sInt slow,sInt fast)
{
}

sBool sJoypad::Identify(sInt kind,sU8 *data,sInt size)
{
  if(kind!=Kind || size!=Size) return 0;
  return sCmpMem(data,Data,size)==0;
}

void sJoypad::Poll()
{
}

/****************************************************************************/
/***                                                                      ***/
/***   InputQueue - pull style                                            ***/
/***                                                                      ***/
/****************************************************************************/

sInputFIFO::sInputFIFO(sInt maxEvents)
{
  ThreadLock = new sThreadLock;
  ThreadLock->Lock();
  
  EventCount = maxEvents;
  EventArray = new sInputFIFOEvent[maxEvents];

  WriteIndex = 0;
  ReadIndex  = WriteIndex;

  ThreadLock->Unlock();
}


sInputFIFO::~sInputFIFO()
{
  ThreadLock->Lock();
  
  EventCount = 0;
  sDeleteArray(EventArray);
  
  ThreadLock->Unlock();
  
  sDelete(ThreadLock);
}

sBool sInputFIFO::GetNextEvent(sInputFIFOEvent &writeMe, sBool remove)
{
  ThreadLock->Lock();
  
  sBool result = ReadIndex != WriteIndex;
  if (result)
  {
    writeMe = EventArray[ReadIndex];
    
    if (remove)
    {
      ReadIndex++;

      if (ReadIndex>=EventCount)
        ReadIndex = 0;
    }
  }
 
  ThreadLock->Unlock();
  return result;
}

sBool sInputFIFO::RemoveEvent()
{
  ThreadLock->Lock();
  
  sBool result = ReadIndex != WriteIndex;
  if (result)
  {
    ReadIndex++;
    if (ReadIndex>=EventCount)
      ReadIndex = 0;
  }
 
  ThreadLock->Unlock();
  return result;
}

void sInputFIFO::NewEvent2(sInputFIFOEvent &readMe)
{
  ThreadLock->Lock();

  EventArray[WriteIndex]            = readMe;
//  EventArray[WriteIndex].Timestamp  = sGetTime();

  WriteIndex++;
  if (WriteIndex>=EventCount)
    WriteIndex = 0;
  if (WriteIndex==ReadIndex)
  {
    // push readindex one element because it points to the oldest 
    // which will be overwritten next time
    ReadIndex++;
    if (ReadIndex>=EventCount)
      ReadIndex = 0;

    //sDPrintF(L"queue full\n");
  }

  ThreadLock->Unlock();
}

class sInputFIFO *sInputQueue = sNULL;

static void sInitInputQueue()
{
  sInputQueue = new sInputFIFO();
}

static void sExitInputQueue()
{
  sDelete(sInputQueue);
}

sADDSUBSYSTEM(InputQueue,0x29,sInitInputQueue,sExitInputQueue); // 0x29 just before joypad (0x30) and after threads (0x08)


/****************************************************************************/
/****************************************************************************/
/***                                                                      ***/
/***   File abstraction                                                   ***/
/***                                                                      ***/
/****************************************************************************/
/****************************************************************************/

sFile::sFile()
{
  MapFakeMem = 0;
}

sFile::~sFile()
{
  delete[] MapFakeMem;
}

sBool sFile::Close()
{
  return 0;
}

sBool sFile::Read(void *data,sDInt size)
{
  return 0;
}

sBool sFile::Read(void *data,sDInt size, sU32 userflags)
{
  return 0;
}

sBool sFile::Write(const void *data,sDInt size)
{
  return 0;
}

sU8 *sFile::Map(sS64 offset,sDInt size)
{
  return 0;
}

sBool sFile::SetOffset(sS64 offset)
{
  return 0;
}

sS64 sFile::GetOffset()
{
  return 0;
}

sBool sFile::SetSize(sS64)
{
  return 0;
}

sS64 sFile::GetSize()
{
  return 0;
}

sInt sFile::GetID()
{
  return -1;
}


sFileReadHandle sFile::BeginRead(sS64 offset,sDInt size, void *destbuffer)
{
  sFatal(L"asynchronous io not supported");
  return 0;
}

void sFile::EndRead(sFileReadHandle handle)
{
  sFatal(L"asynchronous io not supported");
}

sBool sFile::DataAvailable(sFileReadHandle handle)
{
  sFatal(L"asynchronous io not supported");
  return 0;
}

void *sFile::GetData(sFileReadHandle handle)
{
  sFatal(L"asynchronous io not supported");
  return 0;
}

sU64 sFile::GetPhysicalPosition(sS64 offset)
{
  sFatal(L"physical positions not supported");
  return 0;
}

sU8 *sFile::MapAll()
{
  if(MapFakeMem) 
    return MapFakeMem;

  sS64 size = GetSize();
  if(size>0x7fffffff)
    return 0;

  sU8 *map = Map(0,size);

  if(map==0)
  {
#if sSYSTEM_CONFIG_WINDOWS
    sDPrintF(L"file mapping failed\n");
#endif
#if sPLATFORM!=sPLAT_PS2
    map = MapFakeMem = (sU8 *)sAllocMem(size,sMEMFLAG_NORMAL,16,sMEMSEG_HEAP);
#else
    map = MapFakeMem = (sU8 *)sAllocMem(sAlign(size,2048),sMEMFLAG_NORMAL,64,sMEMSEG_HEAP);
#endif
    if(!Read(MapFakeMem,sDInt(size)))
    {
      sDeleteArray(MapFakeMem);
      return 0;
    }
  }

  return map;
}


sInt sFile::CopyFrom(sFile *f)
{
  if (!f) return 0;
  return CopyFrom(f,-1);
}

sInt sFile::CopyFrom(sFile *f, sS64 max)
{
  if (!f) return 0;
  max=sMin(max,f->GetSize()-f->GetOffset());

#if sPLATFORM==sPLAT_WINDOWS || sPLATFORM==sPLAT_APPLE || sPLATFORM==sPLAT_LINUX
  static const sS64 CHUNK=4*1024*1024;
#else
  static const sS64 CHUNK=65536;
#endif
  sU8 *buffer=new sU8[CHUNK];
  sInt done=0;

  while (max)
  {
    sInt todo=sMin(max,CHUNK);
    if (!f->Read(buffer,todo)) break;
    if (!Write(buffer,todo)) break;
    max-=todo;
    done+=todo;
  }

  sDeleteArray(buffer);
  return done;
}

/****************************************************************************/

sFileHandler::~sFileHandler()
{
}

sFile *sFileHandler::Create(const sChar *name,sFileAccess access)
{
  return 0;
};

/****************************************************************************/

static sFileHandler *FileHandlers[16];
static sInt FileHandlerCount;

void sInitFileHandlers()
{
  FileHandlerCount = 0;
}

void sExitFileHandlers()
{
  for(sInt i=0;i<FileHandlerCount;i++)
    sDelete(FileHandlers[i]);
}

sFile *sCreateFile(const sChar *name,sFileAccess access)
{
  // platform quality infix. CHAOS: i don't like this at all!
  sString<sMAXPATH> temp = name;
  sInt id = sFindString(temp,L".$$$");
  if(id>-1)
  {
    const sChar *ptr = sGetPlatformInfix();
    for(sInt i=0;i<4;i++)
      temp[id++] = *ptr++;
    name = temp;
  }


  sFile *file;
  for(sInt i=FileHandlerCount-1;i>=0;i--)   // have to scan backwards!
  {
    file = FileHandlers[i]->Create(name,access);
    if(file) 
      return file;
  }
  sDPrintF(L"[file] File not Found <%s>\n",name);
  return 0;
}

void sAddFileHandler(sFileHandler *h)
{
  sVERIFY(FileHandlerCount<sCOUNTOF(FileHandlers));
  FileHandlers[FileHandlerCount++] = h;
}

sADDSUBSYSTEM(FileHandlers,0x20,sInitFileHandlers,sExitFileHandlers);

/****************************************************************************/
/****************************************************************************/

class sMemFile : public sFile
{
  const sU8 *Data;
  sDInt Size;
  sBool Ownage;
  sDInt Offset;
public:
  sMemFile(const void *data,sDInt size,sBool owndata);
  ~sMemFile();
  sBool Read(void *data,sDInt size);  // read bytes
  sU8 *Map(sS64 offset,sDInt size);  // map file (from offset) (may fail)  
  sBool SetOffset(sS64 offset);       // seek to offset
  sS64 GetOffset();                   // get offset
  sS64 GetSize();                     // get size
};

sMemFile::sMemFile(const void *data,sDInt size,sBool owndata)
{
  Data = (const sU8 *) data;
  Size = size;
  Ownage = owndata;
  Offset = 0;
}

sMemFile::~sMemFile()
{
  if(Ownage)
    delete[] Data;
}

sBool sMemFile::Read(void *data,sDInt size)
{
  if(Offset+size<=Size)
  {
    sCopyMem(data,Data+Offset,size);
    Offset += size;
    return 1;
  }
  else
  {
    return 0;
  }
}

sU8 *sMemFile::Map(sS64 offset,sDInt size)
{
  if(offset>=0 && offset+size<=Size)
    return (sU8 *)(Data+offset);   // loose const!
  else
    return 0;
}

sBool sMemFile::SetOffset(sS64 offset)
{
  if(offset>=0 && offset<=Size)
  {
    Offset = sDInt(offset);
    return 1;
  }
  else
  {
    Offset = 0;
    return 0;
  }
}

sS64 sMemFile::GetOffset()
{
  return Offset;
}


sS64 sMemFile::GetSize()
{
  return Size;
}

/****************************************************************************/

sFile *sCreateMemFile(const void *data,sDInt size,sBool owndata)
{
  return new sMemFile(data,size,owndata);
}

/****************************************************************************/
/***                                                                      ***/
/***   sFileCalcMD5                                                       ***/
/***                                                                      ***/
/****************************************************************************/

sCalcMD5File::sCalcMD5File()
{
  TempCount = 0;
  TotalCount = 0;
  Checksum.CalcBegin();
}

sCalcMD5File::~sCalcMD5File()
{
}

sBool sCalcMD5File::Close()
{
  TotalCount += TempCount;
  Checksum.CalcEnd(TempBuf,TempCount,TotalCount);
  return sTRUE;
}

sBool sCalcMD5File::Read(void *data,sDInt size)
{
  sFatal(L"sFileMakeMD5::Read not supported");
  return sFALSE;
}

sBool sCalcMD5File::Write(const void *data,sDInt size)
{
  const sU8 *ptr = (const sU8*) data;
  sDInt count = size;
  sInt done = 0;

  if(TempCount)
  {
    // insert temp data from last write
    sInt add = sMin(size,sCOUNTOF(TempBuf)-TempCount);
    sCopyMem(TempBuf+TempCount,ptr,add);

    ptr += add;
    TempCount += add;
    count -= add;

    done = Checksum.CalcAdd(TempBuf,TempCount);
    TotalCount += done;

    if(done<TempCount)
    {
      sVERIFY(add==size);
      // size was smaller than tempbuffer, we are done
      sCopyMem(TempBuf,TempBuf+done,TempCount-done);
      TempCount -= done;
      return sTRUE;
    }

    TempCount = 0;
  }

  // calc checksum
  done = Checksum.CalcAdd(ptr,count);
  ptr += done;
  TotalCount += done;
  count -= done;

  // any temp data
  if(count>0)
  {
    TempCount = count;
    sVERIFY(TempCount>0 && TempCount<sCOUNTOF(TempBuf));
    sCopyMem(TempBuf,ptr,TempCount);
  }

  return sTRUE;
}

sU8 *sCalcMD5File::Map(sS64 offset,sDInt size)
{
  return 0;
}

sBool sCalcMD5File::SetOffset(sS64 offset)
{
  sFatal(L"sFileMakeMD5::SetOffset not supported\n");
  return sFALSE;
}

sS64 sCalcMD5File::GetOffset()
{
  return TotalCount;
}

sBool sCalcMD5File::SetSize(sS64)
{
  sFatal(L"sFileMakeMD5::SetOffset not supported\n");
  return sFALSE;
}

sS64 sCalcMD5File::GetSize()
{
  return TotalCount;
}

/****************************************************************************/
/***                                                                      ***/
/***   File System Wrappers                                               ***/
/***                                                                      ***/
/****************************************************************************/

sBool sCheckFile(const sChar *name)
{
  sFile *f = sCreateFile(name,sFA_READ);
  sBool result = (f!=0);
  delete f;
  return result;
}

sU8 *sLoadFile(const sChar *name)
{
  sDInt size;
  return sLoadFile(name,size);
}

sU8 *sLoadFile(const sChar *name,sDInt &size)
{
  sFile *file = sCreateFile(name,sFA_READ);
  if(file==0)
    return 0;
  size = file->GetSize();
  sVERIFY(size<=0x7fffffff);

  const sInt align = (sPLATFORM==sPLAT_PS2) ? 64 : 16;
#if sPLATFORM==sPLAT_PS2
  sU8 *mem = (sU8 *)sAllocMem((size+0x3f)&0xffffffc0,sMEMFLAG_NORMAL,align,sMEMSEG_HEAP);
#else
  sU8 *mem = (sU8 *)sAllocMem(size,sMEMFLAG_NORMAL,align,sMEMSEG_HEAP);
#endif

  if(!file->Read(mem,size))
    sDeleteArray(mem);
  delete file;
  return mem;
}

sChar *sLoadText(const sChar *name)
{
  sScopeMemSeg seg(sMEMSEG_HEAP);

  sDInt size;
  sChar *mem,*d,*s16;
  sInt count;
  sU8 *s8;

  sU8 *data = sLoadFile(name,size);
  s16 = (sChar *) data;

  if(!data) return 0;

  // handles little and big endian cpu
  // handles little and big endian file
  
  if(s16[0]==0xfeff)    // cpu endian == file endian
  {
    d = (sChar *) data;
    s16 = d+1;
    count = size/2-1;
    for(sInt i=0;i<count;i++)
    {
      if(*s16!='\r')
        *d++ = *s16;
      s16++;
    }
    *d++ = 0;
    return (sChar *)data;
  }
  else if(s16[0]==0xfffe)    // cpu endian != file endian
  {
    d = (sChar *) data;
    s16 = d;
    count = size/2-1;
    for(sInt i=0;i<count;i++)
    {
      sChar v = *s16; v = ((v>>8)|(v<<8))&0xffff;
      if(v!='\r')
        *d++ = v;
      s16++;
    }
    d[count] = 0;
    return d;
  }
  else    // 8 bit -> 16 bit
  {
    count = size;
    d = mem = new sChar[count+1];
    s8 = data;

    for(sInt i=0;i<count;i++)
    {
      if(*s8!='\r')
        *d++ = *s8;
      s8++;
    }
    *d++ = 0;
    delete[] data;
    return mem;
  }
}


sBool sSaveFile(const sChar *name,const void *data,sDInt bytes)
{
  sFile *file=sCreateFile(name,sFA_WRITE);
  if (!file) return 0;
  sBool ret=file->Write(data,bytes);
  delete file;
  return ret;
}

sBool sSaveTextAnsi(const sChar *name,const sChar *data)
{
  sFile *file = sCreateFile(name,sFA_WRITE);
  if(!file)
    return 0;

  const sInt BUFFER_SIZE = 2048;
  sU8 buffer[BUFFER_SIZE];
  sBool lastWasCR = sFALSE;

  // convert in small chunks
  while(*data)
  {
    sInt pos = 0;
    while(*data && pos < BUFFER_SIZE - 2)
    {
      if(sCONFIG_SYSTEM_WINDOWS)
      {
        if(*data == '\n' && !lastWasCR) // convert LF to CRLF on windows
          buffer[pos++] = '\r';

        lastWasCR = *data == '\r';
      }
      buffer[pos++] = *data++ & 0xff;
    }

    if(!file->Write(buffer,pos))
    {
      delete file;
      return 0;
    }
  }

  delete file;
  return 1;
}

sBool sBackupFile(const sChar *name)
{
  sArray<sDirEntry> dir;
  sDirEntry *ent;
  sInt len;
  sInt best,val;
  sString<sMAXPATH> buffer;

  if(!sLoadDir(dir,L"backup"))
    if(!sMakeDir(L"backup")) return 0;

  buffer = sFindFileWithoutPath(name);
  buffer.Add(L"#");
  len = sGetStringLen(buffer);

  best = 0;

  sFORALL(dir,ent)
  {
    if(sGetStringLen(ent->Name)>len && sCmpStringILen(ent->Name,(const sChar *)buffer,len)==0)
    {
      const sChar *scan = ent->Name+len;
      if(sScanInt(scan,val) && val>best)
        best = val;
    }
  }
  sSPrintF(buffer,L"backup/%s#%04d",sFindFileWithoutPath(name),best+1);
  return sCopyFile(name,buffer);
}

sBool sFileCalcMD5(const sChar *name, sChecksumMD5 &md5)
{
  const sInt BufSize = 65536;
  sU8 buffer[BufSize];

  sFile *file = sCreateFile(name);
  if(file)
  {
    md5.CalcBegin();
    sS64 size = file->GetSize();
    sInt left = size;
    while(left>BufSize)
    {
      if(!file->Read(buffer,BufSize))
        goto failed;
      md5.CalcAdd(buffer,BufSize);
      left -= BufSize;
    }
    if(left)
      if(!file->Read(buffer,left))
        goto failed;

    md5.CalcEnd(buffer,left,size);
    sDelete(file);
    return sTRUE;
  }
failed:
  md5.Hash[0] = 0;
  md5.Hash[1] = 0;
  md5.Hash[2] = 0;
  md5.Hash[3] = 0;
  sDelete(file);
  return sFALSE;
}

/****************************************************************************/

// this can parse strings in the form:
// "aa/bb/cc"        relative paths
// "aa\bb\cc"        both kinds of slashes
// "aa/bb/cc/"       trailing '/' optional
// "aa:/bb/cc"       windows absolute paths
// "//aa/bb/cc"      windows network paths
// "/aa/bb/cc"       unix absolute paths

sBool sMakeDirAll(const sChar *path)
{
  sString<sMAXPATH> canon;
  sString<sMAXPATH> test;
  sInt len = sGetStringLen(path);
  sInt pos;

  // remove trailing '/'

  if(len>0 && (path[len-1]=='/' || path[len-1]=='\\'))
    len--;

  // too long or empty strings

  if(len>=sMAXPATH-1 && len<1)    
    return 0;

  // convert slashes

  for(sInt i=0;i<len;i++)
  {
    if(path[i]=='\\')
      canon[i] = '/';
    else
      canon[i] = path[i];
  }
  canon[len] = 0;

  // special case: network paths

  sInt skip = 0;
  if(canon[0]=='/' && canon[1]=='/')
    skip = 1;

  // handle "xx:/", "//" and "/"

  pos = sFindLastChar(canon,':');
  if(pos==-1)
    pos = 0;
  else
    pos++;
  while(canon[pos]=='/')
    pos++;

  // now the remainder is in the form "aa/bb/cc\0"

  sBool ok = 1;
  while(canon[pos] && ok)
  {
    // find next dir

    while(canon[pos]!='/' && canon[pos]!=0)
      pos++;
    test = canon;
    test[pos] = 0;

    // check and create
    
    if(skip==0)
    {
      if(!sCheckDir(test))
        ok = sMakeDir(test);
    }
    else
    {
      skip--;
    }

    // next

    if(canon[pos]!=0)
      pos++;
  }
  
  // done

  return ok;
}

/****************************************************************************/
/***                                                                      ***/
/***   Memory Management                                                  ***/
/***                                                                      ***/
/****************************************************************************/

void sEnableMemoryAllocation(sBool enable)
{
  sAllocMemEnabled = enable;
}


// memory debug helper
#if !sRELEASE

sMemDebugInfo::sMemDebugInfo()
{
  FreeList = 0xffff;
  ItemCount = 0;
  sClear(ItemPool);
}

void sMemDebugInfo::Init()
{
  FreeList = 0xffff;
  ItemCount = 0;
  sClear(ItemPool);
}

void sMemDebugInfo::Exit()
{
  ItemCount=0;
  sClear(ItemPool);
}

sU16 sMemDebugInfo::Add(sU16 id, sPtr ptr, sInt size, const sChar8 *file, sU16 line)
{
  sU16 nid = 0xffff;
  if(FreeList != 0xffff)
  {
    nid = FreeList;
    FreeList = ItemPool[nid].Next;
  }
  else
    nid = ItemCount++;

  sVERIFY(nid<sCOUNTOF(ItemPool));
  Item *item = &ItemPool[nid];
  item->Init(ptr, size, file, line, id);
  return nid;
}

sU16 sMemDebugInfo::Free(sU16 id, sPtr ptr)
{
  sU16 result=id;
  Item *last=0;

  while(id!=0xffff)
  {
    Item *item = &ItemPool[id];
    if(item->Ptr == ptr)
    {
      if(last)
        last->Next = item->Next;
      else
        result = item->Next;
      item->Init(0,0,0,0xffff,FreeList);
      FreeList = id;
      return result;
    }
    last = item;
    id = item->Next;
  }

  return result;
}

void sMemDebugInfo::Item::Init(sDInt ptr, sInt size, const sChar8 *file, sU16 line, sU16 next)
{
  Ptr = ptr;
  Size = size;
  File = file;
  Line = line;
  Next = next;
}

sMemDebugInfo::Item *sMemDebugInfo::Get(sU16 id)
{
  if(id==0xffff)
    return 0;
  
  Item *item = &ItemPool[id];
  return item->Ptr ? item : 0;
}

sInt sCmpString8(const sChar8 *a,const sChar8 *b)
{
  a = a ? a : "";
  b = b ? b : "";
  sInt aa,bb;

  do
  {
    aa = *a++;
    bb = *b++;
  }
  while(aa!=0 && aa==bb);
  return sSign(aa-bb);
}

sInt sMemDebugInfo::GetAllocSize(sU16 id)
{
  sInt bytes=0;
  Item *item=Get(id);
  while(item)
  {
    bytes += item->Size;
    item = Get(item->Next);
  }
  return bytes;
}

#if sCONFIG_COMPILER_MSC

void sMemDebugInfo::StatisticDump(sU16 id)
{
  Item *item=0;

  // get used info items and sort for file and line
  sStackArray<Item,sMEMSEG_DBGINFOMAX> sorted;

  if(id==0xffff)
  {
    for(sInt i=0;i<ItemCount;i++)
      if(ItemPool[i].Ptr)
        sorted.AddTail(ItemPool[i]);
  }
  else
  {
    item = &ItemPool[id];
    while(item)
    {
      sorted.AddTail(*item);
      item = Get(item->Next);
      //item = item->Next<ItemCount ? &ItemPool[item->Next] : 0;
    }
  }

  sInt count = sorted.GetCount();

  for(sInt i=0;i<count-1;i++)
  {
    for(sInt j=i+1;j<count;j++)
    {
      sBool cmp = sCmpString8(sorted[i].File,sorted[j].File);
      if(cmp<0 || (!cmp && sorted[i].Line<sorted[j].Line))
        sSwap(sorted[i],sorted[j]);
    }
  }

  // merge list
  Item *last=0;
  sInt bytes=0;
  sInt lastid = 0;
  count=0;

  sInt ccheck = 0;

  sFORALL(sorted,item)
  {
    if(last && (sCmpString8(last->File,item->File) || last->Line!=item->Line))
    {
      sorted[lastid].File = last->File;
      sorted[lastid].Size = bytes;
      sorted[lastid].Line = last->Line;
      sorted[lastid].Next = count;

      lastid++;
      ccheck += count;
      if(ccheck!=_i)
        sDPrintF(L"");

      count = 1;
      bytes = item->Size;
    }
    else
    {
      count ++;
      bytes += item->Size;
    }
    last = item;
  }
  if(last)
  {
    sorted[lastid].File = last->File;
    sorted[lastid].Size = bytes;
    sorted[lastid].Line = last->Line;
    sorted[lastid].Next = count;

    ccheck += count;
    if(ccheck!=sorted.GetCount())
      sDPrintF(L"");

    lastid++;
  }
  while(sorted.GetCount()>lastid) sorted.RemTail();

  // sort for allocated size
  count = sorted.GetCount();
  for(sInt i=0;i<count-1;i++)
  {
    for(sInt j=i+1;j<count;j++)
    {
      if(sorted[i].Size<sorted[j].Size)
        sSwap(sorted[i],sorted[j]);
    }
  }

  // show mem use list
  sChar temp0[sMAXPATH];
  bytes = 0;
  count = 0;
  sFORALL(sorted,item)
  {
    temp0[0] = 0;
    if(item->File)
      sCopyString(temp0,item->File,sMAXPATH);
    sDPrintF(L"%48s(%5d): allocated %09d bytes, %04d allocs\n",temp0,item->Line,item->Size,item->Next);
    bytes += item->Size;
    count += item->Next;
  }
  sDPrintF(L"%04d allocs %09d bytes\n",count,bytes);
}

#endif

void sMemDebugInfo::FlushDump(sU16 id)
{
  Item *item = Get(id);
  sInt count = 0;
  sInt bytes = 0;
  while(item)
  {
    sChar temp[4096];
    if(item->File)
      sCopyString(temp,item->File,sCOUNTOF(temp));
    else
      temp[0]=0;
    sDPrintF(L"%s(%d): allocated %d bytes at 0x%08x\n",temp,item->Line,item->Size,item->Ptr);
    bytes += item->Size;
    count++;
    item = Get(item->Next);
  }
  if(count)
    sDPrintF(L"%d items not freed (%09d bytes)\n",count,bytes);
}

#endif

/****************************************************************************/

static const sInt Alignment = 16;

#undef sAllocMem
#undef new
void * sCDECL operator new(sCONFIG_SIZET size)
{
  return sAllocMem(size,sMEMFLAG_NORMAL,Alignment,sGetConstructorMemSegment());
}

void * sCDECL operator new[](sCONFIG_SIZET size)
{
  return sAllocMem(size,sMEMFLAG_NORMAL,Alignment,sGetConstructorMemSegment());
}

#if !sRELEASE
void * sCDECL operator new(sCONFIG_SIZET size,const char *file,int line)
{
  return sAllocMem(size,sMEMFLAG_NORMAL,Alignment,sGetConstructorMemSegment(),file,line);
}

void * sCDECL operator new[](sCONFIG_SIZET size,const char *file,int line)
{
  return sAllocMem(size,sMEMFLAG_NORMAL,Alignment,sGetConstructorMemSegment(),file,line);
}

#endif

void sCDECL operator delete(void *p)
{
  sFreeMem(p);
}

void sCDECL operator delete[](void *p)
{
  sFreeMem(p);
}

#define new sDEFINE_NEW
#define sAllocMem(size,flags,align,segid) sDEFINE_ALLOC(size,flags,align,segid)

/****************************************************************************/
