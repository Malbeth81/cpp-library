/*
* TCPClientSocket.h - An implementation of the TCP protocol's client socket using Windows Sockets 2
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
#ifndef TCPClientSocket_H_
#define TCPClientSocket_H_

#include <winsock2.h>
#if (defined DEBUG && DEBUG == 0)
  #include <iostream>
#endif

#define SO_CONNECT_TIME 0x700C
        
class TCPClientSocket
{
public:
  TCPClientSocket(SOCKET socket = INVALID_SOCKET, bool blocking = true)
  {
    /* Initialize the Winsock 2 DLL */
    WSADATA info;
    if (WSAStartup(MAKEWORD(2,0),&info) != 0)
      throw WSANOTINITIALISED;
    if (info.wVersion != MAKEWORD(2,0))
      throw WSAVERNOTSUPPORTED;
    SocketId = socket;
    SetBlocking(blocking);

  }

  ~TCPClientSocket()
  {
    Close();
    WSACleanup();
  }

  bool Close()
  {
    if (SocketId != INVALID_SOCKET)
    {
      /* Close the current socket */
      shutdown(SocketId,SD_BOTH);
      closesocket(SocketId);
      SocketId = INVALID_SOCKET;
      return true;
    }
    return false;
  }

  bool Connect(const unsigned long Address, const int Port)
  {
    if (SocketId == INVALID_SOCKET && Address != INADDR_NONE && Address != INADDR_ANY)
    {
      /* Get a new socket descriptor */
      SocketId = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
      if (SocketId != INVALID_SOCKET)
      {
        /* Change the socket mode */
        SetBlocking(Blocking);
        /* Prepare an address descriptor to the host */
        sockaddr_in SocketAddress;
        SocketAddress.sin_family = AF_INET;
        SocketAddress.sin_addr.s_addr = Address;
        SocketAddress.sin_port = htons(Port);
        /* Connect to the host */
        if (connect(SocketId,(SOCKADDR*)&SocketAddress,sizeof(SocketAddress)) != SOCKET_ERROR)
        {
#if (defined DEBUG && DEBUG == 0)
  std::cout << "Connected to " << Address << ":" << Port << std::endl;
#endif
          return true;
        }
        Close();
      }
    }
    return false;
  }

  bool Connect(const char* Address, const int Port)
  {
    return Connect(StringToAddress(Address), Port);
  }

  bool IsBlocking()
  {
    return Blocking;
  }

  bool IsConnected()
  {
    if (SocketId != INVALID_SOCKET)
    {
      long ConnectionTime;
      int Size = sizeof(ConnectionTime);
      if (getsockopt(SocketId,SOL_SOCKET,SO_CONNECT_TIME,(char*)&ConnectionTime,&Size) == 0)
        return (ConnectionTime != SOCKET_ERROR);
    }
    return false;
  }

  short ReceiveByte()
  {
    char Buffer;
    if (ReceiveBytes(&Buffer,sizeof(char)) == sizeof(char))
    {
#if (defined DEBUG && DEBUG == 0)
  std::cout << "Received byte : " << (short)Buffer << std::endl;
#endif
      return (short)Buffer;
    }
    return 0;
  }

  unsigned long ReceiveBytes(const void* Data, const unsigned long DataSize)
  {
    unsigned long ReceivedSize = 0;
    while (ReceivedSize < DataSize)
    {
      int Result = recv(SocketId,(char*)Data+ReceivedSize,DataSize-ReceivedSize,0);
      if (Result == 0 || Result == SOCKET_ERROR)
      {
        if (WSAGetLastError() != WSAEWOULDBLOCK)
          Close();
        break;
      }
      ReceivedSize += Result;
    }
    return ReceivedSize;
  }

  unsigned long ReceiveBytes(const void* Buffer, const unsigned long BufferSize, const void* Delimiter, const unsigned long DelimiterSize)
  {
    unsigned long ReceivedSize = 0;
    while (ReceivedSize < BufferSize && (ReceivedSize < DelimiterSize || memcmp((char*)Buffer+ReceivedSize-DelimiterSize,Delimiter,DelimiterSize) != 0))
    {
      int Result = recv(SocketId,(char*)Buffer+ReceivedSize,1,0);
      if (Result == 0 || Result == SOCKET_ERROR)
      {
        if (WSAGetLastError() != WSAEWOULDBLOCK)
          Close();
        break;
      }
      ReceivedSize += Result;
    }
    return ReceivedSize;
  }

  long ReceiveInteger()
  {
    long Buffer;
    if (ReceiveBytes(&Buffer,sizeof(long)) == sizeof(long))
    {
#if (defined DEBUG && DEBUG == 0)
  std::cout << "Received integer : " << ntohl(Buffer) << std::endl;
#endif
      return ntohl(Buffer);
    }
    return 0;
  }

  short ReceiveShort()
  {
    short Buffer;
    if (ReceiveBytes(&Buffer,sizeof(short)) == sizeof(short))
    {
#if (defined DEBUG && DEBUG == 0)
	std::cout << "Received short : " << ntohs(Buffer) << std::endl;
#endif
      return ntohs(Buffer);
    }
    return 0;
  }

  char* ReceiveString()
  {
    unsigned long DataSize = ReceiveInteger();
    if (DataSize > 0)
    {
      char* Data = new char[DataSize];
      memset(Data,sizeof(Data),0);
      if (ReceiveBytes(Data, DataSize) == DataSize)
      {
#if (defined DEBUG && DEBUG == 0)
  std::cout << "Received string : " << Data << std::endl;
#endif
        return Data;
      }
      delete[] Data;
    }
    return NULL;
  }

  WCHAR* ReceiveUTF8String()
  {
    unsigned long DataSize = ReceiveInteger();
    if (DataSize > 0)
    {
      char* Data = new char[DataSize+1];
      if (ReceiveBytes(Data, DataSize) == DataSize)
      {
        Data[DataSize] = '\0';
#if (defined DEBUG && DEBUG == 0)
	std::cout << "Received string : " << Data << std::endl;
#endif
        unsigned long Length = MultiByteToWideChar(CP_UTF8,0,Data,DataSize,NULL,0);
        if (Length > 0)
        {
          WCHAR* Result = new WCHAR[Length+1];
          if (MultiByteToWideChar(CP_UTF8,0,Data,DataSize,Result,Length) > 0)
          {
            Result[Length] = L'\0';
            return Result;
          }
        }
      }
    }
    return NULL;
  }

  bool SendByte(const char Data)
  {
    if (SendBytes((char*)&Data, sizeof(char)) == sizeof(char))
    {
#if (defined DEBUG && DEBUG == 0)
  std::cout << "Sent byte : " << Data << std::endl;
#endif
      return true;
    }
    return false;
  }

  unsigned long SendBytes(const void* Data, const unsigned long DataSize)
  {
    unsigned long SentSize = 0;
    while (SentSize < DataSize)
    {
      int Result = send(SocketId,(char*)Data+SentSize,DataSize-SentSize,0);
      if (Result == SOCKET_ERROR)
      {
        if (WSAGetLastError() != WSAEWOULDBLOCK)
          Close();
        break;
      }
      SentSize += Result;
    }
    return SentSize;
  }

  bool SendInteger(const long Data)
  {
    long Buffer = htonl(Data);
    if (SendBytes((char*)&Buffer, sizeof(long)) == sizeof(long))
    {
#if (defined DEBUG && DEBUG == 0)
  std::cout << "Sent integer : " << Data << std::endl;
#endif
      return true;
    }
    return false;
  }

  bool SendShort(const short Data)
  {
    short Buffer = htons(Data);
    if (SendBytes((char*)&Buffer, sizeof(short)) == sizeof(short))
    {
#if (defined DEBUG && DEBUG == 0)
	std::cout << "Sent short : " << Data << std::endl;
#endif
      return true;
    }
    return false;
  }

  bool SendString(const char* Data)
  {
    unsigned long DataSize = strlen(Data)+1;
    if (SendInteger(DataSize) && SendBytes(Data, DataSize) == DataSize)
    {
#if (defined DEBUG && DEBUG == 0)
  std::cout << "Sent string : " << Data << std::endl;
#endif
      return true;
    }
    return false;
  }

  bool SendUTF8String(const WCHAR* Data)
  {
    unsigned long Length = WideCharToMultiByte(CP_UTF8,0,Data,-1,NULL,0,NULL,NULL);
    if (Length > 0)
    {
      char* Buffer = new char[Length];
      if (WideCharToMultiByte(CP_UTF8,0,Data,-1,Buffer,Length,NULL,NULL) > 0 && SendInteger(Length-1) && SendBytes(Buffer, Length-1) == Length-1)
      {
#if (defined DEBUG && DEBUG == 0)
	std::cout << "Sent string : " << Buffer << std::endl;
#endif
        return true;
      }
    }
    return false;
  }

  void SetBlocking(const bool Value)
  {
    Blocking = Value;
    if (SocketId != INVALID_SOCKET)
    {
      u_long Arg = (Blocking ? 0 : 1);
      ioctlsocket(SocketId,FIONBIO,&Arg);
    }
  }

  static unsigned long StringToAddress(const char* Address)
  {
    hostent* Host = gethostbyname(Address);
    if (Host != NULL)
      return *(u_long*)Host->h_addr_list[0];
    else
      return inet_addr(Address);
  }

protected:
  bool Blocking;
  SOCKET SocketId;
};

#endif
