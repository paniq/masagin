/****************************************************************************/
/***  Linux platform layer: window, input, audio and file access via      ***/
/***  SDL3 (sdl_shim) and POSIX.                                          ***/
/****************************************************************************/

#include "base/types.hpp"
#if sCONFIG_SYSTEM_LINUX

#include "base/system.hpp"
#include "base/graphics.hpp"
#include "base/windows.hpp"
#include "base/sound.hpp"
#include "base/sdl_shim.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <fnmatch.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>

void Render3D();
void PreInitGFX(sInt flags,sInt &xs,sInt &ys);
void InitGFX(sInt flags,sInt xs,sInt ys);
void ExitGFX();
extern sInt sExitFlag;
extern sInt sSystemFlags;

// consumed by graphics_vk.cpp
void *sSDLWindow = 0;
sInt sVkVsync = 1;
const char *sVkShaderDir = "shaders";
const char *sVkDumpDir = 0;
sInt sVkDumpStart = 0, sVkDumpEvery = 1;
sInt sSDLDesktopW = 0, sSDLDesktopH = 0;

static sInt SystemFlags;
static sInt ErrorCode;
static sInt MaxFrames;
static sInt FrameNo;
static sBool Headless;
static sInt ConstructorSeg = sMEMSEG_HEAP;
static char ShaderDirBuf[1024], DumpDirBuf[1024];

static char ExeShaderDir[1024];

// The shaders are looked for in the working directory first and next to the
// program second, so a show can be run from its own folder.
static bool IsDir(const char *path) { struct stat st; return stat(path,&st)==0 && S_ISDIR(st.st_mode); }
static bool ShaderDirNextToProgram()
{
  char exe[1024];
  ssize_t n = readlink("/proc/self/exe",exe,sizeof(exe)-1);
  if(n <= 0) return false;
  exe[n] = 0;
  char *slash = strrchr(exe,'/');
  if(!slash) return false;
  *slash = 0;
  snprintf(ExeShaderDir,sizeof(ExeShaderDir),"%s/shaders",exe);
  return IsDir(ExeShaderDir);
}

/****************************************************************************/
/***  helpers                                                             ***/
/****************************************************************************/

static void Narrow(char *out,const sChar *s,sInt max)
{
  sInt i = 0;
  for(;s[i] && i<max-1;i++) out[i] = (s[i]=='\\') ? '/' : (char)s[i];
  out[i] = 0;
}

/****************************************************************************/
/***  memory                                                              ***/
/****************************************************************************/

#undef sAllocMem
void *sAllocMem(sCONFIG_SIZET size,sInt flags,sInt align,sInt segid,const sChar8 *file,sInt line)
{
  void *p = 0;
  if(align<16) align = 16;
  if(posix_memalign(&p,align,size?size:1)!=0) { sFatal(L"out of memory"); return 0; }
  return p;
}
void sFreeMem(void *p) { free(p); }
void sSetConstructorMemSegment(sInt segid) { ConstructorSeg = segid; }
sInt sGetConstructorMemSegment() { return ConstructorSeg; }
sU64 sGetUsedMainMemory() { return 0; }

void sSetErrorCode(sInt i) { ErrorCode = i; }
void sPingSound() {}

sBool sThreadLock::TryLock() { return pthread_mutex_trylock((pthread_mutex_t*)CriticalSection)==0; }

/****************************************************************************/
/***  files                                                               ***/
/****************************************************************************/

class sRootFile : public sFile
{
  int Fd; sFileAccess Access; sBool Ok;
public:
  sRootFile(int fd,sFileAccess a) : Fd(fd), Access(a), Ok(sTRUE) {}
  ~sRootFile() { Close(); }
  sBool Close() { if(Fd>=0) { if(close(Fd)<0) Ok = sFALSE; Fd = -1; } return Ok; }
  sBool Read(void *data,sDInt size)
  {
    sU8 *p = (sU8*)data;
    while(size>0) { ssize_t n = read(Fd,p,size); if(n<=0) { Ok = sFALSE; return sFALSE; } p += n; size -= n; }
    return sTRUE;
  }
  sBool Write(const void *data,sDInt size)
  {
    const sU8 *p = (const sU8*)data;
    while(size>0) { ssize_t n = write(Fd,p,size); if(n<=0) { Ok = sFALSE; return sFALSE; } p += n; size -= n; }
    return sTRUE;
  }
  sU8 *Map(sS64 offset,sDInt size) { return 0; }          // MapAll() falls back to reading
  sBool SetOffset(sS64 offset) { return lseek(Fd,offset,SEEK_SET)>=0; }
  sS64 GetOffset() { return lseek(Fd,0,SEEK_CUR); }
  sBool SetSize(sS64 size) { return ftruncate(Fd,size)==0; }
  sS64 GetSize() { struct stat st; if(fstat(Fd,&st)<0) return 0; return st.st_size; }
};

