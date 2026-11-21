#include "base/types.hpp"

#if sCONFIG_SYSTEM_LINUX

/****************************************************************************/
#include "system.hpp"

#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>

#include <sys/types.h>
#include <dirent.h>

#include <time.h>

#include <signal.h>
#include <pthread.h>
#include <unistd.h>

/****************************************************************************/
/***                                                                      ***/
/***   Debugging (declared in types.hpp!)                                 ***/
/***                                                                      ***/
/****************************************************************************/

void sCDECL sFatalImpl(const sChar *str)
{
  sDPrint(str);
  sDPrint(sTXT("\n"));

  exit(1);
}

static sBool sDPrintFlag=sFALSE;

void sDPrint(const sChar *text)
{
  sChar8 buffer[2048];

  sCopyString(buffer, text, sCOUNTOF(buffer));


  if (sNetPrint && !(sGetCurrentThreadContext().Flags&sTHF_NONETDEBUG))
  {
    sNetPrint(text);
  }

  fprintf(stderr,buffer);
  //wprintf(text);
  sDPrintFlag=sTRUE;
}

void sDPrintMark()
{
  if (sDPrintFlag)
  {
    sDPrint(L"-----\n");
    sDPrintFlag=sFALSE;
  }
}

void sPrint(const sChar* text)
{
  sChar8 buffer[2048];

  sCopyString(buffer, text, sCOUNTOF(buffer));


  fprintf(stdout,buffer);

  //wprintf(t);
}

sInt sStartTime = 0;

void sInitGetTime()
{
  timespec currentTime;
  clock_gettime(CLOCK_REALTIME, &currentTime);

  sStartTime = currentTime.tv_sec * 1000 + currentTime.tv_nsec/1000000;
}

sInt sGetTime()
{
  timespec currentTime;
  clock_gettime(CLOCK_REALTIME, &currentTime);

  return currentTime.tv_sec * 1000 + currentTime.tv_nsec/1000000 - sStartTime;
}

void sQueueInput(sInt devtype,sInt devnum,sU32 key)
{
}

void sTriggerEvent(sInt event)
{
  sFatal(L"sTriggerEvent not implemented.\n");
}






/****************************************************************************/
/***                                                                      ***/
/***   Multithreading                                                     ***/
/***                                                                      ***/
/****************************************************************************/


sStaticArray<sThread*> sThreadList;
sThreadLock            *sThreadListLock = sNULL;

sThreadContext         sThreadContextMain;            // the context of the mainthread

/****************************************************************************/

// sThread::ThreadId is 32 bit but pthread_t is 64 bit: compare a folded hash instead
static sU32 sFoldThreadId(pthread_t t) { unsigned long v = (unsigned long)t; sU32 r = (sU32)(v ^ (v>>32)); return r ? r : 1; }

sThread *sGetCurrentThread()
{
  sThread *result = sNULL;

  sThreadListLock->Lock();

  sInt i = sThreadList.GetCount();
  while (!result && i--)
  {
    result = sThreadList[i];
    
    if ( sFoldThreadId(pthread_self()) != result->ThreadId )
      result = sNULL;
  }

  sThreadListLock->Unlock();

  return result;
}

/****************************************************************************/


sThreadContext &sGetCurrentThreadContext()
{
  sThread *thread = sGetCurrentThread();

  return thread ? thread->Context : sThreadContextMain;
}

/****************************************************************************/

void sThreadContext::Init()
{
  Flags       = 0;
  UserData    = sNULL;
  ThreadName  = L"unnamed";
  MemSegId    = sMEMSEG_HEAP;
}

/****************************************************************************/

void sSleep(sInt ms)
{
  usleep(ms*1000); //uSleep need microseconds
}

/****************************************************************************/


void * sSTDCALL sThreadTrunk_pthread(void *ptr)
{
  sThread *th = (sThread *) ptr;

  // wait for the ThreadId to be set in the mainthread
  while (!th->ThreadId)
   sSleep(10);


  sU32 self = sFoldThreadId(pthread_self());
  sDPrintF(L"New sThread started. 0x%x\n", self);

  sVERIFY( sFoldThreadId(pthread_self()) == th->ThreadId);

  th->Code(th,th->Userdata);

  return 0;
}

/****************************************************************************/

