/*
* RDPSocket.h - An implementation of the reliable datagram protocol as described by Gene Michael Stover
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
#ifndef RDPSocket_H_
#define RDPSocket_H_

#include "tcpclientsocket.h"

enum RDPSocketErrors { ConnectionFailed };

struct RDPClientSocket
{
  unsigned long Address;
  unsigned int Port;
  TCPClientSocket* Socket;
};

class RDPSocket
{
public:
  RDPSocket()
  {
  }

  ~RDPSocket()
  {
    /* Delete all the sockets */
    while (ClientSockets.Size() > 0)
      delete ClientSockets.Remove();
  }

  void Receive(void* &Data, unsigned long &DataLength, char* &Address, int &Port)
  {
    /* Loop over all the sockets */
    RDPClientSocket* ClientSocket = ClientSockets.GetFirst();
    while (ClientSocket != NULL)
    {
      if (ClientSocket->Socket->IsConnected())
      {
        /* Receive the data */
        Data = ClientSocket->Socket->ReceiveBytes(&DataLength);
        in_addr Addr;
        Addr.s_addr = ClientSocket->Address;
        Address = inet_ntoa(Addr);
        Port = ClientSocket->Port;
      }   
      ClientSocket = ClientSockets.GetNext();
    }
  }

  bool ReceiveFrom(void* &Data, unsigned long &DataLength, const char* Address, const int Port)
  {
    /* Get the socket to the specified address and port */
    TCPClientSocket* Socket = GetSocket(StringToAddress(Address), Port);
    if (Socket != NULL && Socket->IsConnected())
    {
      /* Receive the data */
      Data = Socket->ReceiveBytes(&DataLength);
      return Data != NULL;
    }
    return false;
  }

  bool SendTo(const void* Data, const unsigned long DataLength, const char* Address, const int Port)
  {
    /* Get the socket to the specified address and port */
    TCPClientSocket* Socket = GetSocket(StringToAddress(Address), Port);
    if (Socket != NULL && Socket->IsConnected())
    {
      /* Send the data */
      return Socket->SendBytes(Data, DataLength);
    }
    return false;
  }
protected:
  LinkedList<RDPClientSocket> ClientSockets;

  TCPClientSocket* GetSocket(const unsigned long Address, const unsigned int Port)
  {
    /* Find the socket if it exists */
    RDPClientSocket* ClientSocket = ClientSockets.GetFirst();
    while (ClientSocket != NULL)
    {
      if (ClientSocket->Address == Address && ClientSocket->Port == Port)
      {
        if (!ClientSocket->Socket->IsConnected())
          ClientSocket->Socket->Connect(Address, Port);
        return ClientSocket->Socket;
      }   
      ClientSocket = ClientSockets.GetNext();
    }
    /* Create a new socket */
    TCPClientSocket* Socket = new TCPClientSocket;
    if (Socket->Connect(Address, Port))
    {
      ClientSocket = new RDPClientSocket;
      ClientSocket->Address = Address;
      ClientSocket->Port = Port;
      ClientSocket->Socket = Socket;
      ClientSockets.Add(ClientSocket);
      return Socket;
    }
    return NULL;
  }
};

#endif
