/*
* UDPSocket.h - Functions to create UDP socket connections.
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
#ifndef UDPSocket_H_
#define UDPSocket_H_

#include <winsock2.h>

static int UDPSocketCount = 0;

inline bool CloseUDPSocket(const SOCKET SocketId)
{
  if (SocketId == INVALID_SOCKET)
  {
    UDPSocketCount--;
    /* Terminate link to the Winsock 2 DLL */
    if (UDPSocketCount == 0)
      WSACleanup();
    /* Close the current socket */
    closesocket(SocketId);
    return true;
  }
  return false;
}

inline SOCKET CreateUDPSocket(const int Port, const int Timeout = 1000)
{
  /* Initialize link to the Winsock 2 DLL */
  if (UDPSocketCount == 0)
  {
    WSADATA info;
    if (WSAStartup(MAKEWORD(2,0),&info) != 0 || info.wVersion != MAKEWORD(2,0))
      return INVALID_SOCKET;
  }
  /* Create a new Windows socket */
  SOCKET SocketId = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
  UDPSocketCount++;
  /* Change the socket's timeout values */
  setsockopt(SocketId, SOL_SOCKET, SO_RCVTIMEO, (char*)&Timeout, sizeof(Timeout));
  setsockopt(SocketId, SOL_SOCKET, SO_SNDTIMEO, (char*)&Timeout, sizeof(Timeout));
  /* Bind the socket to any address on the port */
  sockaddr_in Addr;
  Addr.sin_family = AF_INET;
  Addr.sin_port = htons(Port);
  Addr.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(SocketId, (SOCKADDR*)&Addr, sizeof(Addr));
  return SocketId;
}

inline SOCKADDR_IN* CreateUDPSocketAddress(const char* Address, const int Port)
{
  /* Find the host by name or address */
  unsigned long Addr = 0;
  if (Address != NULL && strlen(Address) > 0)
  {
    HOSTENT* Host = gethostbyname(Address);
    if (Host != NULL)
      Addr = *(u_long*)Host->h_addr_list[0];
    else
      Addr = inet_addr(Address);
  }
  else
    Addr = htonl(INADDR_ANY);
  
  /* Return an address descriptor */
  SOCKADDR_IN* SocketAddress = new SOCKADDR_IN;
  SocketAddress->sin_family = AF_INET;
  SocketAddress->sin_port = htons(Port);
  SocketAddress->sin_addr.s_addr = Addr;
  return SocketAddress;
}

inline unsigned int MaxUDPPacketSize(const SOCKET SocketId)
{
  /* Get the maximum packet size for UDP (usually 65506 bytes) */
  unsigned int MaxSize;
  int Size = sizeof(MaxSize);
  if (getsockopt(SocketId, SOL_SOCKET, SO_MAX_MSG_SIZE, (char*)&MaxSize, &Size) != SOCKET_ERROR)
    return MaxSize-1;
  return 0;
}

inline char* ReceiveString(const SOCKET SocketId, SOCKADDR_IN* SocketAddress)
{
  /* Receive a string from the socket */
  if (SocketId != INVALID_SOCKET)
  {
    int AddressSize = sizeof(*SocketAddress);
    int BufferSize = MaxUDPPacketSize(SocketId);
    if (BufferSize > 0)
    {
      /* Receive data */
      char Buffer[BufferSize];
      int DataSize = recvfrom(SocketId, Buffer, BufferSize, 0, (SOCKADDR*)SocketAddress, &AddressSize);
      if (DataSize > 0 && DataSize != SOCKET_ERROR)
      {
        char* Result = new char[DataSize+1];
        strncpy(Result,Buffer,DataSize);
        Result[DataSize] = '\0';
        return Result;
      }
    }
  }
  return NULL;
}

inline bool SendString(const SOCKET SocketId, SOCKADDR_IN* SocketAddress, const char* String)
{
  /* Send the string trough the socket */
  if (SocketId != INVALID_SOCKET && String != NULL && strlen(String) <= MaxUDPPacketSize(SocketId) && sendto(SocketId, String, strlen(String), 0, (SOCKADDR*)SocketAddress, sizeof(*SocketAddress)) != SOCKET_ERROR)
    return true;
  return false;
}

inline bool SetNonBlocking(const SOCKET SocketId, const bool Value)
{
  /* Change the socket type */
  u_long Arg = (Value ? 1 : 0);
  return (ioctlsocket(SocketId,FIONBIO,&Arg) == 0);
}

#endif
