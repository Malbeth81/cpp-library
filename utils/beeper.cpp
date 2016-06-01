/*
* Beeper.h
*
* Functions that allows to periodically plays a tone on the speaker
*
* Copyright (C) 2010 Vortex Solution.
* Created by : Marc-André Lamothe
*/

#include "beeper.h"

// Private functions -----------------------------------------------------------

unsigned long __stdcall BeeperThread(void* Param)
{
  BeepInfo* Data = (BeepInfo*)Param;
  do
  {
    /* Beeps */
    for (unsigned int i = 0; i < Data->Count; i++)
    {
      if (i > 0)
        Sleep(100);
      Beep(Data->Frequency, Data->Duration);
    }
    /* Wait */
    Sleep(Data->Interval);
  }
  while (Data->Interval > 0);
  /* Clean up */
  delete Data;
  return 0;
}

// Public functions ------------------------------------------------------------

BeepInfo* StartBeepCycle(const unsigned int Interval, const unsigned int Count, const unsigned int Frequency, const unsigned int Duration)
{
  /* Start thread */
  if (Interval > 0)
  {
    BeepInfo* Data = new BeepInfo;
    Data->Interval = Interval;
    Data->Count = Count;
    Data->Frequency = Frequency;
    Data->Duration = Duration;
    CreateThread(NULL,0,BeeperThread,Data,0,NULL);
    return Data;
  }
  return NULL;
}

void StopBeepCycle(BeepInfo* Data)
{
  /* Stop thread */
  if (Data != NULL)
    Data->Interval = 0;
}
