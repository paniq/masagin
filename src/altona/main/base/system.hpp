/****************************************************************************/
/***                                                                      ***/
/***   (C) 2005 Dierk Ohlerich, all rights reserved                       ***/
/***                                                                      ***/
/****************************************************************************/

#ifndef HEADER_ALTONA_BASE_SYSTEM
#define HEADER_ALTONA_BASE_SYSTEM

#include "base/types.hpp"
#include "base/serialize.hpp"
#include "base/types2.hpp"        // only for non-console-stuff

/****************************************************************************/

// platform file infixes
#if sRENDERER==sRENDER_XBOX360
#define sPLATFORM_INFIX L".xbox360"
#elif sRENDERER==sRENDER_PS3GL
#define sPLATFORM_INFIX L".ps3"
#elif sRENDERER == sRENDER_PS2
#define sPLATFORM_INFIX L".ps2"
#elif sRENDERER == sRENDER_WII
#define sPLATFORM_INFIX L".wii"
#else
// replaced with pch or pcl use sSetPlatformInfix(const sChar *infix) to change quality infix
#define sPLATFORM_INFIX L".$$$"
#endif

/****************************************************************************/
/***                                                                      ***/
/***   initialization                                                     ***/
/***                                                                      ***/
/****************************************************************************/

void sInit(sInt flags,sInt xs=0,sInt ys=0);
void sExit();
void sRestart();
sInt sGetSystemFlags();
void sSetErrorCode(sInt i=1);           // set the commandline error code. 0=OK, 1=ERROR

/****************************************************************************/

enum sInitSystemFlags
{
  sISF_FULLSCREEN       = 0x0001,     // create window in fullscreen
  sISF_3D               = 0x0002,     // allow 3d rendering (DirectX)
  sISF_2D               = 0x0004,     // allow 2d rendering (GDI)
  sISF_CONTINUOUS       = 0x0008,     // update automatically
  sISF_NOVSYNC          = 0x0100,     // disable wait for VSync, for benchmarks!
  sISF_FSAA             = 0x1000,     // enable full scene anti aliasing

  sISF_SHADER_BEST      = 0x0000,     // use best available shader model
  sISF_SHADER_00        = 0x0010,     // revert to fixed function pipeline
  sISF_SHADER_13        = 0x0020,     // revert to shader model 1.3
  sISF_SHADER_20        = 0x0030,     // revert to shader model 2.0
  sISF_SHADERMASK       = 0x0070, 

  sISF_REFRAST          = 0x0200,     // use reference rasterizer (sloooow)
  sISF_NOBUSYWAIT       = 0x0400,     // no busy wait during FLIP, so you get some more multithreading
 
  sISF_NOSOUND          = 0x0800,     // do not initialise sound

  sISF_TOPMOST          = 0x1000,     // window is always on top

#if _DEBUG
  sISF_FULLSCREENIFRELEASE = 0,
#else
  sISF_FULLSCREENIFRELEASE = sISF_FULLSCREEN,
#endif
};

/****************************************************************************/

void sAddSubsystem(const sChar *name,sInt priority,void (*init)(),void (*exit)());
void sSetRunlevel(sInt priority);
#define sADDSUBSYSTEM(n,p,i,e) namespace sSubSystems { struct n##_type { n##_type() {sAddSubsystem(sTXT(#n),p,i,e);} } n##_global; }

/****************************************************************************/

// additional init if called from outside (winmain is not possible)
void sExternMainInit(void *p_instance, void *p_hwnd);
void sExternMessageLoop(void *p_messageLoopData);
int  sExternMessageLoopGetSize(); // just to be sure to have the same struct
void sExternMainExit();

/****************************************************************************/
/***                                                                      ***/
/***   Misc                                                               ***/
/***                                                                      ***/
/****************************************************************************/

