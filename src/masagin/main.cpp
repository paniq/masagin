/****************************************************************************/
/***                                                                      ***/
/***   (C) 2005 Dierk Ohlerich, all rights reserved                       ***/
/***                                                                      ***/
/****************************************************************************/

#include "main.hpp"
#include <stdio.h>
#include <unistd.h>
#include "base/windows.hpp"
#include "util/image.hpp"
#include "util/shaders.hpp"
// there is no selector dialog: all options come from the command line
#include "msglib/packfile.hpp"
#include "msglib/constants.hpp"

/****************************************************************************/

// initialize resources
 
MyApp::MyApp()
{
  Masagin = sNULL;
  Loader = sNULL;
  LoadLevel = LL_LOADING;
  LoaderThread = sNULL;
}

void LoadDemo(sThread *thread, void *udata)
{
  MyApp *app = (MyApp*)udata;
  app->Reload();
  app->Masagin->RenderSerialize = sFALSE;
  app->LoadLevel = LL_LOAD_END;
}

sBool MyApp::HasPackfile()
{
  sDirEntry de;
  return sGetFileInfo(PAKFILE, &de);
}

sBool MyApp::HasLogfile()
{
  sDirEntry de;
  return sGetFileInfo(TXTFILE, &de);
}

void MyApp::Init()
{
  if (sGetShellSwitch(L"pakfile") && !HasPackfile() && HasLogfile())
  {
    bWritePackFile(PAKFILE, TXTFILE);
    return;
  }

  sDirEntry de;
  if (sGetShellSwitch(L"logfiles"))
    bAddFileLogger(TXTFILE);
  else if (HasPackfile())
  {
    bAddPackFile(PAKFILE);
  }

  sString<256> scriptPath;
  if(sGetShellParameter(0,0))
  {
    scriptPath = sGetShellParameter(0,0);
  }
  else
  {
    scriptPath = L"demo.msg.o";
  }
  if (!HasPackfile() && !sGetFileInfo(scriptPath, &de))
  {
    sDPrintF(L"no such file: %s  (a path in the show folder; run with --help for usage)\n", scriptPath);
    return;
  }

  // the command line takes the place of a selector dialog:
  //   (default)         fullscreen at desktop resolution; -window for a 640x400 window; -w N -h N sets a window size
  //   -novsync          disable vsync         -mb N       motion blur: 0 off, 1 16, 2 16 interleaved (default), 3 8, 4 8 interleaved
  //   -framedrop        allow dropping frames -loop       start with loop enabled
  //   -nobloom          disable bloom
  sInt blur = sClamp(sGetShellInt(L"mb",L"motionblur",MSG_MB_16_I),0,4);
  sBool loop = sGetShellSwitch(L"loop");
  sBool framedrop = sGetShellSwitch(L"framedrop");
  sU32 flags = sISF_3D|sISF_CONTINUOUS;
  sInt reqw = sGetShellInt(L"w",L"width",-1);
  sInt reqh = sGetShellInt(L"h",L"height",-1);
  // fullscreen at desktop resolution unless a window is asked for (-window, or an explicit -w/-h)
  sBool fullscreen = !(sGetShellSwitch(L"window") || sGetShellSwitch(L"headless") || reqw>0 || reqh>0);
  if (sGetShellSwitch(L"f"))
    fullscreen = sTRUE;
  if (fullscreen)
    flags |= sISF_FULLSCREEN;
  if (sGetShellSwitch(L"novsync"))
    flags |= sISF_NOVSYNC;
  sInt w = 640;
  sInt h = 400;
  if (fullscreen)
  {
    sScreenInfo si;
    sGetScreenInfo(si, 0, -1);
    w = si.CurrentXSize;
    h = si.CurrentYSize;
  }
  if (reqw>0) w = reqw;
  if (reqh>0) h = reqh;
  sInit(flags,w,h);

  sSetWindowName(L"masagin");

  Loader = new sMasagin;
  Loader->MotionBlur = MSG_MB_16;
  Loader->Bloom = sFALSE;
  Loader->LoadScript(L"loader.msg.o");

  Masagin = new sMasagin;
  Masagin->MotionBlur = blur;
  Masagin->Slow = framedrop;
  if (sGetShellSwitch(L"nobloom"))
    Masagin->Bloom = sFALSE;
  Masagin->BloomRadius = sClamp(sGetShellInt(L"bloomradius",0,100),0,400)/100.0f;
  Masagin->BloomStrength = sClamp(sGetShellInt(L"bloomstrength",0,100),0,400)/100.0f;
  (void)loop; // TODO: ToggleLoop() needs LoopBegin/LoopEnd from the script, apply after load

  Masagin->ScriptPath = scriptPath;

  sLoadingTimeSource *ts = new sLoadingTimeSource();
  ts->Progress = Masagin;
  Loader->SetTimeSource(ts);

  //LoadDemo(sNULL, this);
  LoaderThread = new sThread(LoadDemo, -1, 16384, this);
  Masagin->Lock.Lock(); // initial lock
  LoadLevel = LL_LOADING;
}

void MyApp::Reload()
{
  Masagin->Reload();
}
// free resources (memory leaks are evil)

MyApp::~MyApp()
{
  if (LoadLevel == LL_LOADING)
  {
    // quitting while the loader thread is still running: the main thread holds Masagin->Lock for the
    // whole load, so joining the loader would deadlock. Nothing is worth saving here, so leave right away.
    fflush(stdout);
    fflush(stderr);
    _exit(0);
  }
  if (LoaderThread)
    LoaderThread->Terminate();
  sDelete(LoaderThread);
  sDelete(Loader);
  sDelete(Masagin);
  bExitFileLogger();
}

