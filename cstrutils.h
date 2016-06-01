/*
* CStrUtils.h - A collection of functions that works with c strings.
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
#ifndef CSTRUTILS_H_
#define CSTRUTILS_H_

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <cwchar>
/*
inline int argcount(const char* Str)
{
  int Result = 0;
  int i = 0;
  while (i != -1 && i < (int)strlen(Str))
  {
    if (Str[i] == '"')
    {
      i = strpos(Str, "\"", i+1);
      if (i != -1)
        i ++;
      Result ++;
    }
    else if (Str[i] == ' ')
      i = strnpos(Str, " ", i);
    else
    {
      i = strpos(Str, " ", i);
      Result ++;
    }
  }
  return Result;
}
*/
/* Returns a string representation of an integer value */
inline char* inttostr(const long Number)
{
  char* Result = new char[12];
  itoa(Number, Result, 10);
  return Result;
}

inline wchar_t* inttowstr(const long Number)
{
  wchar_t* Result = new wchar_t[12];
  _itow(Number, Result, 10);
  return Result;
}


inline char* getline(const char* Str, unsigned int Line = 1)
{
  unsigned int Size = strlen(Str);
  if (Size > 0 && Line > 0)
  {
    const char* LineBegin = Str;
    for (unsigned int i = 0; i < Size-1; i++)
      if (Str[i] == (char)13 || Str[i] == (char)10)
      {
        Line--;
        if (Line == 0)
          Size = i-(LineBegin-Str);
        else
          LineBegin = Str+i+1;
      }
    char* Result = new char[Size+1];
    strncpy(Result,LineBegin,Size);
    Result[Size] = '\0';
    return Result;
  }
  return NULL;
}

inline wchar_t* getline(const wchar_t* Str, unsigned int Line = 1)
{
  unsigned int Size = wcslen(Str);
  if (Size > 0 && Line > 0)
  {
    const wchar_t* LineBegin = Str;
    for (unsigned int i = 0; i < Size-1; i++)
      if (Str[i] == (wchar_t)13 || Str[i] == (wchar_t)10)
      {
        Line--;
        if (Line == 0)
          Size = i-(LineBegin-Str);
        else
          LineBegin = Str+i+1;
      }
    wchar_t* Result = new wchar_t[Size+1];
    wcsncpy(Result,LineBegin,Size);
    Result[Size] = '\0';
    return Result;
  }
  return NULL;
}

/* Get the value of a parameter in a string */
inline char* getvalue(const char* String, const char* Name, const char* Separator)
{
  if (String != NULL && Name != NULL && Separator != NULL)
  {
    while (String != NULL)
    {
      /* Find name in string */
      String = strstr(String,Name);
      if (String != NULL)
      {
        /* Skip the name */
        String += strlen(Name);
        /* Skip whitespace */
        String += strspn(String," ");
        if (strcmp(String,Separator) == 0)
        {
          /* Skip the separator */
          String += strlen(Separator);
          /* Skip whitespace */
          String += strspn(String," ");
          /* Get the value size */
          int Size = 0;
          if (strcmp(String+strlen(Separator),"\"") == 0)
          {
            /* Skip the quote */
            String++;
            Size = strcspn(String,"\"");
          }
          else
            Size = strcspn(String," ");
          /* Extract the value */
          char* Result = new char[Size+1];
          strncpy(Result,String,Size);
          return Result;
        }
      }
    }
  }
  return NULL;
}

/* Return a lowercase version of the string */
inline char* lowerstr(const char* Str)
{
  int Size = strlen(Str);
  char* Result = new char[Size+1];
  for (int i = 0; i < Size; i++)
    Result[i] = (char)tolower(Str[i]);
  Result[Size] = '\0';
  return Result;
}

/* Return a pointer to the first occurence of a giving string in a block of memory */
inline void* memstr(const void* ptr, const void* str, const unsigned int size, const unsigned int num)
{
  if (ptr != NULL && str != NULL && num > 0 && size > 0)
  {
    if (size < num)
    {
      for (unsigned int i = 0; i < num-size; i++)
        if (memcmp((char*)ptr+i,str,size) == 0)
          return (char*)ptr+i;
    }
    else if (size == num && memcmp(ptr,str,size) == 0)
      return (void*)ptr;
  }
  return NULL;
}

