/*
* Beeper.h
*
* Functions that allows to periodically plays a tone on the speaker
*
* Copyright (C) 2010 Vortex Solution.
* Created by : Marc-André Lamothe
*/

#ifndef Beeper_H
#define Beeper_H

#include <windows.h>

struct BeepInfo
{
  unsigned int Interval;  // in milliseconds
  unsigned int Count;     // nb played each time
  unsigned int Frequency; // in Hertz
  unsigned int Duration;  // in milliseconds
};

BeepInfo* StartBeepCycle(const unsigned int Interval, const unsigned int Count = 1, const unsigned int Frequency = 400, const unsigned int Duration = 200);
void StopBeepCycle(BeepInfo* Data);

#endif
