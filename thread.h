/*
 * thread.h
 *
 *  Created on: 2010-09-19
 *      Author: Marc
 *
 *  See Windows CreateThread and SetThreadPriority functions for values to constructor's parameters
 */

#ifndef THREAD_H_
#define THREAD_H_

#include <windows.h>

class Thread
{
public:
  Thread(SECURITY_ATTRIBUTES* Attributes = NULL, unsigned int StackSize = 0)
  {
    /* Initialization */
    Active = false;
    SuspendCount = 1;
    Handle = CreateThread(Attributes, StackSize, ThreadProc, this, CREATE_SUSPENDED, &Id);
    CriticalSection = new CRITICAL_SECTION();
    InitializeCriticalSection(CriticalSection);
  }

  virtual ~Thread()
  {
    /* Stop thread */
    Active = false;

    /* Cleanup */
    CloseHandle(Handle);
    DeleteCriticalSection(CriticalSection);
    delete CriticalSection;
  }

  void CriticalSectionBegin()
  {
    EnterCriticalSection(CriticalSection);
  }

  void CriticalSectionEnd()
  {
    LeaveCriticalSection(CriticalSection);
  }

  int IsSuspended()
  {
    return (SuspendCount > 0);
  }

  int GetPriority()
  {
    return GetThreadPriority(Handle);
  }

  void Resume()
  {
    ResumeThread(Handle);
    if (SuspendCount > 0)
      SuspendCount --;
  }

  void SetPriority(int Priority)
  {
    SetThreadPriority(Handle,Priority);
  }

  void Suspend()
  {
    SuspendThread(Handle);
    SuspendCount ++;
  }

protected:
  bool Active;
  unsigned int SuspendCount;
  HANDLE Handle;
  CRITICAL_SECTION* CriticalSection;
  DWORD Id;

  virtual int Run()
  {
    return 0;
  }

  static DWORD WINAPI ThreadProc(void* Param)
  {
    Thread* thread = (Thread*)Param;
    thread->Active = true;
    return thread->Run();
  }
};

#endif /* THREAD_H_ */