/* Return a string representing the specified file size in the appropriate unit */
inline char* sizeformat(const long Size)
{
  char* Result = new char[MAX_PATH];
  char* Str = new char[96];
  Result[0] = '\0';
  if (Size >= 1073741824)
  {
    itoa(Size/1073741824,Str,10);
    strcpy(Result, Str);
    strcat(Result, ".");
    itoa(Size%1073741824,Str,10);
    strncat(Result, Str, 2);
    strcat(Result, " GB (");
  }
  else if (Size >= 1048576)
  {
    itoa(Size/1048576,Str,10);
    strcpy(Result, Str);
    strcat(Result, ".");
    itoa(Size%1048576,Str,10);
    strncat(Result, Str, 2);
    strcat(Result, " MB (");
  }
  else if (Size >= 1024)
  {
    itoa(Size/1024,Str,10);
    strcpy(Result, Str);
    strcat(Result, ".");
    itoa(Size%1024,Str,10);
    strncat(Result, Str, 2);
    strcat(Result, " KB (");
  }
  itoa(Size,Str,10);
  strcat(Result, Str);
  strcat(Result, " bytes");
  if (Size >= 1024)
    strcat(Result, ")");
  return Result;
}

/* Find the position of the first occurrence of one of the specified characters in a string */
inline int strcontains(const char* Str, const char* Str2, const int Index = 0)
{
  int Size = strlen(Str);
  int Size2 = strlen(Str2);
  if (Size > 0 && Size2 > 0)
    for (int i = Index; i < Size; i++)
      for (int j = 0; j < Size2; j++)
        if (strncmp(Str+i, Str2+j, 1) == 0)
          return i;
  return -1;
}

/* Find the position of the first occurrence of the specified string in a string */
inline int strpos(const char* Str, const char* Str2, const int Index = 0)
{
  int Size = strlen(Str);
  int Size2 = strlen(Str2);
  if (Size > 0 && Size2 > 0)
    for (int i = Index; i <= Size-Size2; i++)
      if (strncmp(Str+i, Str2, Size2) == 0)
        return i;
  return -1;
}

/* Find the position of the first occurrence of the specified character in a string */
inline int strpos(const char* Str, const char Char, const int Index = 0)
{
  int Size = strlen(Str);
  if (Size > 0)
    for (int i = Index; i <= Size-1; i++)
      if (strncmp(Str+i, &Char, 1) == 0)
        return i;
  return -1;
}

/* Find the position of the first occurrence of the specified string in a string */
inline int strpos(const wchar_t* Str, const wchar_t* Str2, const int Index = 0)
{
  int Size = wcslen(Str);
  int Size2 = wcslen(Str2);
  if (Size > 0 && Size2 > 0)
    for (int i = Index; i <= Size-Size2; i++)
      if (wcsncmp(Str+i, Str2, Size2) == 0)
        return i;
  return -1;
}

/* Find the position of the first occurrence of the specified character in a string */
inline int strpos(const wchar_t* Str, const wchar_t Char, const int Index = 0)
{
  int Size = wcslen(Str);
  if (Size > 0)
    for (int i = Index; i <= Size-1; i++)
      if (wcsncmp(Str+i, &Char, 1) == 0)
        return i;
  return -1;
}

/* Find the position of the first occurrence of the specified string in a string without case sensitivity */
inline int stripos(const char* Str, const char* Str2, const int Index = 0)
{
  int Size = strlen(Str);
  int Size2 = strlen(Str2);
  if (Size > 0 && Size2 > 0)
    for (int i = Index; i <= Size-Size2; i++)
      if (strnicmp(Str+i, Str2, Size2) == 0)
        return i;
  return -1;
}

/* Find the position of the first occurrence of the specified character in a string without case sensitivity */
inline int stripos(const char* Str, const char Char, const int Index = 0)
{
  int Size = strlen(Str);
  if (Size > 0)
    for (int i = Index; i <= Size-1; i++)
      if (strnicmp(Str+i, &Char, 1) == 0)
        return i;
  return -1;
}

