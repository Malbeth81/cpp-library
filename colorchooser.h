/*
* ColorChooser.h - A Windows control that displays a colour selection button.
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
#ifndef COLORCHOOSER_H_
#define COLORCHOOSER_H_

#include "custombutton.h"
#include <windows.h>

// PUBLIC FUNCTIONS ------------------------------------------------------------

inline void ApplyThemeToColorChooser(HWND hWnd)
{
  ApplyThemeToCustomButton(hWnd);
}

inline void SetColorChooserColor(HWND hWnd, COLORREF Color)
{
  char* Str = new char[9];
  itoa(Color,Str,10);
  SetWindowText(hWnd, Str);
  delete[] Str;
  InvalidateRect(hWnd, NULL, FALSE);
}

inline COLORREF GetColorChooserColor(HWND hWnd)
{
  int Len = GetWindowTextLength(hWnd);
  char* Text = new char[Len+1];
  GetWindowText(hWnd, Text, Len+1);
  COLORREF Color = (COLORREF)atoi(Text);
  delete[] Text;
  return Color;
}

inline void DrawColorChooser(HWND hWnd, LPDRAWITEMSTRUCT DrawItem)
{
  /* Draw the button */
  DrawCustomButton(hWnd, DrawItem);
  /* Draw the button's content */
  SetBkMode(DrawItem->hDC, TRANSPARENT);
  InflateRect(&DrawItem->rcItem, -2, -2);
  if (DrawItem->itemState & ODS_DISABLED)
  {
    HPEN Pen = (HPEN)SelectObject(DrawItem->hDC, CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW)));
    Rectangle(DrawItem->hDC, DrawItem->rcItem.left, DrawItem->rcItem.top, DrawItem->rcItem.right, DrawItem->rcItem.bottom);
    DeleteObject(SelectObject(DrawItem->hDC, Pen));
  }
  else
  {
    HBRUSH Brush = (HBRUSH)SelectObject(DrawItem->hDC, CreateSolidBrush(GetColorChooserColor(hWnd)));
    Rectangle(DrawItem->hDC, DrawItem->rcItem.left, DrawItem->rcItem.top, DrawItem->rcItem.right, DrawItem->rcItem.bottom);
    DeleteObject(SelectObject(DrawItem->hDC, Brush));
  }
}

inline void ClickColorChooser(HWND hWnd)
{
  static COLORREF Colors[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  CHOOSECOLOR Dlg;
  Dlg.lStructSize = sizeof(CHOOSECOLOR);
  Dlg.hwndOwner = GetParent(hWnd);
  Dlg.rgbResult = GetColorChooserColor(hWnd);
  Dlg.lpCustColors = &Colors[0];
  Dlg.Flags = CC_FULLOPEN | CC_RGBINIT;
  Dlg.lCustData = 0;
  if (ChooseColor(&Dlg) != 0)
    SetColorChooserColor(hWnd, Dlg.rgbResult);
}

#endif
