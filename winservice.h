/*
* WinService.h - A wrapper for the Windows API service system.
*
* v 1.0
*
* Copyright (C) 2011 Marc-André Lamothe.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Library General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/
#ifndef WinService_H_
#define WinService_H_

#if (!defined _WIN32_WINNT || _WIN32_WINNT < 0x0501)
  #define _WIN32_WINNT 0x0501 // Windows NT
#endif
#include <cstdio>
#include <windows.h>
#include "observer.h"

enum WinServiceEvent {ServiceStarting = 901, ServiceStarted, ServiceStopping, ServiceStopped};

class WinService : public Observable
{
public:
  ~WinService()
  {
  }

  static WinService* GetInstance()
  {
    static WinService* Instance = NULL;
    if (Instance == NULL)
      Instance = new WinService();
    return Instance;
  }

  bool Initialize(const char* ParentService = "", DWORD ServiceType = SERVICE_WIN32_OWN_PROCESS)
  {
    if (ParentService != NULL)
    {
      char Name[MAX_PATH];
      strncpy(Name, ParentService, MAX_PATH);
      State = SERVICE_STOPPED;
      Type = ServiceType;

      SERVICE_TABLE_ENTRY Entries[2];
      Entries[0].lpServiceName = Name;
      Entries[0].lpServiceProc = ServiceProc;
      Entries[1].lpServiceName = NULL;
      Entries[1].lpServiceProc = NULL;

      if (StartServiceCtrlDispatcher(Entries) != 0)
        return true;
      else
        OutputDebugString(GetErrorMessage("StartServiceCtrlDispatcher()"));
    }
    return false;
  }

  static bool Install(const char* Name, const char* DisplayName, const char* Params = "", DWORD Type = SERVICE_WIN32_OWN_PROCESS, DWORD StartupType = SERVICE_AUTO_START, DWORD ErrorControl = SERVICE_ERROR_NORMAL)
  {
    // Get a handle to the SCM
    SC_HANDLE ServiceManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (ServiceManager != NULL)
    {
      char* Command = new char[MAX_PATH];
      char* FileName = new char[MAX_PATH];
      SC_HANDLE Service = NULL;

      if (GetModuleFileName(NULL, FileName, MAX_PATH) != 0)
      {
        ZeroMemory(Command, sizeof(Command));
        strcat(Command, "\"");
        strcat(Command, FileName);
        strcat(Command, "\" ");
        strcat(Command, Params);

        // Install the service
        Service = CreateService(ServiceManager,Name,DisplayName,SERVICE_ALL_ACCESS,Type,StartupType,ErrorControl,Command,NULL,NULL,NULL,NULL,NULL);
        CloseServiceHandle(ServiceManager);
        CloseServiceHandle(Service);
      }
      delete[] FileName;
      delete[] Command;
      return (Service != NULL);
    }
    return false;
  }

  bool IsInstalled(const char* Name)
  {
    // Get a handle to the SCM
    SC_HANDLE ServiceManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (ServiceManager != NULL)
    {
      // Find the service
      SC_HANDLE Service = OpenService(ServiceManager,Name,SERVICE_ALL_ACCESS);
      CloseServiceHandle(ServiceManager);
      CloseServiceHandle(Service);
      if (Service != NULL)
        return true;
    }
    return false;
  }

  void Start()
  {
    SetState(SERVICE_START_PENDING);
    OutputDebugString("SERVICE_STARTING.\n");

    NotifyObservers(ServiceStarting);

    SetState(SERVICE_RUNNING);
    OutputDebugString("SERVICE_STARTED.\n");

    NotifyObservers(ServiceStarted);
  }

  void Stop()
  {
    SetState(SERVICE_STOP_PENDING);
    OutputDebugString("SERVICE_STOPPING.\n");

    NotifyObservers(ServiceStopping);

    SetState(SERVICE_STOPPED);
    OutputDebugString("SERVICE_STOPPED.\n");

    NotifyObservers(ServiceStopped);
  }

  static bool Uninstall(const char* Name)
  {
    // Get a handle to the SCM
    SC_HANDLE ServiceManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (ServiceManager != NULL)
    {
      // Uninstall the service
      SC_HANDLE Service = OpenService(ServiceManager,Name,SERVICE_ALL_ACCESS);
      CloseServiceHandle(ServiceManager);
      if (Service != NULL && DeleteService(Service) != 0)
        return true;
    }
    return false;
  }

protected:

  WinService()
  {
  }

private:
  SERVICE_STATUS_HANDLE hStatus;

  DWORD State;
  DWORD Type;

  static char* GetErrorMessage(const char* Message)
  {
    unsigned long ErrorCode = GetLastError();
    char* Buffer = new char[256];
    char* Result = new char[1024];

    sprintf(Buffer, "API = %s, ", Message);
    strcat(Result, Buffer);
    ZeroMemory(Buffer, sizeof(Buffer));
    sprintf(Buffer, "error code = %u, ", (unsigned int)ErrorCode);
    strcat(Result, Buffer);

    LPVOID ErrorMessage;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,NULL,ErrorCode,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR)&ErrorMessage,0,NULL);
    ZeroMemory(Buffer, sizeof(Buffer));
    sprintf(Buffer, "message = %s", (char*)ErrorMessage);
    LocalFree(ErrorMessage);
    strcat(Result, Buffer);

    return Result;
  }

  void SetState(DWORD ServiceState, DWORD ExitCode = NO_ERROR, DWORD CheckPoint = 0, DWORD WaitHint = 0)
  {
    State = ServiceState;

    SERVICE_STATUS Status;
    Status.dwControlsAccepted = (State == SERVICE_START_PENDING ? 0 : SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
    Status.dwServiceType = Type;
    Status.dwServiceSpecificExitCode = 0;
    Status.dwCurrentState = State;
    Status.dwWin32ExitCode = ExitCode;
    Status.dwCheckPoint = CheckPoint;
    Status.dwWaitHint = WaitHint;
    if (!SetServiceStatus(hStatus, &Status))
    {
      OutputDebugString(GetErrorMessage("SetServiceStatus()"));
      Stop();
    }
  }

  static void __stdcall ServiceHandler(DWORD ControlCode)
  {
    // Handle the control code
    switch (ControlCode)
    {
      case SERVICE_CONTROL_INTERROGATE:
      {
        GetInstance()->SetState(GetInstance()->State);
        break;
      }
      case SERVICE_CONTROL_STOP:
      case SERVICE_CONTROL_SHUTDOWN:
      {
        GetInstance()->Stop();
        break;
      }
    }
  }

  static void __stdcall ServiceProc(DWORD dwArgc, LPSTR* lpszArgv)
  {
    // Register the control handler
    GetInstance()->hStatus = RegisterServiceCtrlHandler(lpszArgv[0], (LPHANDLER_FUNCTION)ServiceHandler);

    // Start the service
    GetInstance()->Start();
  }

};

#endif
