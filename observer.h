/*
* Observer.h
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
#ifndef OBSERVER_H
#define	OBSERVER_H

#include <algorithm>

using namespace std;

class Observer
{
public:
  Observer()
  {
  }

  virtual ~Observer()
  {
  }

  virtual void Notify(const int Event, const void* Param)
  {
  }
};

class Observable
{
public:
  Observable()
  {
    /* Initialize members */
    Observers = NULL;
    ObserverCount = 0;
    UpdateCount = 0;
  }

  ~Observable()
  {
    /* Clean up */
    if (Observers != NULL)
      delete[] Observers;
  }

  void AddObserver(Observer* Object)
  {
    /* Increase the size of the array */
    Observer** NewArray = new Observer*[ObserverCount+1];
    copy(Observers, Observers+ObserverCount, NewArray);
    if (Observers != NULL)
      delete[] Observers;
    Observers = NewArray;

    /* Add new object */
    Observers[ObserverCount] = Object;
    ObserverCount++;
  }

  void BeginUpdate()
  {
    UpdateCount++;
  }

  void EndUpdate()
  {
    UpdateCount--;
  }

  void NotifyObservers(const int Event, const void* Param = NULL)
  {
    if (UpdateCount == 0)
    {
      /* Update all the objects in the array */
      for (unsigned int i = 0; i < ObserverCount; i++)
        Observers[i]->Notify(Event, Param);
    }
  }

private:
  Observer** Observers;
  unsigned int ObserverCount;
  unsigned int UpdateCount;
};

#endif