sThread::sThread(void (*code)(sThread *,void *),sInt pri,sInt stacksize,void *userdata)
{

  
  TerminateFlag = 0;
  Code = code;
  Userdata = userdata;

  Context.Init();

  RegisterToThreadList();
 
  ThreadHandle = new pthread_t;
  ThreadId     = 0;

  // windows: ThreadHandle = CreateThread(0,stacksize,sThreadTrunk,this,0,(ULONG *)&ThreadId);

  int result = pthread_create((pthread_t*)ThreadHandle, sNULL, sThreadTrunk_pthread, this);

  sVERIFY(result==0);

  ThreadId = sFoldThreadId(*(pthread_t*)ThreadHandle);

// clone(sThreadTrunk, StackMemory + stacksize - 1,  CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_VM|CLONE_THREAD ,this);

//  if(pri>0)
//  {
//    SetPriorityClass(ThreadHandle,HIGH_PRIORITY_CLASS);
//    SetThreadPriority(ThreadHandle,THREAD_PRIORITY_TIME_CRITICAL);
//  }
//  if(pri<0)
//  {
////    SetPriorityClass(ThreadHandle,BELOW_NORMAL_PRIORITY_CLASS);
//    SetThreadPriority(ThreadHandle,THREAD_PRIORITY_BELOW_NORMAL);
//  }
}

/****************************************************************************/

sThread::~sThread()
{
  if(TerminateFlag==0)
    Terminate();
  if(TerminateFlag==1)
  {
    // windows: WaitForSingleObject(ThreadHandle,INFINITE);
    pthread_join(*(pthread_t*)ThreadHandle, sNULL);
  }
  // windows: CloseHandle(ThreadHandle);
  // not needed under linux

  UnregisterFromThreadList();

  delete (pthread_t*)ThreadHandle;
}

/****************************************************************************/

void sThread::RegisterToThreadList()
{
  sThreadListLock->Lock();

  sBool found = sFALSE;

  sInt i = sThreadList.GetCount();

  // check if this thread isn't already registered
  while (!found && i--)
  {
    found = sThreadList[i] == this;
  }

  // register
  if (!found)
  {
    sVERIFY(sThreadList.GetCount()<=sMAX_THREADCOUNT);
    sThreadList.AddTail(this);
  }

  sThreadListLock->Unlock();
}

/****************************************************************************/

void sThread::UnregisterFromThreadList()
{
  sThreadListLock->Lock();

  sThreadList.Rem(this);  

  sThreadListLock->Unlock();
}

/****************************************************************************/

void sInitThread()
{
  sThreadList.HintSize(sMAX_THREADCOUNT);
  sThreadListLock = new sThreadLock;

  sThreadContext &threadContext = sGetCurrentThreadContext();
  threadContext.Init();
  threadContext.ThreadName = L"MainThread";
}

/****************************************************************************/

void sExitThread()
{
  sThreadList.Reset();
  sDelete(sThreadListLock);
}

/****************************************************************************/

sThreadLock::sThreadLock()
{
  CriticalSection = new pthread_mutex_t;

  pthread_mutexattr_t mattr;
  pthread_mutexattr_init(&mattr);
  pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
  pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);   // msglib re-enters its critical sections, so the mutexes have to be recursive
  pthread_mutex_init((pthread_mutex_t*) CriticalSection, &mattr);  
}

/****************************************************************************/

sThreadLock::~sThreadLock()
{
  delete (pthread_mutex_t*)CriticalSection;
}

/****************************************************************************/

void sThreadLock::Lock()
{
  pthread_mutex_lock((pthread_mutex_t*) CriticalSection);
}

/****************************************************************************/

void sThreadLock::Unlock()
{
  pthread_mutex_unlock((pthread_mutex_t*) CriticalSection);

}

/****************************************************************************/


void sRunLoop();
int main(int argc, char **argv)
{ 
  
  sInitGetTime();

  sInitThread();


  sChar commandLine[2048];
  
  // create a commandline
  commandLine[0] = 0;
  for (sInt i=0; i<argc; i++)
  {
    sChar buffer[2048];
    if(i>0 && argv[i][0]!='-') sAppendString(commandLine,L"\"",sCOUNTOF(commandLine));
    sCopyString(buffer,argv[i],sCOUNTOF(buffer));
    sAppendString(commandLine,buffer,sCOUNTOF(commandLine));
    if(i>0 && argv[i][0]!='-') sAppendString(commandLine,L"\"",sCOUNTOF(commandLine));
    sAppendString(commandLine,L" ",sCOUNTOF(commandLine));
  }
  
  sParseCmdLine(commandLine); 

  // ignore brogen pipe signal
  signal(SIGPIPE, SIG_IGN);

 
  sSetRunlevel(0x80);
  sMain();
  sRunLoop();
  sSetRunlevel(0x00);

  sExitThread();

  return 0;
} 

/****************************************************************************/

#else

sInt sDummyLink_system_linux;

#endif    // linux