class sRootFileHandler : public sFileHandler
{
public:
  sFile *Create(const sChar *name,sFileAccess access)
  {
    char path[1024]; Narrow(path,name,sizeof(path));
    int fl = 0;
    switch(access)
    {
    case sFA_READ: case sFA_READRANDOM: fl = O_RDONLY; break;
    case sFA_WRITE: fl = O_WRONLY|O_CREAT|O_TRUNC; break;
    case sFA_WRITEAPPEND: fl = O_WRONLY|O_CREAT|O_APPEND; break;
    case sFA_READWRITE: fl = O_RDWR|O_CREAT; break;
    }
    int fd = open(path,fl,0644);
    if(fd<0) return 0;
    return new sRootFile(fd,access);
  }
};

static void InitRootFiles() { sAddFileHandler(new sRootFileHandler); }
sADDSUBSYSTEM(RootFiles,0x21,InitRootFiles,0);   // right after the handler list (0x20)

static void FillEntry(sDirEntry *de,const char *name,const struct stat &st)
{
  sChar wname[256]; sCopyString(wname,name,sCOUNTOF(wname));
  de->Name = wname;
  de->Size = (sInt)st.st_size;
  de->Flags = sDEF_EXISTS | (S_ISDIR(st.st_mode)?sDEF_DIR:0) | ((st.st_mode&S_IWUSR)?0:sDEF_WRITEPROTECT);
  de->LastWriteTime = (sU64)st.st_mtime;
}

sBool sGetFileInfo(const sChar *name,sDirEntry *de)
{
  char path[1024]; Narrow(path,name,sizeof(path));
  struct stat st;
  if(stat(path,&st)<0) return sFALSE;
  if(de) FillEntry(de,path,st);
  return sTRUE;
}

sBool sLoadDir(sArray<sDirEntry> &list,const sChar *path,const sChar *pattern)
{
  char p[1024],pat[256];
  Narrow(p,path,sizeof(p));
  if(pattern) Narrow(pat,pattern,sizeof(pat));
  DIR *d = opendir(p[0]?p:".");
  if(!d) return sFALSE;
  while(struct dirent *e = readdir(d))
  {
    if(pattern && fnmatch(pat,e->d_name,0)!=0) continue;
    char full[2048]; snprintf(full,sizeof(full),"%s/%s",p[0]?p:".",e->d_name);
    struct stat st; if(stat(full,&st)<0) continue;
    sDirEntry de; FillEntry(&de,e->d_name,st);
    list.AddTail(de);
  }
  closedir(d);
  return sTRUE;
}

sBool sChangeDir(const sChar *name) { char p[1024]; Narrow(p,name,sizeof(p)); return chdir(p)==0; }
sBool sMakeDir(const sChar *name) { char p[1024]; Narrow(p,name,sizeof(p)); return mkdir(p,0755)==0 || errno==EEXIST; }
sBool sCheckDir(const sChar *name) { char p[1024]; Narrow(p,name,sizeof(p)); struct stat st; return stat(p,&st)==0 && S_ISDIR(st.st_mode); }

sBool sCopyFile(const sChar *source,const sChar *dest,sBool failifexists)
{
  char s[1024],d[1024]; Narrow(s,source,sizeof(s)); Narrow(d,dest,sizeof(d));
  if(failifexists && access(d,F_OK)==0) return sFALSE;
  FILE *a = fopen(s,"rb"); if(!a) return sFALSE;
  FILE *b = fopen(d,"wb"); if(!b) { fclose(a); return sFALSE; }
  char buf[65536]; size_t n; sBool ok = sTRUE;
  while((n=fread(buf,1,sizeof(buf),a))>0) if(fwrite(buf,1,n,b)!=n) ok = sFALSE;
  fclose(a); if(fclose(b)!=0) ok = sFALSE;
  return ok;
}

/****************************************************************************/
/***  window, events, main loop                                           ***/
/****************************************************************************/

void sSetWindowName(const sChar *name) { char n[256]; Narrow(n,name,sizeof(n)); shim_set_title(n); }

void sInit(sInt flags,sInt xs,sInt ys)
{
  SystemFlags = flags; sSystemFlags = flags;
  if(!sFrameHook) sFrameHook = new sHooks;
  if(!sNewDeviceHook) sNewDeviceHook = new sHooks;

  Headless = sGetShellSwitch(L"headless") || getenv("MSG_HEADLESS");
  MaxFrames = sGetShellInt(L"frames",0,0);
  const sChar *s;
  if((s = sGetShellString(L"shaders",0,0))) { Narrow(ShaderDirBuf,s,sizeof(ShaderDirBuf)); sVkShaderDir = ShaderDirBuf; }
  else if(getenv("MSG_SHADER_DIR")) sVkShaderDir = getenv("MSG_SHADER_DIR");
  else if(!IsDir(sVkShaderDir) && ShaderDirNextToProgram()) sVkShaderDir = ExeShaderDir;
  if((s = sGetShellString(L"dump",0,0))) { Narrow(DumpDirBuf,s,sizeof(DumpDirBuf)); sVkDumpDir = DumpDirBuf; mkdir(DumpDirBuf,0755); }
  sVkDumpStart = sGetShellInt(L"dumpstart",0,0);
  sVkDumpEvery = sGetShellInt(L"dumpevery",0,1);
  sVkVsync = (flags & sISF_NOVSYNC) ? 0 : 1;

  if(!shim_init(!Headless)) sFatal(L"SDL initialisation failed");
  if(!Headless) shim_desktop_size(&sSDLDesktopW,&sSDLDesktopH);
  if(!Headless)
  {
    sSDLWindow = shim_create_window(xs,ys,(flags&sISF_FULLSCREEN)!=0,"masagin");
    if(!sSDLWindow) sFatal(L"cannot create window");
  }
  PreInitGFX(flags,xs,ys);
  InitGFX(flags,xs,ys);
}

