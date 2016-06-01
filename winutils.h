/*
* WinUtils.h - A collection of functions for the Windows API.
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
#ifndef WinUtils_H_
#define WinUtils_H_

#if (!defined _WIN32_IE || _WIN32_IE < 0x0300)
  #define _WIN32_IE 0x0300 // IE 3.0 controls
#endif
#include "cstrutils.h"
#include <windows.h>
#include <commctrl.h>

#define fsBold      1
#define fsItalic    2
#define fsUnderline 4

struct FontInfo
{
  char Name[LF_FACESIZE];
  int Size;
  unsigned short Style;
};

struct Fraction
{
  int numerator;
  int denominator;
};

static HWND PreviousInstance = NULL;

char* FormatDateTime(const SYSTEMTIME* DateTime, const int DateFlag, const char* DateFormat, const int TimeFlag, const char* TimeFormat);

inline void AddListViewCollumn(const HWND List, char* Text, const int Index, const int Width)
{
  LVCOLUMN CollumnInfo;
  CollumnInfo.mask = LVCF_WIDTH | LVCF_TEXT;
  CollumnInfo.cx = Width;
  CollumnInfo.pszText = Text;
  ListView_InsertColumn(List, Index, &CollumnInfo);
}

inline void AddToLog(const char* LogFile, const char* String)
{
  HANDLE File = CreateFile(LogFile, FILE_WRITE_DATA, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (File == INVALID_HANDLE_VALUE && GetLastError() == ERROR_FILE_NOT_FOUND)
    File = CreateFile(LogFile, FILE_WRITE_DATA, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
  if (File != INVALID_HANDLE_VALUE && SetFilePointer(File, 0, NULL, FILE_END) != INVALID_SET_FILE_POINTER)
  {
    DWORD N;
    SYSTEMTIME SystemTime;
    GetSystemTime(&SystemTime);
    char* Datetime = FormatDateTime(&SystemTime, DATE_SHORTDATE, NULL, TIME_FORCE24HOURFORMAT | TIME_NOSECONDS | TIME_NOTIMEMARKER, NULL);
    WriteFile(File, Datetime, strlen(Datetime), &N, NULL);
    delete[] Datetime;
    WriteFile(File, ": ", 2, &N, NULL);
    WriteFile(File, String, strlen(String), &N, NULL);
    WriteFile(File, "\n", 1, &N, NULL);
    CloseHandle(File);
  }
}

inline WCHAR* AnsiToUnicode(const char* Str)
{
  if (Str == NULL)
  {
    int Size = strlen(Str);
    WCHAR* Result = new WCHAR[Size+1];
    if (Size > 0)
      MultiByteToWideChar(CP_ACP, 0, Str, Size, Result, Size);
    Result[Size] = '\0';
    return Result;
  }
  return NULL;
}

inline void AppendMenu(const HMENU hMenu, const UINT Id)
{
  char* Str = new char[MAX_PATH];
  LoadString(GetModuleHandle(NULL), Id, Str, MAX_PATH);
  AppendMenu(hMenu,MF_STRING,Id,Str);
  delete[] Str;
}

inline void AppendMenu(const HMENU hMenu, const UINT Id, const HMENU hSubMenu)
{
  char* Str = new char[MAX_PATH];
  LoadString(GetModuleHandle(NULL), Id, Str, MAX_PATH);
  AppendMenu(hMenu,MF_POPUP | MF_STRING,(UINT_PTR)hSubMenu,Str);
  delete[] Str;
}

inline void AppendSeparator(const HMENU hMenu)
{
  AppendMenu(hMenu,MF_SEPARATOR,0,NULL);
}

inline HBRUSH CreateHollowBrush()
{
  LOGBRUSH Brush;
  Brush.lbStyle = BS_HOLLOW;
  return CreateBrushIndirect(&Brush);
}

inline Fraction DecimalToFraction(const float value, const int precision = 1000)
{
  Fraction result;

  // Extract integer and decimal parts
  int integer = (int)value;
  int decimal = (int)((value-integer)*precision);

  // Find the base demoninator and numerator
  result.denominator = 1;
  while (decimal/result.denominator != 0)
    result.denominator = result.denominator*10;
  result.numerator = decimal + integer*result.denominator;

  // Reduce the fraction
  int i = 2;
  while (i <= result.numerator)
  {
    float n = (float)result.numerator/i;
    float d = (float)result.denominator/i;
    if (n == (int)n && d == (int)d)
    {
      result.numerator = (int)n;
      result.denominator = (int)d;
    }
    else
      ++i;
  }
  return result;
}

inline bool DirectoryExists(const char* dirName)
{
  DWORD result = GetFileAttributes(dirName);
  if (result == INVALID_FILE_ATTRIBUTES)
    return false;
  return (result & FILE_ATTRIBUTE_DIRECTORY);
}

inline HFONT EasyCreateFont(const HDC DC, const char* FontName, const int Size, const unsigned short Style)
{
  HDC hDC = DC;
  if (DC == NULL)
    hDC = GetDC(NULL);
  HFONT Font = CreateFont(-MulDiv(Size, GetDeviceCaps(hDC, LOGPIXELSY), 72),
    0, 0, 0, (Style & fsBold) == fsBold ? FW_BOLD : FW_NORMAL,
    (Style & fsItalic) == fsItalic, (Style & fsUnderline) == fsUnderline, false,
    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
    DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, FontName);
  if (DC == NULL)
    ReleaseDC(NULL,hDC);
  return Font;
}

inline HFONT EasyCreateFont(const HDC DC, const FontInfo* Font)
{
  return EasyCreateFont(DC, Font->Name, Font->Size, Font->Style);
}

static inline BOOL __stdcall EnumWndProc(const HWND hWnd, const LPARAM lParam)
{
  char ClassName[128];
  GetClassName(hWnd, ClassName, 128);
  if (!lstrcmp(ClassName, (const char*)lParam))
  {
    PreviousInstance = hWnd;
    return FALSE;
  }
  return TRUE;
}

inline HWND FindPreviousInstance(const char* ClassName)
{
  if (EnumWindows((WNDENUMPROC)EnumWndProc, (LPARAM)ClassName))
    return PreviousInstance;
  return NULL;
}

inline char* FormatDate(const SYSTEMTIME Time, const int Flag, const char* Format)
{
  char* Result = new char[MAX_PATH];
  /* Format to string */
  GetDateFormat(LOCALE_USER_DEFAULT, Flag, &Time, Format, Result, MAX_PATH);
  return Result;
}

