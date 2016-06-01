/*
* AsyncSocket.h - Functions to create asynchronous socket connections.
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
#ifndef ASYNCSOCKET_H_
#define ASYNCSOCKET_H_

#include <string>
#include <windows.h>
#include <winsock2.h>

using namespace std;

#define WM_WSAASYNC (WM_USER +5)

/* Initiate a link with the Winsock 2 DLL before using those functions */
//  WSADATA info;
//  if (WSAStartup(MAKEWORD(2,2),&info) != 0)
//    throw "Could not initiate Winsock 2 DLL (Ws2_32.dll)";
//  if (w.wVersion != MAKEWORD(2,2))
//    throw "Requires version 2.2 of Winsock DLL (Ws2_32.dll)";
/* Once done close the socket and terminate the link */
//  closesocket(Socket);
//  WSACleanup();

/* After creating the socket 
 * The specified window hWnd must listen for the WM_WSAASYNC message, here is an example :
 * 
    case WM_WSAASYNC:
    {
      // Check for a network error 
      if (WSAGETSELECTERROR(lParam) != 0)
      {
        CloseSocket(wParam);
        return 0;
      }
      switch (WSAGETSELECTEVENT(lParam))
      {
        case FD_ACCEPT:     // server only
        {
          // Accept the client's connection request 
          Accept(wParam,hWindow);
          AppendMessage("Client connected. Waiting for data...\n");
          break;
        }
        case FD_CONNECT:    // client only
        {
          AppendMessage("Connection established. Sending data...\n");
          //for (int i = 0; i < 10; i++)
            SendLine(wParam,"ping\n");
          if (shutdown(wParam,SD_SEND) == SOCKET_ERROR) 
          {
            CloseSocket(wParam);
            break;
          }
          AppendMessage("Closing connection...\n");
          break;
        }
        case FD_CLOSE:      
        {
          CloseSocket(wParam);
          AppendMessage("Connection closed.\n");
          CanClose = true;
          break;
        }
        case FD_READ:      
        {
          string Data = ReceiveLine(wParam);
          if (Data.length() > 0)
          {
            AppendMessage("Receiving data: ");
            AppendMessage(Data.c_str());
          }
          if (shutdown(wParam,SD_SEND) == SOCKET_ERROR) 
          {
            CloseSocket(wParam);
            break;
          }
          AppendMessage("Closing connection...\n");
          break;
        }
        case FD_WRITE:      
        {
          // Sent when new space is availlable on the sending buffer after it was full.
          break;
        }
      }
      return 0;
    }
 */

inline SOCKET OpenAsyncServerSocket(const int Port, const HWND hWnd, const int QueueSize)
{
  /* Create a new Windows socket */
  SOCKET Socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  if (Socket == INVALID_SOCKET)
    return INVALID_SOCKET;
  
  /* Make the socket asynchronous */
  WSAAsyncSelect(Socket,hWnd,WM_WSAASYNC,FD_ACCEPT|FD_CLOSE);

  hostent* thisHost = gethostbyname("");
  char* ip = inet_ntoa (*(struct in_addr *)*thisHost->h_addr_list);
  
  /* Obtain the host socket internet address */
  sockaddr_in SocketAddress;
  memset(&SocketAddress,0,sizeof(sockaddr_in));
  SocketAddress.sin_family = AF_INET;
  SocketAddress.sin_addr.s_addr = inet_addr(ip);
  SocketAddress.sin_port = htons(Port);

  /* Bind the socket to the internet address */
  if (bind(Socket,(SOCKADDR*)&SocketAddress,sizeof(SocketAddress)) == SOCKET_ERROR)
  {
    CloseSocket(Socket);
    return INVALID_SOCKET;
  }

  /* Make the socket listen for incomming connections */
  if (listen(Socket,QueueSize) == SOCKET_ERROR)
  {
    CloseSocket(Socket);
    return INVALID_SOCKET;
  }
  return Socket;
}

inline SOCKET OpenAsyncClientSocket(const string& HostAddress, const int Port, const HWND hWnd)
{
  /* Initiate a ling with the Winsock 2 DLL */
  if (SocketCount == 0)
  {
    WSADATA info;
    if (WSAStartup(MAKEWORD(2,2),&info) != 0)
      throw "Could not initiate Winsock 2 DLL (Ws2_32.dll)";
  }
  SocketCount++;
  
  /* Create a new Windows socket */
  SOCKET Socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  if (Socket == INVALID_SOCKET)
    return INVALID_SOCKET;
  
  /* Make the socket asynchronous */
  WSAAsyncSelect(Socket,hWnd,WM_WSAASYNC,FD_READ|FD_WRITE|FD_CONNECT|FD_CLOSE);

  /* Obtain information on the host */
  hostent *Host;
  if ((Host = gethostbyname(HostAddress.c_str())) == 0)
  {
    CloseSocket(Socket);
    return INVALID_SOCKET;
  }
  
  /* Fill in the host socket address information */
  sockaddr_in SocketAddress;
  SocketAddress.sin_family = AF_INET;
  SocketAddress.sin_port = htons(Port);
  SocketAddress.sin_addr = *((in_addr *)Host->h_addr);
  memset(&(SocketAddress.sin_zero), 0, 8);
  
  /* Connect to the host. */
  if (connect(Socket, (sockaddr*)&SocketAddress, sizeof(sockaddr)) == SOCKET_ERROR)
  {
    int Error = WSAGetLastError();
    if (Error != WSAEWOULDBLOCK)
    {
      CloseSocket(Socket);
      return INVALID_SOCKET;
    }
    // On an asynchronous socket, connect always returns WSAEWOULDBLOCK.
    // Connection success is indicated by the FD_CONNECT message only.
  }
  return Socket;
}

inline SOCKET Accept(const SOCKET Socket, const HWND hWnd)
{
  /* Accept the connection request and create a new socket to handle it */
  SOCKET ClientSocket = accept(Socket,NULL,NULL);
  if (ClientSocket == INVALID_SOCKET)
    // TODO Handle various error messages here, may need to close Socket for some...
    return INVALID_SOCKET;
  /* Make the socket asynchronous (specifies the desired messages) */
  WSAAsyncSelect(ClientSocket,hWnd,WM_WSAASYNC,FD_READ|FD_WRITE);
  return ClientSocket;
}

#endif