/* Find the position of the first occurrence of the specified string in a string without case sensitivity */
inline int stripos(const wchar_t* Str, const wchar_t* Str2, const int Index = 0)
{
  int Size = wcslen(Str);
  int Size2 = wcslen(Str2);
  if (Size > 0 && Size2 > 0)
    for (int i = Index; i <= Size-Size2; i++)
      if (wcsnicmp(Str+i, Str2, Size2) == 0)
        return i;
  return -1;
}

/* Find the position of the first occurrence of the specified character in a string without case sensitivity */
inline int stripos(const wchar_t* Str, const wchar_t Char, const int Index = 0)
{
  int Size = wcslen(Str);
  if (Size > 0)
    for (int i = Index; i <= Size-1; i++)
      if (wcsnicmp(Str+i, &Char, 1) == 0)
        return i;
  return -1;
}

/* Find the position of the last occurrence of the specified string in a string */
inline int strrpos(const char* Str, const char* Str2, const int Index = 0)
{
  int Size = strlen(Str);
  int Size2 = strlen(Str2);
  if (Size > 0 && Size2 > 0)
    for (int i = Size-Size2-Index; i >= 0; i--)
      if (strncmp(Str+i, Str2, Size2) == 0)
        return i;
  return -1;
}

/* Find the position of the last occurrence of the specified character in a string */
inline int strrpos(const char* Str, const char Char, const int Index = 0)
{
  int Size = strlen(Str);
  if (Size > 0)
    for (int i = Size-1-Index; i >= 0; i--)
      if (strncmp(Str+i, &Char, 1) == 0)
        return i;
  return -1;
}

/* Find the position of the last occurrence of the specified string in a string */
inline int strrpos(const wchar_t* Str, const wchar_t* Str2, const int Index = 0)
{
  int Size = wcslen(Str);
  int Size2 = wcslen(Str2);
  if (Size > 0 && Size2 > 0)
    for (int i = Size-Size2-Index; i >= 0; i--)
      if (wcsncmp(Str+i, Str2, Size2) == 0)
        return i;
  return -1;
}

/* Find the position of the last occurrence of the specified character in a string */
inline int strrpos(const wchar_t* Str, const wchar_t Char, const int Index = 0)
{
  int Size = wcslen(Str);
  if (Size > 0)
    for (int i = Size-1-Index; i >= 0; i--)
      if (wcsncmp(Str+i, &Char, 1) == 0)
        return i;
  return -1;
}

/* Extract a piece of a string */
inline char* substr(const char* Str, const unsigned int First = 0, const unsigned int Last = 0xffffffff)
{
  unsigned int Size = strlen(Str);
  if (Size > 0 && First < Size && Last >= First)
  {
    Size = (Last > Size ? Size : Last)-First;
    char* Result = new char[Size+1];
    strncpy(Result,Str+First,Size);
    Result[Size] = '\0';
    return Result;
  }
  char* Result = new char[1];
  Result[0] = '\0';
  return Result;
}

/* Extract a piece of a string */
inline wchar_t* substr(const wchar_t* Str, const unsigned int First = 0, const unsigned int Last = 0xffffffff)
{
  unsigned int Size = wcslen(Str);
  if (Size > 0 && First < Size && Last >= First)
  {
    Size = (Last > Size ? Size : Last)-First;
    wchar_t* Result = new wchar_t[Size+1];
    wcsncpy(Result,Str+First,Size);
    Result[Size] = '\0';
    return Result;
  }
  wchar_t* Result = new wchar_t[1];
  Result[0] = '\0';
  return Result;
}

/* Return a string containing the specified seconds in time format */
inline char* timeformat(const unsigned int Seconds, const bool LeadingZeros = false)
{
  char* Result = new char[32];
  Result[0] = '\0';
  char* HourStr = inttostr(Seconds/3600);
  char* MinuteStr = inttostr((Seconds%3600)/60);
  char* SecondStr = inttostr((Seconds%3600)%60);
  if (Seconds > 3600)
  {
    if (strlen(HourStr) == 1 && LeadingZeros)
      strcat(Result, "0");
    strcat(Result, HourStr);
    strcat(Result, ":");
  }
  if (strlen(MinuteStr) == 1 && LeadingZeros)
    strcpy(Result, "0");
  strcat(Result, MinuteStr);
  strcat(Result, ":");
  if (strlen(SecondStr) == 1)
    strcat(Result, "0");
  strcat(Result, SecondStr);
  /* Clean up */
  delete[] HourStr;
  delete[] MinuteStr;
  delete[] SecondStr;
  return Result;
}