sBool sExecuteShell(const sChar *cmdline);
sBool sExecuteShell(const sChar *cmdline,sTextBuffer *tb);
void sConsoleWindowClear();
sInt sGetTime();
sInt sGetRandomSeed();
const sChar *sGetCommandLine();
extern sHooks *sFrameHook;        // called every frame
extern sHooks *sNewDeviceHook;    // called when new io devices where attached
void sQueueInput(sInt devtype,sInt devnum,sU32 key); // queue input event

// change platform quality infix only supported on pc
void sSetPlatformInfix(const sChar *infix);
const sChar* sGetPlatformInfix();

// timing helper for easy scope profiling
struct sTimeHelper
{
  sInt Start;
  const sChar *Msg;
  sTimeHelper(const sChar *msg):Msg(msg) {Start = sGetTime();}
  ~sTimeHelper() {sInt End = sGetTime(); sDPrintF(L"%s %f sec\n",Msg,(End-Start)/1000.0f); }
};

#define sGetScopeTime(msg) sTimeHelper gettime##__LINE__(msg);

/****************************************************************************/

// system memory info
enum sSysMemKind
{
  sMEMKIND_CPU  = 0x01,
  sMEMKIND_GPU  = 0x02,
};

struct sSysMemInfo
{
  sInt Kind;
  sCONFIG_SIZET PhysicalTotal;
  sCONFIG_SIZET PhysicalAvailable;
  sCONFIG_SIZET VirtualTotal;
  sCONFIG_SIZET VirtualAvailable;
};

// returns number of different memory types, fillst up to count info entries with mem infos
sInt sGetSystemMemInfo(sSysMemInfo* info=0, sInt count=0);

sU64 sGetUsedMainMemory();
void sEnableMemoryAllocation(sBool enable);

/****************************************************************************/
/***                                                                      ***/
/***   Keyboard                                                           ***/
/***                                                                      ***/
/****************************************************************************/

void sCatchCtrlC(sBool enable=1);
sBool sGotCtrlC();
sU32 sGetKeyQualifier();                // get shift-state sKEYQ

/****************************************************************************/
/***                                                                      ***/
/***   Mouse                                                              ***/
/***                                                                      ***/
/****************************************************************************/

enum sMouseButton
{
  sMB_LEFT = 1,                   // left mousebutton
  sMB_RIGHT = 2,                  // right mousebutton
  sMB_MIDDLE = 4,                 // middle mousebutton
  sMB_X1 = 8,                     // 4th mousebutton
  sMB_X2 = 16,                    // 5th mousebutton
};

struct sMouseData
{
  sInt MouseX;
  sInt MouseY;
  sInt DeltaX;
  sInt DeltaY;
  sInt MouseButtons;
};

void sGetMouse(sMouseData &md,sBool center=0,sBool cleardelta=1);
void sSetMouse(sInt x,sInt y);
void sGetMouseHard(sInt &buttons,sInt &x,sInt &y,sInt &z);

/****************************************************************************/
/***                                                                      ***/
/***   Joypads                                                            ***/
/***                                                                      ***/
/****************************************************************************/

enum sJoypadDataAxes              // typical axe-semantics of a PC joypad
{
  sJOYAXE_LEFTX  = 0,
  sJOYAXE_LEFTY  = 1,
  sJOYAXE_RIGHTX = 2,
#if sCONFIG_SYSTEM_WINDOWS
  sJOYAXE_RIGHTY = 5,
#else
  sJOYAXE_RIGHTY = 3,
#endif
  // wii
  sJOYAXE_POINTER_X = 6,
  sJOYAXE_POINTER_Y = 7,
  sJOYAXE_POINTER_DISTANCE = 8,
  sJOYAXE_POINTER_ANGLE = 9,
  sJOYAXE_ACCEL_X = 10,
  sJOYAXE_ACCEL_Y = 11,
  sJOYAXE_ACCEL_Z = 12,
  sJOYAXE_ACCEL2_X = 13,
  sJOYAXE_ACCEL2_Y = 14,
  sJOYAXE_ACCEL2_Z = 15,
  sJOYAXE_MAX = 16,
};

