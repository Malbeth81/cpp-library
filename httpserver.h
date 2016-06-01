/*
* HTTPServer.h - A class to embed an HTTP server inside an application
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
#ifndef HTTPSERVER_H_
#define HTTPSERVER_H_

#include "cstrutils.h"
#include "strutils.h"
#include "linkedlist.h"
#include "tcpclientsocket.h"
#include "tcpserversocket.h"
#include "winutils.h"

#define BUFFER_SIZE 10240

enum HTTPMethod {HTTP_UNKNOWN, HTTP_HEAD, HTTP_GET, HTTP_POST, HTTP_PUT, HTTP_DELETE, HTTP_TRACE, HTTP_OPTIONS, HTTP_CONNECT};
enum HTTPVersion {HTTP_10, HTTP_11};

class HTTPServer;
struct HTTPConnection
{
  HTTPServer* Server;
  SOCKET SocketId;
};

struct HTTPFile
{
  string Name;
  string FileName;
  void* Content;
};

struct HTTPRequest
{
  HTTPMethod Method;
  HTTPVersion Version;
  string Filename;
  map<string, string> Params;
  map<string, string> Headers;
  map<string, string> Values;
  LinkedList<HTTPFile> Files;
};

struct HTTPResponse
{
  unsigned long Size;
};

struct HTTPBinaryResponse : HTTPResponse
{
  void* Content;
  unsigned long ContentSize;
};

struct HTTPHTMLResponse : HTTPResponse
{
  map<string, string> Headers;
  string Status;
  string Content;
};

typedef HTTPResponse* (__stdcall *REQUESTHANDLERPROC)(HTTPRequest* Request);

class HTTPServer
{
public:
  REQUESTHANDLERPROC RequestHandlerProc;

  HTTPServer()
  {
    hServerThread = NULL;
  }

  ~HTTPServer()
  {
    Close();
  }

  bool Close()
  {
    if (Socket.IsOpened())
    {
      /* Close the server socket */
      return Socket.Close();
    }
    return false;
  }

  bool Open(const int Port)
  {
    if (!Socket.IsOpened())
    {
      /* Create a server socket that listens for connections */
      Socket.Open(Port);

      unsigned long ThreadId;
      /* Start the threads */
      hServerThread = CreateThread(NULL,0,HTTPServer::ServerThread,(void*)this,0,&ThreadId);
      if (hServerThread == NULL)
        return false;
      return true;
    }
    return false;
  }