inline char* FormatDateTime(const SYSTEMTIME* DateTime, const int DateFlag, const char* DateFormat, const int TimeFlag, const char* TimeFormat)
{
  char* Result = new char[140];
  /* Format to string */
  GetDateFormat(LOCALE_USER_DEFAULT, DateFlag, DateTime, DateFormat, Result, 128);
  strcat(Result, " ");
  char* Time = new char[12];
  GetTimeFormat(LOCALE_USER_DEFAULT, TimeFlag, DateTime, TimeFormat, Time, 12);
  strcat(Result, Time);
  delete[] Time;
  return Result;
}

inline char* FormatTime(const SYSTEMTIME Time, const int Flag, const char* Format)
{
  char* Result = new char[12];
  /* Format to string */
  GetTimeFormat(LOCALE_USER_DEFAULT, Flag, &Time, Format, Result, 12);
  return Result;
}

inline char* GetApplicationPath()
{
  char* Str = new char[MAX_PATH];
  int Size = GetModuleFileName(NULL, Str, MAX_PATH);
  if (Size > 0)
  {
    if (Size >= MAX_PATH)
    {
      delete[] Str;
      Str = new char[MAX_PATH*2];
      Size = GetModuleFileName(NULL, Str, MAX_PATH*2);
    }
    if (Size > 0)
    {
      char* Result = substr(Str,0,strrpos(Str,"\\"));
      delete[] Str;
      return Result;
    }
  }
  strcpy(Str,"");
  return Str;
}

inline char* GetCurrentPath()
{
  char* Str = new char[MAX_PATH];
  int Size = GetCurrentDirectory(MAX_PATH,Str);
  if (Size > 0)
  {
    if (Size > MAX_PATH)
    {
      delete[] Str;
      Str = new char[Size];
      Size = GetCurrentDirectory(Size,Str);
    }
    if (Size > 0)
      return Str;
  }
  strcpy(Str,"");
  return Str;
}

inline float GetDPIScaleFactor()
{
  float scaleFactor = 1;
  HDC hDC = GetDC(NULL);
  if (hDC != NULL)
  {
    scaleFactor = (GetDeviceCaps(hDC, LOGPIXELSX)/96.0);
    ReleaseDC(NULL, hDC);
  }
  return scaleFactor;
}

inline bool GetFileCreationDate(const char* FileName, FILETIME* FileTime)
{
  WIN32_FIND_DATA Data;
  HANDLE hFind = FindFirstFile(FileName, &Data);
  if (hFind != INVALID_HANDLE_VALUE)
  {
    FindClose(hFind);
    *FileTime = Data.ftCreationTime;
    return true;
  }
  return false;
}