struct sJoypadData
{
  sInt Connected;                 // 0: disconnected, 1: connected
  sInt ButtonMask;                // which buttons are available? usually continuous
  sInt AnalogMask;                // which axes are available? usually NOT continous
  sInt PovMask;                   // which POV's are available? usually the first
  sU32 Buttons;                   // all buttons states
  sU32 Povs;                      // 8 pov states @ 4 bits
  sU16 Analog[sJOYAXE_MAX];       // 16 analog channel states, 0 .. 0xffff
  sU8 Pressure[32];               // pressure of the buttons
};

class sJoypad
{
  friend void sAddJoypad(sJoypad *j);
protected:
  sString<64> Name;
  const sU8 *Data;
  sInt Size;
  sInt Kind;
  sInt Id;
public:
  sJoypad();
  virtual ~sJoypad();
  virtual sBool IsConnected();
  virtual void GetData(sJoypadData &data);
  virtual void GetName(const sStringDesc &name); 
  virtual void SetMotor(sInt slow,sInt fast);       // two motors, 0..255
  sInt GetId() { return Id; }

  // used internally
  virtual void Poll();                              // called every frame by system
  sBool Identify(sInt kind,sU8 *data,sInt size); // identify in a platform dependent way
  sDNode Node;
};

// query joypads

sInt sGetJoypadCount();                     // max number of pads
sJoypad *sGetJoypad(sInt id);               // sJoypad stays associated to id even when pad gets unplugged

// interface for managing joypads. not available on consoles!
void sAddJoypad(sJoypad *j);                // add a joypad to the joypad manager
sJoypad *sIdentifyJoypad(sInt kind,sU8 *data,sInt size); // identify joypad. used to check before sAddJoypad if this pad is already known

// obsolete joypad functions

sBool sOBSOLETE sGetJoypad(sInt device,sJoypadData &data);
sBool sOBSOLETE sGetJoypadName(sInt device,const sStringDesc &name);
sBool sOBSOLETE sSetJoypadMotor(sInt device,sInt slow,sInt fast);     // two motors, 0..255

/****************************************************************************/
/***                                                                      ***/
/***   All button codes                                                   ***/
/***                                                                      ***/
/****************************************************************************/

enum sInputKeys
{
  // keyboard

  sKEY_BACKSPACE    = 8,              // traditional ascii
  sKEY_TAB          = 9,
  sKEY_ENTER        = 10,
  sKEY_ESCAPE       = 27,
  sKEY_SPACE        = 32,

  sKEY_UP           = 0x0000e000,     // cursor block
  sKEY_DOWN         = 0x0000e001,
  sKEY_LEFT         = 0x0000e002,
  sKEY_RIGHT        = 0x0000e003,
  sKEY_PAGEUP       = 0x0000e004,
  sKEY_PAGEDOWN     = 0x0000e005,
  sKEY_HOME         = 0x0000e006,
  sKEY_END          = 0x0000e007,
  sKEY_INSERT       = 0x0000e008,
  sKEY_DELETE       = 0x0000e009,

  sKEY_PAUSE        = 0x0000e010,      // Pause/Break key
  sKEY_SCROLL       = 0x0000e011,      // ScrollLock key
  sKEY_NUMLOCK      = 0x0000e012,      // NumLock key
  sKEY_PRINT        = 0x0000e013,
  sKEY_WINL         = 0x0000e014,      // left windows key
  sKEY_WINR         = 0x0000e015,      // right windows key
  sKEY_WINM         = 0x0000e016,      // windows menu key
  sKEY_SHIFT        = 0x0000e017,      // a shift key was pressed
  sKEY_CTRL         = 0x0000e018,      // a ctrl key was pressed

