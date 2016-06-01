/*
* DiagUtils.h - A collection of Windows functions that display various system dialogs.
*
* v 1.0
*
* Copyright (C) 2007-2010 Marc-André Lamothe.
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
#ifndef DialogUtils_H_
#define DialogUtils_H_

#include <string>
#include <windows.h>

using namespace std;

#define OPF_PRINTERNAME 0x01
#define OPF_PATHNAME 0x02

typedef void (__stdcall *pSHObjectProperties)(HWND, UINT, LPCSTR, LPCSTR);

inline bool OpenFileDialog(HWND Parent, string& FileName, const char* InitialDir, const char* DefExt, const char* Filters, const int FilterIndex)
{
  bool Result = false;
  OPENFILENAME* File = new OPENFILENAME;
  File->lStructSize = sizeof(OPENFILENAME);
  File->hwndOwner = Parent;
  File->hInstance = NULL;
  File->lpstrFilter = Filters;
  File->lpstrCustomFilter = NULL;
  File->nMaxCustFilter = 0;
  File->nFilterIndex = FilterIndex;
  File->lpstrFile = new char[MAX_PATH*2];
  File->nMaxFile = MAX_PATH*2;
  File->lpstrFileTitle = NULL;
  File->nMaxFileTitle = 0;
  File->lpstrInitialDir = InitialDir;
  File->lpstrTitle = NULL;
  File->Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER;
  File->nFileOffset = 0;
  File->nFileExtension = 0;
  File->lpstrDefExt = DefExt;
  File->lCustData = 0;
  File->lpfnHook = NULL;
  File->lpTemplateName = NULL;
  strcpy(File->lpstrFile, FileName.c_str());
  if (GetOpenFileName(File) != 0)
  {
    FileName.assign(File->lpstrFile);
    Result = true;
  }
  delete[] File->lpstrFile;
  delete File;
  return Result;
}

inline void FilePropertiesDialog(const char* FileName)
{
  BOOL (__stdcall *SHObjectProperties)(HWND,UINT,LPCSTR,LPCSTR);
  SHObjectProperties = (BOOL (WINAPI*)(HWND,UINT,LPCSTR,LPCSTR))GetProcAddress(GetModuleHandle("SHELL32.DLL"), (LPCSTR)178);
  (*SHObjectProperties)(NULL, OPF_PATHNAME, "c:\\a.txt", NULL);
}

inline HDC PrintDialog(HWND Parent)
{
  PRINTDLG DlgInfo;
  DlgInfo.lStructSize = sizeof(PRINTDLG);
  DlgInfo.hwndOwner = Parent;
  DlgInfo.hDevMode = NULL;
  DlgInfo.hDevNames = NULL;
  DlgInfo.Flags = PD_ALLPAGES | PD_NOPAGENUMS | PD_NOSELECTION | PD_RETURNDC;
  DlgInfo.nCopies = 1;
  PrintDlg(&DlgInfo);
  return DlgInfo.hDC;
}

inline bool SaveFileAsDialog(HWND Parent, string& FileName, const char* InitialDir, const char* DefExt, const char* Filters, const int FilterIndex)
{
  bool Result = false;
  OPENFILENAME* File = new OPENFILENAME;
  File->lStructSize = sizeof(OPENFILENAME);
  File->hwndOwner = Parent;
  File->hInstance = NULL;
  File->lpstrFilter = Filters;
  File->lpstrCustomFilter = NULL;
  File->nMaxCustFilter = 0;
  File->nFilterIndex = FilterIndex;
  File->lpstrFile = new char[MAX_PATH*2];
  File->nMaxFile = MAX_PATH*2;
  File->lpstrFileTitle = NULL;
  File->nMaxFileTitle = 0;
  File->lpstrInitialDir = InitialDir;
  File->lpstrTitle = NULL;
  File->Flags = OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_EXPLORER;
  File->nFileOffset = 0;
  File->nFileExtension = 0;
  File->lpstrDefExt = DefExt;
  File->lCustData = 0;
  File->lpfnHook = NULL;
  File->lpTemplateName = NULL;
  strcpy(File->lpstrFile, FileName.c_str());
  if (GetSaveFileName(File) != 0)
  {
    FileName.assign(File->lpstrFile);
    Result = true;
  }
  delete[] File->lpstrFile;
  delete File;
  return Result;
}

#endif