/* Return a string containing the specified seconds in duration format */
inline char* timeperiodformat(const unsigned int Seconds)
{
  char* Result = new char[256];
  Result[0] = '\0';
  if (Seconds > 86400)
  {
    char* DaysStr = inttostr(Seconds/86400);
    strcat(Result, DaysStr);
    strcat(Result, " days, ");
    delete[] DaysStr;
  }
  if (Seconds > 3600)
  {
    char* HourStr = inttostr((Seconds%86400)/3600);
    strcat(Result, HourStr);
    strcat(Result, " hours, ");
    delete[] HourStr;
  }
  if (Seconds > 60)
  {
    char* MinuteStr = inttostr((Seconds%3600)/60);
    strcat(Result, MinuteStr);
    strcat(Result, " minutes, ");
    delete[] MinuteStr;
  }
  char* SecondStr = inttostr((Seconds%3600)%60);
  strcat(Result, SecondStr);
  strcat(Result, " seconds.");
  delete[] SecondStr;
  return Result;
}

/* Return an upper case version of the string */
inline char* upperstr(const char* Str)
{
  int Size = strlen(Str);
  char* Result = new char[Size+1];
  for (int i = 0; i < Size; i++)
    Result[i] = (char)toupper(Str[i]);
  Result[Size] = '\0';
  return Result;
}

/* Convert a regular character into an unicode one (wide) */
inline wchar_t* chartowchar(const char* Str)
{
  size_t Size = strlen(Str);
  wchar_t* Result = new wchar_t[Size+1];
  Size = mbstowcs(Result,Str,Size);
  if (Size == (size_t)-1)
  {
    delete[] Result;
    return NULL;
  }
  Result[Size] = '\0';
  return Result;
}

/* Convert an unicode character (wide) into a regular character */
inline char* wchartochar(const wchar_t* Str)
{
  size_t Size = wcslen(Str);
  char* Result = new char[Size+1];
  Size = wcstombs(Result,Str,Size);
  if (Size == (size_t)-1)
  {
    delete[] Result;
    return NULL;
  }
  Result[Size] = '\0';
  return Result;
}

/* Convert a utf-8 character into a standard unicode character */
inline wchar_t* utf8towchar(const char *String)
{
  long Size = 0;
  if ((unsigned char) String[0] == 0xEF && (unsigned char) String[1] == 0xBB && (unsigned char) String[2] == 0xBF)
    String += 3;
  for (const unsigned char *ptr = (unsigned char*) String; *ptr > 0; ptr++)
    if (*ptr < 0x80 || (*ptr & 0xC0) == 0xC0)
      Size++;
  wchar_t *Result = new wchar_t[Size+1];
  long Index = 0;
  for (unsigned char* ptr = (unsigned char*) String; *ptr; ptr++)
  {
    if ((*ptr & 0x80) == 0)
      //Byte represents an ASCII character.
      Result[Index] = *ptr;
    else if ((*ptr & 0xC0) == 0x80)
      //Byte is the middle of an encoded character. Ignore.
      continue;
    else if ((*ptr & 0xE0) == 0xC0)
      //Byte represents the start of an encoded character in the range U+0080 to U+07FF
      Result[Index] = ((*ptr & 0x1F) << 6) | (ptr[1] & 0x3F);
    else if ((*ptr & 0xF0) == 0xE0)
      //Byte represents the start of an encoded character in the range U+07FF to U+FFFF
      Result[Index] = ((*ptr & 0x0F) << 12) | ((ptr[1] & 0x3F) << 6) | (ptr[2] & 0x3F);
    else if ((*ptr & 0xF8) == 0xF0)
      //Byte represents the start of an encoded character beyond the U+FFFF limit of 16-bit integers
      Result[Index] = '?';
    Index++;
  }
  Result[Size] = '\0';
  return Result;
}
#endif