  sKEY_F1           = 0x0000e020,      // function keys
  sKEY_F2           = 0x0000e021,
  sKEY_F3           = 0x0000e022,
  sKEY_F4           = 0x0000e023,
  sKEY_F5           = 0x0000e024,
  sKEY_F6           = 0x0000e025,
  sKEY_F7           = 0x0000e026,
  sKEY_F8           = 0x0000e027,
  sKEY_F9           = 0x0000e028,
  sKEY_F10          = 0x0000e029,
  sKEY_F11          = 0x0000e02a,
  sKEY_F12          = 0x0000e02b,

  // mouse

  sKEY_LMB          = 0x0000e100,
  sKEY_RMB          = 0x0000e101,
  sKEY_MMB          = 0x0000e102,
  sKEY_X1MB         = 0x0000e103,
  sKEY_X2MB         = 0x0000e104,
  sKEY_LASTMB       = 0x0000e107,
  sKEY_MOUSEMOVE    = 0x0000e180,
  sKEY_WHEELUP      = 0x0000e181,
  sKEY_WHEELDOWN    = 0x0000e182,

  // joypad

  sPAD_BUTTON       = 0x0000e200,           // general joypad 
  sPAD_POV          = 0x0000e220,
  sPAD_PLUGIN       = 0x0000e2f0,
  sPAD_PLUGOUT      = 0x0000e2f1,

  sPAD_TRIANGLE     = sPAD_BUTTON+0x00,     // ps2-like controllers
  sPAD_CIRCLE       = sPAD_BUTTON+0x01,
  sPAD_CROSS        = sPAD_BUTTON+0x02,
  sPAD_SQUARE       = sPAD_BUTTON+0x03,
  sPAD_L2           = sPAD_BUTTON+0x04,
  sPAD_R2           = sPAD_BUTTON+0x05,
  sPAD_L1           = sPAD_BUTTON+0x06,
  sPAD_R1           = sPAD_BUTTON+0x07,
  sPAD_SELECT       = sPAD_BUTTON+0x08,
  sPAD_L3           = sPAD_BUTTON+0x09,
  sPAD_R3           = sPAD_BUTTON+0x0a,
  sPAD_START        = sPAD_BUTTON+0x0b,
  sPAD_UP           = sPAD_BUTTON+0x0c,
  sPAD_RIGHT        = sPAD_BUTTON+0x0d,
  sPAD_DOWN         = sPAD_BUTTON+0x0e,
  sPAD_LEFT         = sPAD_BUTTON+0x0f,

  sPAD_XBOX_LB      = sPAD_BUTTON+0x08,     // xbox-360 controllers
  sPAD_XBOX_LT      = sPAD_BUTTON+0x0a,
  sPAD_XBOX_RB      = sPAD_BUTTON+0x09,
  sPAD_XBOX_RT      = sPAD_BUTTON+0x0b,
  sPAD_XBOX_Y       = sPAD_BUTTON+0x0f,
  sPAD_XBOX_X       = sPAD_BUTTON+0x0e,
  sPAD_XBOX_B       = sPAD_BUTTON+0x0d,
  sPAD_XBOX_A       = sPAD_BUTTON+0x0c,
  sPAD_XBOX_UP      = sPAD_BUTTON+0x00,
  sPAD_XBOX_DOWN    = sPAD_BUTTON+0x01,
  sPAD_XBOX_LEFT    = sPAD_BUTTON+0x02,
  sPAD_XBOX_RIGHT   = sPAD_BUTTON+0x03,
  sPAD_XBOX_BACK    = sPAD_BUTTON+0x05,
  sPAD_XBOX_START   = sPAD_BUTTON+0x04,
  sPAD_XBOX_L3      = sPAD_BUTTON+0x06,
  sPAD_XBOX_R3      = sPAD_BUTTON+0x07,