private:
  static long Delimiter;
  static char LineBreak[3];
  HANDLE hServerThread;
  TCPServerSocket Socket;

  static void ParseMultiPartFormData(HTTPRequest* Request, const char* Content, const unsigned int ContentSize, const char* Boundary, const char* Name = NULL)
  {
    /* Create boundary values */
    char* StartBoundary = new char[strlen(Boundary)+3];
    strcpy(StartBoundary,"--");
    strcat(StartBoundary,Boundary);
    char* EndBoundary = new char[strlen(StartBoundary)+3];
    strcpy(EndBoundary,StartBoundary);
    strcat(EndBoundary,"--");

    /* Find the start boundary */
    char* ptr = strstr(Content,StartBoundary);
    while (ptr != NULL)
    {
      /* Skip the start boundary */
      ptr += strlen(StartBoundary);

      /* Extract the header values */
      map<string,string> Headers;
      char* end = (char*)memstr(ptr,&Delimiter,sizeof(Delimiter),ContentSize);
      if (end == NULL)
        end = ptr+ContentSize;
      extractvaluelist(Headers, ptr, end-ptr, LineBreak, ":");
      /* Skip the delimiter */
      if (strlen(end))
        ptr = end+sizeof(Delimiter);

      /* Find the end boundary */
      end = (char*)memstr(ptr+strlen(StartBoundary),StartBoundary,strlen(StartBoundary),ContentSize-(ptr-Content));

      /* Parse the content disposition */
      const char* String = Headers["Content-Disposition"].c_str();
      if (String != NULL)
      {
        /* Extract the content disposition */
        int Size = strcspn(String,";");
        char* ContentDisposition = new char[Size+1];
        strncpy(ContentDisposition,String,Size);
        ContentDisposition[Size] = '\0';

        /* Extract the name and file name */
        const char* String2 = getvalue(String,"name","=");
        if (String2 != NULL)
          Name = String2;
        if (strncmp(ContentDisposition,"multipart/mixed",15) == 0)
        {
          /* Parse the content */
          const char* Boundary = getvalue(String,"boundary","=");
          ParseMultiPartFormData(Request, ptr, end-ptr, Boundary, Name);
          delete[] Boundary;
        }
        else if (strncmp(ContentDisposition,"form-data",9) == 0)
        {
          /* Extract the content */
          const char* FileName = getvalue(String,"filename","=");
          if (FileName != NULL)
          {
            HTTPFile* File = new HTTPFile;
            File->Name = Name;
            File->FileName = FileName;
            File->Content = new char[end-ptr];
            memcpy(File->Content,ptr,end-ptr);
            Request->Files.Add(File);
          }
          else
            Request->Values[string(Name)] = string(ptr,end-ptr);
        }
        /* Cleanup */
        delete[] String2;
        delete[] ContentDisposition;
      }
      if (end != NULL && strncmp(end, EndBoundary, strlen(EndBoundary)) == 0)
        ptr = NULL;
      else
        ptr = end;
      /* Cleanup */
      delete[] String;
    }
    /* Cleanup */
    delete[] StartBoundary;
    delete[] EndBoundary;
  }

  static void ParseRequestContent(HTTPRequest* Request, const char* Content, const unsigned int ContentSize)
  {
    const char* String = Request->Headers["Content-Type"].c_str();
    /* Extract the content type */
    int Size = strcspn(String,";");
    char* ContentType = new char[Size+1];
    strncpy(ContentType,String,Size);
    ContentType[Size] = '\0';
    /* Parse the content */
    if (strncmp(ContentType,"multipart/form-data",19) == 0)
    {
      /* Extract the content */
      const char* Boundary = getvalue(String,"boundary","=");
      ParseMultiPartFormData(Request,Content,ContentSize,Boundary);
      /* Clean up */
      delete[] Boundary;
    }
    else if (strncmp(ContentType,"application/x-www-form-urlencoded",33) == 0)
      extractvaluelist(Request->Values, Content, ContentSize, "&", "=", NULL, ReplaceFormEncodedCharacters);
    /* Clean up */
    delete[] ContentType;
  }

  static void ParseRequestHeader(HTTPRequest* Request, const char* Header)
  {
    if (Header != NULL)
    {
      /* Parse the header */
      const char* ptr = Header+strspn(Header," ");
      if (strlen(ptr))
      {
        /* Find method */
        if (strncmp(ptr, "HEAD",4) == 0)
          Request->Method = HTTP_HEAD;
        else if (strncmp(ptr, "GET",3) == 0)
          Request->Method = HTTP_GET;
        else if (strncmp(ptr, "POST",4) == 0)
          Request->Method = HTTP_POST;
        else if (strncmp(ptr, "PUT",3) == 0)
          Request->Method = HTTP_PUT;
        else if (strncmp(ptr, "DELETE",6) == 0)
          Request->Method = HTTP_DELETE;
        else if (strncmp(ptr, "TRACE",5) == 0)
          Request->Method = HTTP_TRACE;
        else if (strncmp(ptr, "OPTIONS",7) == 0)
          Request->Method = HTTP_OPTIONS;
        else if (strncmp(ptr, "CONNECT",7) == 0)
          Request->Method = HTTP_CONNECT;
        else
        {
          Request->Method = HTTP_UNKNOWN;
          return;
        }
        const char* endline = strstr(ptr,LineBreak);
        if (endline == NULL)
          endline = ptr+strlen(ptr);
        /* Skip method */
        ptr = ptr+strcspn(ptr," ");
        if (ptr < endline)
        {
          /* Skip whitespace */
          ptr = ptr+strspn(ptr," ");
          /* Extract the file name */
          const char* end = strpbrk(ptr, " ?");
          if (end == NULL || end > endline)
            end = endline;
          Request->Filename.assign(ptr, end-ptr);
          /* Extract the parameters */
          if (strncmp(end, "?", 1) == 0)
          {
            ptr = end;
            end = strstr(ptr," ");
            if (end == NULL || end > endline)
              end = endline;
            extractvaluelist(Request->Params, ptr+1, end-(ptr+1), "&", "=", ReplaceURLEncodedCharacters, ReplaceURLEncodedCharacters);
          }
          /* Skip whitespace */
          ptr = end+strspn(end," ");
          /* Extract HTTP Version */
          if (ptr < endline)
          {
            if (strncmp(ptr, "HTTP/1.0", 8) == 0)
              Request->Version = HTTP_10;
            else
              Request->Version = HTTP_11;
          }
        }
        /* Extract the header values */
        ptr = endline+strlen(LineBreak);
        endline = (char*)memstr(ptr,&Delimiter,sizeof(Delimiter),strlen(ptr));
        if (endline == NULL)
          endline = ptr+strlen(ptr);
        extractvaluelist(Request->Headers, ptr, endline-ptr, LineBreak, ":");
      }
    }
  }

  static string __stdcall ReplaceFormEncodedCharacters(string String)
  {
    for (string::size_type i = 0; i < String.size(); i++)
    {
      if (String.compare(i, 1, "+") == 0)
      {
        /* Replace the character in string */
        String.replace(i, 1, " ");
      }
      else if (String.compare(i, 1, "%") == 0)
      {
        /* Get hexadecimal value in string */
        char* Value = new char[3];
        String.copy(Value,2,i+1);
        Value[2] = '\0';
        /* Replace the character in string */
        Value[0] = (char)strtoul(Value,NULL,16);
        Value[1] = '\0';
        String.replace(i, 3, Value);
        /* Clean up */
        delete[] Value;
      }
    }
    return String;
  }

  static string __stdcall ReplaceURLEncodedCharacters(string String)
  {
    for (string::size_type i = 0; i < String.size(); i++)
    {
      if (String.compare(i, 1, "%") == 0)
      {
        /* Get hexadecimal value in string */
        char* Str = new char[3];
        String.copy(Str,2,i+1);
        Str[2] = '\0';
        unsigned long Value = strtoul(Str,NULL,16);
        if ((Value & 0x80) == 0)
        {
          /* Replace the character in string */
          Str[0] = (char)Value;
          Str[1] = '\0';
          wchar_t* Str2 = utf8towchar(Str);
          char* Str3 = wchartochar(Str2);
          String.replace(i, 3, Str3);
          /* Clean up */
          delete[] Str3;
          delete[] Str2;
        }
        /* Clean up */
        delete[] Str;
      }
    }
    return String;
  }

  static bool SendResponse(TCPClientSocket* Socket, HTTPResponse* Response)
  {
    if (Response->Size == sizeof(HTTPBinaryResponse))
    {
      Socket->SendBytes(((HTTPBinaryResponse*)Response)->Content, ((HTTPBinaryResponse*)Response)->ContentSize);
      return true;
    }
    else if (Response->Size == sizeof(HTTPHTMLResponse))
    {
      HTTPHTMLResponse* Response2 = (HTTPHTMLResponse*)Response;
      char* Data = new char[BUFFER_SIZE+1];
      /* Send response */
      strcpy(Data, "HTTP/1.1 ");
      if (Response2 == NULL)
        strcat(Data, "404 Not Found");
      else if (Response2->Status.length() > 0)
        strcat(Data, Response2->Status.c_str());
      else
        strcat(Data, "200 OK");
      strcat(Data, LineBreak);
      Socket->SendBytes(Data, strlen(Data));
      /* Send header */
      if (Response2 != NULL)
        for (map<string,string>::iterator it = Response2->Headers.begin() ; it != Response2->Headers.end(); it++)
        {
          strcpy(Data, it->first.c_str());
          strcat(Data, ": ");
          strcat(Data, it->second.c_str());
          strcat(Data, LineBreak);
          Socket->SendBytes(Data, strlen(Data));
        }
      /* Send content */
      Socket->SendBytes(LineBreak, strlen(LineBreak));
      if (Response2 != NULL)
        Socket->SendBytes(Response2->Content.c_str(), Response2->Content.length());
      /* Cleanup */
      delete[] Data;
      return true;
    }
    return false;
  }

  static unsigned long __stdcall ConnectionThread(void* arg)
  {
    HTTPConnection* ConnectionInfo = (HTTPConnection*)arg;
    HTTPResponse* Response = NULL;
    TCPClientSocket* Socket = NULL;
    try
    {
      Socket = new TCPClientSocket(ConnectionInfo->SocketId);
      try
      {
        /* Read the request header */
        char* Header = new char[BUFFER_SIZE+1];
        Header[Socket->ReceiveBytes(Header,BUFFER_SIZE,&Delimiter,sizeof(Delimiter))] = '\0';
        /* Parse the request header */
        HTTPRequest* Request = new HTTPRequest();
        ParseRequestHeader(Request, Header);
        unsigned int ContentLength = atoi(Request->Headers["Content-Length"].c_str());
        if (ContentLength > 0)
        {
          /* Read the request content */
          char* Content = new char[ContentLength+1];
          int ContentSize = Socket->ReceiveBytes(Content,ContentLength);
          Content[ContentSize] = '\0';
          /* Parse the request content */
          ParseRequestContent(Request, Content, ContentSize);
          /* Clean up */
          delete[] Content;
        }
        /* Prepare a response */
        if (Request->Method == HTTP_UNKNOWN)
        {
          Response = new HTTPHTMLResponse();
          Response->Size = sizeof(HTTPHTMLResponse);
          ((HTTPHTMLResponse*)Response)->Status = "501 Not Implemented";
        }
        else if (ConnectionInfo->Server->RequestHandlerProc != NULL)
            Response = (*ConnectionInfo->Server->RequestHandlerProc)(Request);
        /* Clean up */
        delete Request;
        delete[] Header;
      }
      catch (...)
      {
        Response = new HTTPHTMLResponse();
        Response->Size = sizeof(HTTPHTMLResponse);
        ((HTTPHTMLResponse*)Response)->Status = "500 Internal Server Error";
      }
      /* Send the response */
      if (Response != NULL)
      {
        SendResponse(Socket,Response);
        /* Clean up */
        if (Response->Size == sizeof(HTTPBinaryResponse))
        {
          free(((HTTPBinaryResponse*)Response)->Content);
          delete (HTTPBinaryResponse*)Response;
        }
        else if (Response->Size == sizeof(HTTPHTMLResponse))
          delete (HTTPHTMLResponse*)Response;
      }
      delete Socket;
      delete ConnectionInfo;
      return 0;
    }
    catch (...)
    {
      delete ConnectionInfo;
      return 1;
    }
  }

  static unsigned long __stdcall ServerThread(void* arg)
  {
    HTTPServer* Server = (HTTPServer*)arg;
    while (Server->Socket.IsOpened())
    {
      /* Accept a client socket's connection request */
      SOCKET SocketId = Server->Socket.Accept();
      if (SocketId != INVALID_SOCKET)
      {
        /* Prepare thread data */
        HTTPConnection* ConnectionInfo = new HTTPConnection;
        ConnectionInfo->Server = Server;
        ConnectionInfo->SocketId = SocketId;
        /* Start the connection thread */
        unsigned long ThreadId;
        HANDLE Thread = CreateThread(NULL,0,HTTPServer::ConnectionThread,(void*)ConnectionInfo,0,&ThreadId);
        if (Thread == NULL)
        {
          delete ConnectionInfo;
          shutdown(SocketId,SD_BOTH);
          closesocket(SocketId);
          return WSAGetLastError();
        }
      }
      else
      {
        int Error = WSAGetLastError();
        if (Error != WSAEWOULDBLOCK && Error != WSAETIMEDOUT)
          return Error;
      }
      /* Wait before trying again */
      Sleep(25);
    }
    return 0;
  }

};
char HTTPServer::LineBreak[3] = {(char)13,(char)10,(char)0};
long HTTPServer::Delimiter = MAKELONG(MAKEWORD(13,10),MAKEWORD(13,10));

#endif
