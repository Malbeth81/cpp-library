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

class Observable;

class Observer
{
public:
  Observer()
  {
  }

  virtual ~Observer()
  {
  }

  virtual void Update(const Observable* object, int Event)
  {
  }
};

class Observable
{
public:
  Observable()
  {
    /* Initialize members */
    Observers = 0;
    ObserverCount = 0;
    UpdateCount = 0;
  }

  ~Observable()
  {
    /* Clean up */
    if (Observers != 0)
      delete[] Observers;
  }

  void AddObserver(Observer* object)
  {
    /* Increase the size of the array */
    Observer** Array = new Observer*[ObserverCount+1];
    for (int i = 0; i < ObserverCount; i++)
      Array[i] = Observers[i];
    if (Observers != 0)
      delete[] Observers;
    Observers = Array;

    /* Add new object */
    Observers[ObserverCount] = object;
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

  void NotifyObservers(int Event)
  {
    if (UpdateCount == 0)
    {
      /* Update all the objects in the array */
      for (int i = 0; i < ObserverCount; i++)
        Observers[i]->Update(this, Event);
    }
  }

private:
  Observer** Observers;
  unsigned short ObserverCount;
  unsigned short UpdateCount;
};

#endif