  sPAD_WII_B        = sPAD_BUTTON+0x00,     // wiimote (mapping works like PS2 controllers)
  sPAD_WII_Z        = sPAD_BUTTON+0x01,     // HOME/POWER keys aren't mapped: none of your business.
  sPAD_WII_A        = sPAD_BUTTON+0x02, 
  sPAD_WII_C        = sPAD_BUTTON+0x03,
  sPAD_WII_2        = sPAD_BUTTON+0x05,
  sPAD_WII_1        = sPAD_BUTTON+0x07,
  sPAD_WII_MINUS    = sPAD_BUTTON+0x08,
  sPAD_WII_PLUS     = sPAD_BUTTON+0x0b,
  sPAD_WII_UP       = sPAD_BUTTON+0x0c,
  sPAD_WII_RIGHT    = sPAD_BUTTON+0x0d,
  sPAD_WII_DOWN     = sPAD_BUTTON+0x0e,
  sPAD_WII_LEFT     = sPAD_BUTTON+0x0f,

  sPAD_POV0_UP      = sPAD_POV+0*4+0,       // pov buttons
  sPAD_POV0_RIGHT   = sPAD_POV+0*4+1,
  sPAD_POV0_DOWN    = sPAD_POV+0*4+2,
  sPAD_POV0_LEFT    = sPAD_POV+0*4+3,
  sPAD_POV1_UP      = sPAD_POV+1*4+0,
  sPAD_POV1_RIGHT   = sPAD_POV+1*4+1,
  sPAD_POV1_DOWN    = sPAD_POV+1*4+2,
  sPAD_POV1_LEFT    = sPAD_POV+1*4+3,
  sPAD_POV2_UP      = sPAD_POV+2*4+0,
  sPAD_POV2_RIGHT   = sPAD_POV+2*4+1,
  sPAD_POV2_DOWN    = sPAD_POV+2*4+2,
  sPAD_POV2_LEFT    = sPAD_POV+2*4+3,
  sPAD_POV3_UP      = sPAD_POV+3*4+0,
  sPAD_POV3_RIGHT   = sPAD_POV+3*4+1,
  sPAD_POV3_DOWN    = sPAD_POV+3*4+2,
  sPAD_POV3_LEFT    = sPAD_POV+3*4+3,
  sPAD_POV4_UP      = sPAD_POV+4*4+0,
  sPAD_POV4_RIGHT   = sPAD_POV+4*4+1,
  sPAD_POV4_DOWN    = sPAD_POV+4*4+2,
  sPAD_POV4_LEFT    = sPAD_POV+4*4+3,
  sPAD_POV5_UP      = sPAD_POV+5*4+0,
  sPAD_POV5_RIGHT   = sPAD_POV+5*4+1,
  sPAD_POV5_DOWN    = sPAD_POV+5*4+2,
  sPAD_POV5_LEFT    = sPAD_POV+5*4+3,
  sPAD_POV6_UP      = sPAD_POV+6*4+0,
  sPAD_POV6_RIGHT   = sPAD_POV+6*4+1,
  sPAD_POV6_DOWN    = sPAD_POV+6*4+2,
  sPAD_POV6_LEFT    = sPAD_POV+6*4+3,
  sPAD_POV7_UP      = sPAD_POV+7*4+0,
  sPAD_POV7_RIGHT   = sPAD_POV+7*4+1,
  sPAD_POV7_DOWN    = sPAD_POV+7*4+2,
  sPAD_POV7_LEFT    = sPAD_POV+7*4+3,

  // virtual

//  sKEY_OK           = 0x0000e300,
//  sKEY_CANCEL       = 0x0000e301,


  // qualifiers

  sKEYQ_SHIFTL      = 0x00010000,    // qualifiers
  sKEYQ_SHIFTR      = 0x00020000,
  sKEYQ_CAPS        = 0x00040000,
  sKEYQ_CTRLL       = 0x00080000,
  sKEYQ_CTRLR       = 0x00100000,
  sKEYQ_ALT         = 0x00200000,
  sKEYQ_ALTGR       = 0x00400000,
  sKEYQ_NUM         = 0x20000000,    // from numeric keypad
  sKEYQ_REPEAT      = 0x40000000,    // this is a repeated key
  sKEYQ_BREAK       = 0x80000000,    // this is a break-key