static void HandleEvent(const ShimEvent &e)
{
  sApp *app = sGetApp();
  sInputEvent ie;
  ie.DeviceType = sIED_KEYBOARD; ie.DeviceId = 0; ie.Timestamp = sGetTime(); ie.MouseX = ie.MouseY = 0;
  switch(e.type)
  {
  case SHIM_QUIT: sExitFlag = 1; break;
  case SHIM_KEYDOWN:
  case SHIM_KEYUP:
    ie.Key = (sU32)e.key | (sU32)e.mods;
    if(e.repeat) ie.Key |= sKEYQ_REPEAT;
    if(e.type==SHIM_KEYUP) ie.Key |= sKEYQ_BREAK;
    if(app) app->OnInput(ie);
    break;
  case SHIM_MOUSEDOWN:
  case SHIM_MOUSEUP:
    ie.DeviceType = sIED_MOUSE; ie.MouseX = e.x; ie.MouseY = e.y;
    ie.Key = (sKEY_LMB + (e.key-1)) | (e.type==SHIM_MOUSEUP ? sKEYQ_BREAK : 0);
    if(app) app->OnInput(ie);
    break;
  case SHIM_WHEEL:
    ie.DeviceType = sIED_MOUSE; ie.Key = (e.key>0) ? sKEY_WHEELUP : sKEY_WHEELDOWN;
    if(app) app->OnInput(ie);
    break;
  default: break;
  }
}

// called by main() after sMain() returned: the equivalent of the WM_PAINT loop in system_win.cpp
void sRunLoop()
{
  if(!(SystemFlags & sISF_3D)) return;
  while(!sExitFlag)
  {
    ShimEvent e;
    while(shim_poll(&e)) HandleEvent(e);
    if(sExitFlag) break;
    sFrameHook->Call();
    if(sGetApp()) sGetApp()->OnPrepareFrame();
    Render3D();
    sPingSound();
    FrameNo++;
    if(MaxFrames && FrameNo>=MaxFrames) break;
    if(!(SystemFlags & sISF_CONTINUOUS)) sSleep(10);
  }
  delete sGetApp();
  sSetApp(0);
  ExitGFX();
  shim_quit();
}

void sGetMouse(sMouseData &md,sBool center,sBool cleardelta)
{
  static int lx,ly;
  int x,y,b; shim_mouse_state(&x,&y,&b);
  md.MouseX = x; md.MouseY = y; md.DeltaX = x-lx; md.DeltaY = y-ly; md.MouseButtons = b;
  if(cleardelta) { lx = x; ly = y; }
}

/****************************************************************************/
/***  sound handler (sound.hpp interface) on top of the shim audio stream ***/
/****************************************************************************/

static sSoundHandler SoundHandler;
static void *SoundStream;

static void SoundRender(float *buf,int frames,void *user)
{
  if(!SoundHandler) return;
  sS16 *tmp = new sS16[frames*2];
  SoundHandler(tmp,frames);
  for(int i=0;i<frames*2;i++) buf[i] = tmp[i]/32768.0f;
  delete[] tmp;
}

sBool sSetSoundHandler(sInt freq,sSoundHandler handler,sInt latency)
{
  sClearSoundHandler();
  SoundHandler = handler;
  SoundStream = shim_audio_open(freq,2,freq/5,SoundRender,0);
  return SoundStream!=0;
}
void sClearSoundHandler()
{
  void *s = SoundStream; SoundStream = 0;
  if(s) shim_audio_close(s);
  SoundHandler = 0;
}

/****************************************************************************/
/***  sFont2D: debug text is not rendered on Linux (no GDI)               ***/
/****************************************************************************/

sInt sFont2D::GetHeight() { return 8; }
sInt sFont2D::GetWidth(const sChar *text,sInt len) { if(len<0) len = sGetStringLen(text); return len*6; }
void sFont2D::PrintMarked(sInt flags,const sRect *r,sInt x,sInt y,const sChar *text,sInt len,sPrintInfo *pi) {}
void sFont2D::PrintBasic(sInt flags,const sRect *r,sInt x,sInt y,const sChar *text,sInt len) {}

#else
sInt sDummyLink_platform_linux;
#endif
