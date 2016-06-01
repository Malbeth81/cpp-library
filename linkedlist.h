/*
* LinkedList.h - An implementation of a linked list of objects.
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
#ifndef LinkedList_H_
#define LinkedList_H_

#ifndef NULL
  #define NULL 0
#endif

template <typename DataType>
struct ListNode
{
  DataType* Data;
  ListNode* Prev;
  ListNode* Next;
};

template <typename DataType>
class LinkedList
{
public:
  typedef int (__stdcall *COMPARATOR)(DataType* A, DataType* B); /* if (B < A) return -1; else if (B > A) return 1; else return 0; */

  LinkedList()
  {
    Current = NULL;
    First = NULL;
    Last = NULL;
    NodeCount = 0;
  }

  ~LinkedList()
  {
    ListNode<DataType>* Ptr;
    while (First != NULL)
    {
      Ptr = First;
      First = First->Next;
      delete Ptr;
    }
  }

  void Add(DataType* Object)
  {
    if (Object != NULL)
    {
      /* Create a new node */
      ListNode<DataType>* Ptr = new ListNode<DataType>;
      Ptr->Data = Object;
      Ptr->Prev = Last;
      Ptr->Next = NULL;
      /* Add the node to the list */
      if (Last != NULL)
        Last->Next = Ptr;
      else
        First = Ptr;
      Last = Ptr;
      NodeCount++;
    }
  }

  DataType* Get(unsigned int Index) const
  {
    /* Get the node */
    ListNode<DataType>* Ptr = GetNode(Index);
    /* Return the object */
    if (Ptr != NULL)
      return Ptr->Data;
    return NULL;
  }

  DataType* GetFirst() const
  {
    Current = First;
    if (Current != NULL)
      return Current->Data;
    return NULL;
  }

  DataType* GetLast() const
  {
    Current = Last;
    if (Current != NULL)
      return Current->Data;
    return NULL;
  }

  DataType* GetNext() const
  {
    if (Current != NULL)
      Current = Current->Next;
    if (Current != NULL)
      return Current->Data;
    return NULL;
  }

  DataType* GetPrevious() const
  {
    if (Current != NULL)
      Current = Current->Prev;
    if (Current != NULL)
      return Current->Data;
    return NULL;
  }

  DataType* Remove(unsigned int Index = 0)
  {
    DataType* Object = NULL;
    /* Get the node */
    ListNode<DataType>* Ptr = GetNode(Index);
    if (Ptr != NULL)
    {
      Object = Ptr->Data;
      /* Remove the node from the list */
      if (Ptr->Prev == NULL)
        First = Ptr->Next;
      else
        Ptr->Prev->Next = Ptr->Next;
      if (Ptr->Next == NULL)
        Last = Ptr->Prev;
      else
        Ptr->Next->Prev = Ptr->Prev;
      NodeCount--;
      if (Current == Ptr)
        Current = Ptr->Next;
      delete Ptr;
    }
    /* Return the object */
    return Object;
  }

  void Remove(DataType* Data)
  {
    /* Find the node */
    ListNode<DataType>* Ptr = First;
    while (Ptr != NULL && Ptr->Data != Data)
      Ptr = Ptr->Next;
    if (Ptr != NULL)
    {
      /* Remove the node from the list */
      if (Ptr->Prev == NULL)
        First = Ptr->Next;
      else
        Ptr->Prev->Next = Ptr->Next;
      if (Ptr->Next == NULL)
        Last = Ptr->Prev;
      else
        Ptr->Next->Prev = Ptr->Prev;
      NodeCount--;
      if (Current == Ptr)
        Current = Ptr->Next;
      delete Ptr;
    }
  }

  unsigned int Size() const
  {
    return NodeCount;
  }

  void Sort(COMPARATOR Comparator, bool Descending = false)
  {
    /* Sort the objects using a bubble sort */
    bool Swapped;
    do
    {
      ListNode<DataType>* Ptr = First;
      Swapped = false;
      while (Ptr != NULL)
      {
        ListNode<DataType>* Next = Ptr->Next;
        if (Next != NULL && ((Descending && (*Comparator)(Ptr->Data, Next->Data) > 0) || (!Descending && (*Comparator)(Ptr->Data, Next->Data) < 0)))
        {
          /* Swap Next with Ptr (place Next before Ptr instead of after) */
          if (Ptr->Prev != NULL)
            Ptr->Prev->Next = Next;
          else
            First = Next;
          if (Next->Next != NULL)
            Next->Next->Prev = Ptr;
          else
            Last = Ptr;
          Ptr->Next = Next->Next;
          Next->Prev = Ptr->Prev;
          Ptr->Prev = Next;
          Next->Next = Ptr;
          Swapped = true;
        }
        else
          Ptr = Ptr->Next;
      }
    }
    while (Swapped);
  }

private:
  mutable ListNode<DataType>* Current;
  ListNode<DataType>* First;
  ListNode<DataType>* Last;
  unsigned int NodeCount;

  ListNode<DataType>* GetNode(unsigned int Index) const
  {
    if (Index < NodeCount)
    {
      if (Index <= NodeCount/2)
      {
        /* Find the node */
        ListNode<DataType>* Ptr = First;
        while (Ptr != NULL && Index > 0)
        {
          Ptr = Ptr->Next;
          Index--;
        }
        /* Return the node */
        return Ptr;
      }
      else
      {
        /* Find the node */
        ListNode<DataType>* Ptr = Last;
        while (Ptr != NULL && Index < NodeCount-1)
        {
          Ptr = Ptr->Prev;
          Index++;
        }
        /* Return the node */
        return Ptr;
      }
    }
    return NULL;
  }
};

#endif