  sKEYQ_SHIFT       = 0x00070000,
  sKEYQ_CTRL        = 0x00180000,
  sKEYQ_MASK        = 0x0000ffff,
};

/****************************************************************************/
/***                                                                      ***/
/***   InputQueue - pull style                                            ***/
/***                                                                      ***/
/****************************************************************************/

struct sInputFIFOEvent
{
  sInt Timestamp;                           // ms accurate timestamp - good for 50 days - only use differences!
  sInt DeviceType;                          // sIED_???
  sInt DeviceId;                            // 0..n
  
  union
  {
    sJoypadData Joypad;
  } Data;
};

typedef sInputFIFOEvent sInputQueueEvent;

extern class sInputFIFO *sInputQueue;

// input fifo for retrieving collected inputdata
// if internal buffers are full oldest events get overwritten
// class is threadsave

class sInputFIFO
{
public:
  sInputFIFO(sInt maxEvents = 64); // for wii 16 events per frame * 4 joypads
  ~sInputFIFO();

  sBool GetNextEvent(sInputFIFOEvent &writeMe, sBool remove=sTRUE);
  sBool RemoveEvent();  // like getnextevent(writeMe,sTRUE) but without assignment
  void NewEvent2(sInputFIFOEvent &readMe);  // please set timestamp by yourself!

protected:
  sInputFIFOEvent *EventArray;
  sInt             EventCount;

  sInt             ReadIndex;
  sInt             WriteIndex;

  class sThreadLock *ThreadLock;
};

/****************************************************************************/
/***                                                                      ***/
/***   File abstraction                                                   ***/
/***                                                                      ***/
/****************************************************************************/

enum sFileAccess
{
  sFA_READ = 1,                   // read mostly sequential
  sFA_READRANDOM,                 // read in random order
  sFA_WRITE,                      // create new file, possibly overwriting old data
  sFA_WRITEAPPEND,                // append to existing file or create new file
  sFA_READWRITE,                  // open file for read and write
};

enum sFileUserFlags
{
  sFU_DESTSPU = 1,
};

typedef sInt sFileReadHandle;

class sFile
{
  sU8 *MapFakeMem;                            // memory buffer for fake file mapping in MapAll()
public:
  sFile();
  virtual ~sFile();
  virtual sBool Close();                      // only needed to get error code. the error code is accumulative!
  virtual sBool Read(void *data,sDInt size);  // read bytes. may change mapping.
  virtual sBool Read(void *data,sDInt size,sU32 userflags);  // read bytes. may change mapping.
  virtual sBool Write(const void *data,sDInt size); // write bytes, may change mapping.
  virtual sU8 *Map(sS64 offset,sDInt size);  // map file (independend from SetOffset()) (may fail)  
  virtual sBool SetOffset(sS64 offset);       // seek to offset
  virtual sS64 GetOffset();                   // get offset
  virtual sBool SetSize(sS64);                // change size of file on disk
  virtual sS64 GetSize();                     // get size
  virtual sInt GetID();                       // get FileID (only usefull on PS2!!!)

  // asynchronous interface. expect this to be unimplemented for certain filehandlers
  // normal on disk files and uncompressed files from a packfile should work tho.
  virtual sFileReadHandle BeginRead(sS64 offset,sDInt size,void *destbuffer=0);  // begin reading
  virtual sBool DataAvailable(sFileReadHandle handle);  // data valid?
  virtual void *GetData(sFileReadHandle handle);  // access data (only valid if you didn't specify the buffer yourself!)
  virtual void EndRead(sFileReadHandle handle);   // the data buffer may be reused now

  // PLATFORM DEPENDENT: get physical position of part of file.
  virtual sU64 GetPhysicalPosition(sS64 offset);   

  // helper functions
  sU8 *MapAll();                              // map whole file. if mapping is not supported, copy by hand.
  sInt CopyFrom(sFile *f);                    // copies from f into itself
  sInt CopyFrom(sFile *f, sS64 max);          // copies max bytes from f into itself
};

