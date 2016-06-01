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
  Thread(SECURITY_ATTRIBUTES* Attributes = NULL, unsigned int Stack = 0)
  {
    /* Initialization */
    Active = false;
    Running = false;
    SuspendCount = 1;
    CriticalSection = NULL;
    SecurityAttributes = Attributes;
    StackSize = Stack;
    Handle = CreateThread(SecurityAttributes, StackSize, ThreadProc, this, CREATE_SUSPENDED, &Id);
  }

  virtual ~Thread()
  {
    /* Stop thread */
    Active = false;

    /* Wait for thread to stop */
    while (Running)
      Sleep(100);

    /* Cleanup */
    CloseHandle(Handle);
    if (CriticalSection != NULL)
    {
      DeleteCriticalSection(CriticalSection);
      delete CriticalSection;
    }
  }

  bool IsActive()
  {
    return Active;
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

  void Stop()
  {
    Active = false;

    /* Wait for thread to stop */
    while (Running)
      Sleep(100);

    TerminateThread(Handle, 0);
    CloseHandle(Handle);
    Handle = CreateThread(SecurityAttributes, StackSize, ThreadProc, this, CREATE_SUSPENDED, &Id);
  }

  void Suspend()
  {
    SuspendThread(Handle);
    SuspendCount ++;
  }

protected:
  virtual CRITICAL_SECTION* CreateCriticalSection()
  {
    CRITICAL_SECTION* Result = new CRITICAL_SECTION();
    InitializeCriticalSection(Result);
    return Result;
  }

  void CriticalSectionBegin()
  {
    if (CriticalSection == NULL)
      CriticalSection = CreateCriticalSection();
    if (CriticalSection != NULL)
      EnterCriticalSection(CriticalSection);
  }

  void CriticalSectionEnd()
  {
    if (CriticalSection != NULL)
      LeaveCriticalSection(CriticalSection);
  }

  virtual unsigned int Run()
  {
    return 0;
  }

private:
  bool Active;
  bool Running;
  unsigned int StackSize;
  unsigned int SuspendCount;
  HANDLE Handle;
  CRITICAL_SECTION* CriticalSection;
  SECURITY_ATTRIBUTES* SecurityAttributes;
  DWORD Id;

  static DWORD WINAPI ThreadProc(void* Param)
  {
    Thread* thread = (Thread*)Param;
    thread->Active = true;
    thread->Running = true;
    unsigned int Result = thread->Run();
    thread->Running = false;
    return Result;
  }

};

#endif /* THREAD_H_ */
