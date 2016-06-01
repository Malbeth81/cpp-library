/*
* TCPServerSocket.h - An implementation of the TCP protocol's client socket using Windows Sockets 2
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
#ifndef TCPServerSocket_H_
#define TCPServerSocket_H_

#include <winsock2.h>

class TCPServerSocket
{
public:
  TCPServerSocket()
  {
    /* Initialize the Winsock 2 DLL */
    WSADATA info;
    if (WSAStartup(MAKEWORD(2, 0), &info) != 0)
      throw WSANOTINITIALISED;
    if (info.wVersion != MAKEWORD(2,0))
      throw WSAVERNOTSUPPORTED;
    Blocking = true;
    SocketId = INVALID_SOCKET;
    SocketOpened = false;
  }

  ~TCPServerSocket()
  {
    Close();
    WSACleanup();
  }

  SOCKET Accept()
  {
    if (SocketId != INVALID_SOCKET)
      return accept(SocketId,NULL,NULL);
    else
      return INVALID_SOCKET;
  }

  bool Close()
  {
    if (SocketId != INVALID_SOCKET)
    {
      /* Close the current socket */
      shutdown(SocketId,SD_BOTH);
      closesocket(SocketId);
      SocketId = INVALID_SOCKET;
      SocketOpened = false;
      return true;
    }
    return false;
  }

  bool IsBlocking()
  {
    return Blocking;
  }

  bool IsOpened()
  {
    return SocketOpened;
  }

  bool Open(const int Port)
  {
    if (SocketId == INVALID_SOCKET)
    {
      /* Get a new socket descriptor */
      SocketId = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
      if (SocketId != INVALID_SOCKET)
      {
        /* Change the socket mode */
        SetBlocking(Blocking);
        /* Prepare an address descriptor of the host */
        sockaddr_in SocketAddress;
        SocketAddress.sin_family = AF_INET;
        SocketAddress.sin_addr.s_addr = INADDR_ANY;
        SocketAddress.sin_port = htons(Port);

        /* Bind the socket to the port and listen for incoming connections */
        if (bind(SocketId,(SOCKADDR*)&SocketAddress,sizeof(SocketAddress)) != SOCKET_ERROR && listen(SocketId,SOMAXCONN) != SOCKET_ERROR)
        {
          SocketOpened = true;
          return true;
        }
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

protected:
  bool Blocking;
  SOCKET SocketId;
  bool SocketOpened;
};

#endif