class sFileHandler
{
public:
  virtual ~sFileHandler();
  virtual sFile *Create(const sChar *name,sFileAccess access);
};

void sAddFileHandler(sFileHandler *);
class sFile *sCreateFile(const sChar *name,sFileAccess access=sFA_READ);

class sFile *sCreateMemFile(const void *data,sDInt size,sBool owndata=1);

/****************************************************************************/

// for calculating md5 of serialization without writing it to hardisk
class sCalcMD5File : public sFile
{
  sU8 TempBuf[64];
  sInt TempCount;
  sInt TotalCount;
public:
  sChecksumMD5 Checksum;

  sCalcMD5File();
  ~sCalcMD5File();
  sBool Close();
  sBool Read(void *data,sDInt size);
  sBool Write(const void *data,sDInt size);
  sU8 *Map(sS64 offset,sDInt size);
  sBool SetOffset(sS64 offset);
  sS64 GetOffset();
  sBool SetSize(sS64);
  sS64 GetSize();
};

/****************************************************************************/
/***                                                                      ***/
/***   File Operation                                                     ***/
/***                                                                      ***/
/****************************************************************************/

sBool sCheckFile(const sChar *name);
sU8 *sLoadFile(const sChar *name);
sU8 *sLoadFile(const sChar *name,sDInt &size);
sBool sSaveFile(const sChar *name,const void *data,sDInt bytes);
sChar *sLoadText(const sChar *name);
//sStream *sOpenText(const sChar *name);
sBool sSaveTextAnsi(const sChar *name,const sChar *data);
sBool sBackupFile(const sChar *name);

sBool sFileCalcMD5(const sChar *name, sChecksumMD5 &md5);
template <typename T> sBool sCalcObjectMD5(sChecksumMD5 &md5, T* obj);

/****************************************************************************/

template<class Type> Type *sLoadObject(const sChar *name)
{
  Type *obj = new Type;
  sFile *file = sCreateFile(name,sFA_READ); if(!file) return 0; 
  sReader stream; stream.Begin(file); obj->Serialize(stream); stream.End(); 
  delete file; 
  if(!stream.IsOk())
  {
    sDPrintF(L"[file] error loading <%s>, Serialize failed 1\n",name);
    sDelete(obj);
  }
  return obj;
}

template<class Type> sBool sLoadObject(const sChar *name,Type *obj)
{
  sFile *file = sCreateFile(name,sFA_READ); if(!file) return 0; 
  sReader stream; stream.Begin(file); obj->Serialize(stream); stream.End();
  if(!stream.IsOk())
    sDPrintF(L"[file] error loading <%s>, Serialize failed 2\n",name);
  delete file; return stream.IsOk(); 
}

template<class Type> sBool sSaveObject(const sChar *name,Type *obj)
{
  sFile *file = sCreateFile(name,sFA_WRITE); if(!file) return 0; 
  sWriter stream; stream.Begin(file); obj->Serialize(stream); stream.End(); 
  if(!stream.IsOk())
    sDPrintF(L"[file] error saving <%s>, Serialize failed 3\n",name);
  delete file; return stream.IsOk(); 
}

template <typename T> sBool sCalcObjectMD5(sChecksumMD5 &md5, T* obj)
{
  sCalcMD5File c;
  sWriter s;
  s.Begin(&c);
  obj->Serialize(s);
  if(!s.End()) return sFALSE;
  c.Close();
  md5 = c.Checksum;
  return sTRUE;
}

/****************************************************************************/

void sSetProjectDir(const sChar *name);

struct sDirEntry
{
  sString<256> Name;
  sInt Size;
  sInt Flags;
  sU64 LastWriteTime;
};

enum sDirEntryFlags
{
  sDEF_DIR = 0x0001,
  sDEF_WRITEPROTECT = 0x0002,
  sDEF_EXISTS = 0x0004,
};

