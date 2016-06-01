/*
* StrUtils.h - A collection of functions that works with c++ strings.
*
* v 1.0
*
* Copyright (C) 2009-2010 Marc-André Lamothe.
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
#ifndef STRUTILS_H_
#define STRUTILS_H_

#include <map>
#include <string>
#include <windows.h>

using namespace std;

typedef string (__stdcall *VALUEMODIFIERPROC)(string Value);

inline void extractvaluelist(map<string,string>& Result, const char* String, const unsigned int Size, const char* Delimiter, const char* Separator, const VALUEMODIFIERPROC keyModifier = NULL, const VALUEMODIFIERPROC valueModifier = NULL)
{
  if (String != NULL && Size > 0 && Delimiter != NULL && Separator != NULL)
  {
    const char* end = String+Size;
    while (String != NULL && String < end)
    {
      /* Skip whitespace */
      String += strspn(String," ");
      /* Find separator and delimiter */
      const char* ptr1 = strstr(String,Separator);
      const char* ptr3 = strstr(String,Delimiter);
      if (ptr3 == NULL)
        ptr3 = end;
      /* Extract value */
      if (ptr1 != NULL && ptr3 != NULL)
      {
        /* Skip whitespace */
        const char* ptr2 = ptr1+strlen(Separator);
        ptr2 += strspn(ptr2," ");
        Result[(keyModifier != NULL ? (*keyModifier)(string(String,ptr1-String)) : string(String,ptr1-String))] = (valueModifier != NULL ? (*valueModifier)(string(ptr2,ptr3-ptr2)) : string(ptr2,ptr3-ptr2));
      }
      String = ptr3;
      if (String != NULL)
        String += strlen(Delimiter);
    }
  }
}

inline void findreplace(string& String, const char* Value, const char* Replacement)
{
  if (Value != NULL && Replacement != NULL && strcmp(Value,Replacement) != 0)
  {
    int Size = strlen(Value);
    if (Size > 0)
    {
      string::size_type pos = String.find(Value, 0);
      while (pos != string::npos)
      {
        String.replace(pos, Size, Replacement);
        pos = String.find(Value, pos);
      }
    }
  }
}

inline void findreplace(string& String, const string& Value, const string& Replacement)
{
  if (Value.compare(Replacement) != 0)
  {
    int Size = Value.length();
    if (Size > 0)
    {
      string::size_type pos = String.find(Value, 0);
      while (pos != string::npos)
      {
        String.replace(pos, Size, Replacement);
        pos = String.find(Value, pos);
      }
    }
  }
}

inline void findreplace(string& String, map<string,string> Values)
{
  if (Values.size() > 0)
  {
    for (string::size_type i = 0; i < String.size(); i++)
    {
      for (map<string,string>::iterator it = Values.begin(); it != Values.end(); it++)
      {
        if (String.compare(i, (*it).first.size(), (*it).first))
        {
          String.replace(i, (*it).first.size(), (*it).second);
          break;
        }
      }
    }
  }
}

inline string GetPrivateProfileString(const string Section, const string Key, const string Default, const string FileName)
{
  string Result;
  char* Data = new char[MAX_PATH];
  GetPrivateProfileString(Section.c_str(), Key.c_str(), Default.c_str(), Data, MAX_PATH, FileName.c_str());
  Result.assign(Data);
  delete[] Data;
  return Result;
}

inline void stringtolowercase(string& String)
{
  for (string::iterator i = String.begin(); i != String.end(); ++i)
    *i = tolower(*i);
}

#endif