inline bool GetFileModificationDate(const char* FileName, FILETIME* FileTime)
{
  WIN32_FIND_DATA Data;
  HANDLE hFind = FindFirstFile(FileName, &Data);
  if (hFind != INVALID_HANDLE_VALUE)
  {
    FindClose(hFind);
    *FileTime = Data.ftLastWriteTime;
    return true;
  }
  return false;
}

inline long GetFileSize(const char* FileName)
{
  WIN32_FIND_DATA Data;
  HANDLE hFind = FindFirstFile(FileName, &Data);
  if (hFind != INVALID_HANDLE_VALUE)
  {
    FindClose(hFind);
    return (Data.nFileSizeHigh*MAXDWORD) + Data.nFileSizeLow;
  }
  return -1;
}

inline bool GetPrivateProfileBool(const char* Section, const char* Key, bool Default, const char* FileName)
{
  char* Data = new char[32];
  GetPrivateProfileString(Section, Key, (Default ? "true" : "false"), Data, 32, FileName);
  if (strcmp(Data, "true") == 0)
    return true;
  return false;
}

inline SYSTEMTIME GetPrivateProfileDatetime(const char* Section, const char* Key, SYSTEMTIME Default, const char* FileName)
{
  SYSTEMTIME Result;
  char* DefStr = FormatDateTime(&Default,0,"dd/MM/yyyy",0,"HH:mm");
  char* Data = new char[32];
  GetPrivateProfileString(Section, Key, DefStr, Data, 32, FileName);
  int Sep1 = strpos(Data,"/",0);
  int Sep2 = strpos(Data,"/",Sep1+1);
  int Sep3 = strpos(Data," ",Sep2+1);
  int Sep4 = strpos(Data,":",Sep3+1);
  char* Str = substr(Data,0,Sep1);
  Result.wDay = atoi(Str);
  delete[] Str;
  Str = substr(Data,Sep1+1,Sep2);
  Result.wMonth = atoi(Str);
  delete[] Str;
  Str = substr(Data,Sep2+1,Sep3);
  Result.wYear = atoi(Str);
  delete[] Str;
  Str = substr(Data,Sep3+1,Sep4);
  Result.wHour = atoi(Str);
  delete[] Str;
  Str = substr(Data,Sep4+1,strlen(Data));
  Result.wMinute = atoi(Str);
  delete[] Str;
  delete[] DefStr;
  delete[] Data;
  return Result;
}

inline int GetPrivateProfileInteger(const char* Section, const char* Key, int Default, const char* FileName)
{
  char* DefStr = inttostr(Default);
  char* Data = new char[32];
  GetPrivateProfileString(Section, Key, DefStr, Data, 32, FileName);
  return atoi(Data);
}

inline unsigned long GetSecondsSince(const FILETIME* ft1, const FILETIME* ft2)
{
  __int64 time1 = (((__int64)ft1->dwHighDateTime << 32) + ft1->dwLowDateTime) / 10000000;
  __int64 time2 = (((__int64)ft2->dwHighDateTime << 32) + ft2->dwLowDateTime) / 10000000;
  return time2 - time1;
}

inline unsigned long GetSecondsSince(const SYSTEMTIME* st1, const SYSTEMTIME* st2)
{
  FILETIME ft1;
  FILETIME ft2;
  SystemTimeToFileTime(st1, &ft1);
  SystemTimeToFileTime(st2, &ft2);
  return GetSecondsSince(&ft1, &ft2);
}

inline char* GetWindowText(const HWND hWnd)
{
  int Len = GetWindowTextLength(hWnd);
  char* Str = new char[Len+1];
  GetWindowText(hWnd, Str, Len+1);
  return Str;
}