sBool sCopyFile(const sChar *source,const sChar *dest,sBool failifexists=0);
sBool sLoadDir(sArray<sDirEntry> &list,const sChar *path,const sChar *pattern=0);
sBool sChangeDir(const sChar *name);
void  sGetCurrentDir(const sStringDesc &str);
void  sGetTempDir(const sStringDesc &str);
sBool sGetFileInfo(const sChar *name,sDirEntry *);
sBool sMakeDir(const sChar *);          // make one directory
sBool sMakeDirAll(const sChar *);       // make all directories. may fail with strage paths.
sBool sCheckDir(const sChar *);
sBool sDeleteFile(const sChar *);

/****************************************************************************/
/***                                                                      ***/
/***   Multithreading                                                     ***/
/***                                                                      ***/
/****************************************************************************/

sInt sGetCPUCount();

enum sThreadConsts
{
  sMAX_THREADCOUNT = 16,
};


void sSleep(sInt ms);

enum sThreadContextFlags
{
  sTHF_NONETDEBUG  = 0x00000001,    // in this thread sDPrintF is not allowed to print debuginfo over the net
};

// search for threadcontext in types.hpp

typedef void (sThreadFunc)(sThread *, void *);
typedef sBool (sThreadFuncBool)(sThread *, void *);

class sThread
{
  void *ThreadHandle;
  sU32 ThreadId;
  volatile sBool TerminateFlag;
  sThreadFunc *Code;
  void *Userdata;
  sU8 *Stack;

  void RegisterToThreadList();
  void UnregisterFromThreadList();


  friend unsigned long sSTDCALL sThreadTrunk(void *ptr);
  friend void * sSTDCALL sThreadTrunk_pthread(void *ptr);
  friend sThread *sGetCurrentThread();
public:
  sThread(sThreadFunc *threadFunc,sInt pri=0,sInt stacksize=0x4000, void *userdata=sNULL);
  ~sThread();

  void Terminate()          { TerminateFlag = 1; }
  sBool CheckTerminate()     { return !TerminateFlag; }

  sThreadContext Context;
};

class sThreadLock
{
  void *CriticalSection;
public:
  sThreadLock();
  ~sThreadLock();
  
  void Lock();
  sBool TryLock();
  void Unlock();
};

class sThreadEvent
{
  void *EventHandle;
public:
  sThreadEvent(sBool manual=sFALSE);    // with manual == sFALSE the signal gets deactivated be first waiting thread
                                        // use manual == sTRUE to disable event only by Reset
                                        // (can be usefull to synchronising multiple threads)
  ~sThreadEvent();
  
  sBool Wait(sInt timeout=-1);     // 0 for immediate return, -1 for infinite wait
  void Signal();                  // the signal is consumed by the first thread for automatic events
  void Reset();                   // go into nonsignaled state
};

sThread        *sGetCurrentThread();
sThreadContext &sGetCurrentThreadContext();


// lock helper
template <typename T> struct sLockable
{
private:
  // shouldn't be copied, so private copy contructor and assignment operator
  sLockable(const sLockable& ) {}
  sLockable &operator=(const sLockable& ) {return *this;}
public:

  T Item;
  sThreadLock *ThreadLock;

  operator T&()             { return Item; }
  T &operator=(const T &v)  { Item = v; return Item; }

  T *operator->()           { return &Item; }
  const T *operator->()const{ return &Item; }

  sLockable():ThreadLock(0) {}
  ~sLockable()              { Exit(); }

  void Init()               { ThreadLock = new sThreadLock; }
  void Exit()               { sDelete(ThreadLock); }

  void Lock()               { ThreadLock->Lock(); }
  void Unlock()             { ThreadLock->Unlock(); }
};

// scope lock helper
class sScopeLock
{
private:
  sThreadLock *Lock;
public:
  sScopeLock(sThreadLock *l):Lock(l) { sVERIFY(l); Lock->Lock(); }
  ~sScopeLock()                      { Lock->Unlock(); }
};

/****************************************************************************/

//ALTONA_BASE_SYSTEM
#endif