void MyApp::OnPrepareFrame()
{  
  if (LoadLevel == LL_LOADING)
  {
    Masagin->Lock.Unlock(); // temporary unlock
    sSleep(1); // allow the thread to load
    Masagin->Lock.Lock(); // lock again
    return;
  }
  if (LoadLevel == LL_LOAD_END)
  {
    Masagin->Lock.Unlock(); // temporary unlock
    LoadLevel = LL_PLAYING;
  }
  if (Masagin && Masagin->DebugView && Masagin->DocumentChanged())
    Reload();
}

/****************************************************************************/

// paint a frame

void MyApp::OnPaint3D()
{   
  if ((LoadLevel == LL_LOADING)||(LoadLevel == LL_LOAD_END))
    Loader->Paint();
  else if (LoadLevel == LL_PLAYING)
    Masagin->Paint();
}

/****************************************************************************/

// abort program when escape is pressed

void MyApp::OnInput(const sInputEvent &ie)
{
  if (ie.DeviceType == sIED_KEYBOARD)
  {
    if (Masagin)
    {
      if (Masagin->DebugView)
      {
        sU32 key = ie.Key & ~sKEYQ_REPEAT;
        switch (key)
        {
          case 'l': Masagin->ToggleLoop(); break;
          case 'f': {
            Masagin->FSAA = !Masagin->FSAA;
            sScreenMode sm;
            sGetScreenMode(sm);
            if (Masagin->FSAA)
              sm.MultiLevel = -1;
            else
              sm.MultiLevel = 2;
            sSetScreenMode(sm);
          } break;
          case 'x': Masagin->Slow = !Masagin->Slow; break;
          case 'm': Masagin->ToggleMotionBlur(); break;
          case 'b': Masagin->Bloom = !Masagin->Bloom; break;
          case sKEY_SPACE: Masagin->TogglePause(); break;
          case sKEY_BACKSPACE: Reload(); break;
          case sKEY_LEFT: Masagin->MoveTimeBack(); break;
          case sKEY_RIGHT: Masagin->MoveTimeFwd(); break;
          default: break;
        }
      }
      switch (ie.Key)
      {
        case sKEY_TAB: Masagin->DebugView = !Masagin->DebugView; break;
        default: break;
      }
    }
    if (ie.Key == sKEY_ESCAPE)
      sExit();
  }
}

/****************************************************************************/

// register application class

static void PrintUsage()
{
  printf(
    "masagin (linux/vulkan)\n"
    "usage: masagin [options] [script.msg.o]   (default: demo.msg.o in the show folder)\n"
    "\n"
    "display:\n"
    "  (default)        fullscreen at desktop resolution\n"
    "  -window          640x400 window        -w N -h N   window of the given size\n"
    "  -f               force fullscreen      -novsync    disable vsync\n"
    "quality:\n"
    "  -mb N            motion blur: 0 off, 1 16 samples, 2 16 interleaved (default), 3 8, 4 8 interleaved\n"
    "  -framedrop       allow dropping frames  -nobloom   disable bloom\n"
    "  -bloomradius P   bloom blur radius in percent (default 100)\n"
    "  -bloomstrength P bloom intensity in percent (default 100)\n"
    "  -loop            (accepted, not applied yet)\n"
    "show:\n"
    "  -show NAME       play build/NAME (default: masagin). The player moves into\n"
    "                   that folder first, so the script and everything the scripts\n"
    "                   open (shapes, audio) are paths inside it.\n"
    "  -dir DIR         move into DIR instead (-dir . for a show copied out on its\n"
    "                   own); relative paths, including -dump, resolve there\n"
    "  -shaders DIR     SPIR-V directory (default: the shaders directory next to\n"
    "                   the program; also honoured: $MSG_SHADER_DIR)\n"
    "testing:\n"
    "  -headless        no window (software Vulkan ok)   -frames N   exit after N frames\n"
    "  -dump DIR [-dumpstart S] [-dumpevery E]           write PNG screenshots\n"
    "keys: Esc quit, Backspace reload script\n"
    "environment: MSG_VK_DEVICE=n  MSG_VK_DEBUG=1  MSG_VK_VALIDATE=1  SDL_AUDIODRIVER=dummy\n");
}

// The show built by ./build.sh lives in ./build/<show>.  The player moves there
// before opening anything, so the script and everything it references (shapes,
// audio, loader.msg.o) are found where the scripts expect them; the shaders
// still come from the shaders directory next to the program (see sInit).
static sBool EnterShowDir()
{
  const sChar *show = sGetShellString(L"show",0,0);
  const sChar *dir = sGetShellString(L"dir",0,0);

  sString<1024> showDir;
  if (dir)
    showDir = dir;
  else
    showDir.PrintF(L"build/%s",show?show:L"masagin");

  sChar8 path[1024];
  sCopyString(path,showDir,sCOUNTOF(path));
  if (chdir(path)!=0)
  {
    printf("no such show folder: %s\n",path);
    printf("  ./build.sh builds it; -show NAME picks build/NAME, -dir DIR any other folder\n");
    return sFALSE;
  }
  printf("show: %s\n",path);
  fflush(stdout);   // keep the line in order when the log is piped
  return sTRUE;
}

void sMain()
{
  if (sGetShellSwitch(L"help") || sGetShellSwitch(L"-help") || sGetShellSwitch(L"?"))
  {
    PrintUsage();
    return;
  }

  if (!EnterShowDir())
    return;

  MyApp *app = new MyApp();
  sSetApp(app);
  app->Init();
}

/****************************************************************************/