inline bool IsDirectory(const char* FileName)
{
  WIN32_FIND_DATA Data;
  HANDLE hFind = FindFirstFile(FileName, &Data);
  if (hFind != INVALID_HANDLE_VALUE)
  {
    FindClose(hFind);
    return (Data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
  }
  return false;
}

inline bool IsMenuItemChecked(const HMENU hMenu, const UINT Id)
{
  return GetMenuState(hMenu, Id, MF_BYCOMMAND) & MF_CHECKED;
}

inline SYSTEMTIME LocalTimeToSystemTime(const SYSTEMTIME DateTime)
{
  SYSTEMTIME Result = DateTime;
  /* Get Timezone information */
  TIME_ZONE_INFORMATION TimeZone;
  GetTimeZoneInformation(&TimeZone);
  /* Apply timezone to the file time */
  Result.wHour += (int)(TimeZone.Bias/60);
  Result.wMinute += (int)(TimeZone.Bias%60);
  return Result;
}

inline int PointsToPixels(const HDC DC, const int value)
{
  return MulDiv(value, GetDeviceCaps(DC, LOGPIXELSY), 72);
}

inline int PixelsToPoints(const HDC DC, const int value)
{
  return MulDiv(value, 72, GetDeviceCaps(DC, LOGPIXELSY));
}

inline int ApplyDPIScaling(const HDC DC, const int Size)
{
  return MulDiv(Size, 96, GetDeviceCaps(DC, LOGPIXELSY));
}

inline int RemoveDPIScaling(const HDC DC, const int Size)
{
  return MulDiv(Size, GetDeviceCaps(DC, LOGPIXELSY), 96);
}

inline POINT Point(const long x, const long y)
{
  POINT Pt;
  Pt.x = x;
  Pt.y = y;
  return Pt;
}

inline int PointCmp(const POINT a, const POINT b)
{
  if (a.y == b.y)
    return a.x - b.x;
  else
    return a.y - b.y;
}

inline void PopulateComboList(const HWND hDlg, const int ID, const char* Text)
{
  /* Copy the text */
  char* Str = new char[strlen(Text)+1];
  strcpy(Str, Text);
  /* Replace all the ; with \0 */
  int Count = 1;
  for (unsigned int i=0; i < strlen(Text); i++)
    if (Str[i] == ';')
    {
      Str[i] = '\0';
      Count ++;
    }
  /* Add the strings to the list */
  char* Ptr = Str;
  for (int i=0; i < Count; i++)
  {
    SendDlgItemMessage(hDlg, ID, CB_ADDSTRING, 0, (LPARAM)Ptr);
    Ptr = Ptr+strlen(Ptr)+1;
  }
  delete[] Str;
}

inline void SelectComboBoxItem(const HWND hWnd, const char* Text)
{
  int Index = SendMessage(hWnd, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)Text);
  if (Index != CB_ERR)
    SendMessage(hWnd, CB_SETCURSEL, Index, 0);
}

inline void SelectComboBoxItem(const HWND hDlg, const int ControlID, const char* Text)
{
  int Index = SendDlgItemMessage(hDlg, ControlID, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)Text);
  if (Index != CB_ERR)
    SendDlgItemMessage(hDlg, ControlID, CB_SETCURSEL, Index, 0);
}

inline void SelectListBoxItem(const HWND hWnd, const char* Text)
{
  int Index = SendMessage(hWnd, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)Text);
  if (Index != LB_ERR)
    SendMessage(hWnd, LB_SETCURSEL, Index, 0);
}

inline void SelectListBoxItem(const HWND hDlg, const int ControlID, const char* Text)
{
  int Index = SendDlgItemMessage(hDlg, ControlID, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)Text);
  if (Index != LB_ERR)
    SendDlgItemMessage(hDlg, ControlID, LB_SETCURSEL, Index, 0);
}

inline SYSTEMTIME SystemTimeToLocalTime(const SYSTEMTIME* DateTime)
{
  SYSTEMTIME Result = *DateTime;
  /* Get Timezone information */
  TIME_ZONE_INFORMATION TimeZone;
  GetTimeZoneInformation(&TimeZone);
  /* Apply timezone to the file time */
  Result.wHour -= (int)(TimeZone.Bias/60);
  Result.wHour -= (int)(TimeZone.StandardBias/60);
  Result.wMinute -= (int)(TimeZone.Bias%60);
  Result.wMinute -= (int)(TimeZone.StandardBias%60);
  return Result;
}

inline char* UnicodeToAnsi(const WCHAR* Str)
{
  if (Str == NULL)
  {
    int Size = lstrlenW(Str);
    char* Result = new char[Size+1];
    if (Size > 0)
      WideCharToMultiByte(CP_ACP, 0, Str, Size, Result, Size, NULL, NULL);
    Result[Size] = '\0';
    return Result;
  }
  return NULL;
}

inline BOOL WritePrivateProfileBool(const char* Section, const char* Key, bool Value, const char* FileName)
{
  const char* Str = (Value ? "true" : "false");
  return WritePrivateProfileString(Section, Key, Str, FileName);
}

inline BOOL WritePrivateProfileDatetime(const char* Section, const char* Key, SYSTEMTIME Value, const char* FileName)
{
  char* Str = FormatDateTime(&Value,0,"dd/MM/yyyy",0,"HH:mm");
  BOOL Result = WritePrivateProfileString(Section, Key, Str, FileName);
  delete[] Str;
  return Result;
}

inline BOOL WritePrivateProfileInteger(const char* Section, const char* Key, int Value, const char* FileName)
{
  char* Str = new char[12];
  itoa(Value, Str, 10);
  BOOL Result = WritePrivateProfileString(Section, Key, Str, FileName);
  delete[] Str;
  return Result;
}

#endif
