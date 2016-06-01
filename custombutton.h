/*
* CustomButton.h - Functions to create and display a custom drawn Windows button.
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
#ifndef CustomButton_H_
#define CustomButton_H_

#if (defined _WIN32_WINNT && _WIN32_WINNT < 0x0501)
  #undef _WIN32_WINNT
#endif
#if !defined _WIN32_WINNT
  #define _WIN32_WINNT 0x0501
#endif
#include <windows.h>
#include <tmschema.h>
#include <uxtheme.h>

typedef HRESULT(__stdcall *PFNCLOSETHEMEDATA)(HTHEME);
typedef HRESULT(__stdcall *PFNDRAWTHEMEBACKGROUND)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect);
typedef HRESULT(__stdcall *PFNDRAWTHEMEPARENTBACKGROUND)(HWND hWnd, HDC DC, RECT* R);
typedef HTHEME(__stdcall *PFNOPENTHEMEDATA)(HWND hwnd, LPCWSTR pszClassList);
typedef HRESULT(__stdcall *PFNGETTHEMEBACKGROUNDCONTENTRECT)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pBoundingRect, RECT *pContentRect);

static HANDLE hTheme = NULL;
static HMODULE hThemesModule = LoadLibrary("UXTHEME.DLL");
static PFNOPENTHEMEDATA zOpenThemeData = (PFNOPENTHEMEDATA)GetProcAddress(hThemesModule, "OpenThemeData");
static PFNDRAWTHEMEBACKGROUND zDrawThemeBackground = (PFNDRAWTHEMEBACKGROUND)GetProcAddress(hThemesModule, "DrawThemeBackground");
static PFNDRAWTHEMEPARENTBACKGROUND zDrawThemeParentBackground = (PFNDRAWTHEMEPARENTBACKGROUND)GetProcAddress(hThemesModule, "DrawThemeParentBackground");
static PFNCLOSETHEMEDATA zCloseThemeData = (PFNCLOSETHEMEDATA)GetProcAddress(hThemesModule, "CloseThemeData");
static PFNGETTHEMEBACKGROUNDCONTENTRECT zGetThemeBackgroundContentRect = (PFNGETTHEMEBACKGROUNDCONTENTRECT)GetProcAddress(hThemesModule, "GetThemeBackgroundContentRect");

// PUBLIC FUNCTIONS ------------------------------------------------------------

inline void ApplyThemeToCustomButton(HWND hWnd)
{
  if (hThemesModule != NULL)
  {
    if (hTheme != NULL)
      zCloseThemeData(hTheme);
    hTheme = zOpenThemeData(hWnd, L"Button");
//    FreeLibrary(hThemesModule);
//    hThemesModule = NULL;
  }
}

inline void DrawCustomButton(HWND hWnd, LPDRAWITEMSTRUCT DrawItem)
{
  /* Draw the button */
  SetBkMode(DrawItem->hDC, TRANSPARENT);
  if (hTheme != NULL)
  {
    zDrawThemeParentBackground(GetParent(hWnd), DrawItem->hDC, &DrawItem->rcItem);
    zDrawThemeBackground(hTheme, DrawItem->hDC, BP_PUSHBUTTON, (DrawItem->itemState & ODS_SELECTED ? PBS_PRESSED : (DrawItem->itemState & ODS_DISABLED ? PBS_DISABLED : (DrawItem->itemState & ODS_FOCUS ? PBS_DEFAULTED : PBS_NORMAL))), &DrawItem->rcItem, NULL);
  }
  else
    DrawFrameControl(DrawItem->hDC,&DrawItem->rcItem,DFC_BUTTON,DFCS_BUTTONPUSH|(DrawItem->itemState & ODS_SELECTED ? DFCS_PUSHED : 0)|(DrawItem->itemState & ODS_DISABLED ? DFCS_INACTIVE : 0));
  InflateRect(&DrawItem->rcItem, -3, -3);
  /* Draw the focus rectangle */
  if (DrawItem->itemState & ODS_FOCUS)
    DrawFocusRect(DrawItem->hDC, &DrawItem->rcItem);
}

#endif
