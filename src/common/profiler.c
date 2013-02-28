#include "common/debug.h"
#include "common/stddef.h"

#include "common/profiler.h"

#include "drivers/beagle/beGPTimer.h"
#include "drivers/beagle/beProfiler.h"

/*
  you need a *char device with major number 222
  and then you can write r to flush its buffer
  if you read it, you'll get a list of PC values
  as native unsigned ints if I remember correctly
  so basically just do something like echo r > /dev/profiler
  and when you're done, cat /dev/profiler > ./list_of_pc_values
*/

struct Profiler profiler;

void profilerPause() 
{
  if (profiler.enabled) 
  {
    profiler.enabled = FALSE;
    gptBEStop(PROFILER_GPT);
  }
}

void profilerResume() 
{
  if (!profiler.enabled) 
  {
    profiler.enabled = TRUE;
    gptBEStart(PROFILER_GPT);
  }
}

void profilerReset() 
{
  profiler.writeIndex = 0;
  profiler.counter = 0;
  profiler.bufferFull = FALSE;
}

void profilerInit() 
{
  profilerReset();
  profilerBEInit();
}

void profilerRecord(u32int address) 
{
  if (profiler.enabled && !profiler.bufferFull) 
  {
    profiler.entries[profiler.writeIndex] = address;
/*    profiler.writeIndex = (profiler.writeIndex + 1) % PROFILER_ENTRY_NUM;
    profiler.counter++;
    if (!profiler.bufferFull && (profiler.counter >= PROFILER_ENTRY_NUM)) 
    {
      profiler.bufferFull = TRUE;
    }
*/
    profiler.writeIndex++;
    profiler.counter++;
    if (profiler.writeIndex >= PROFILER_ENTRY_NUM)
    {
      printf("profiler: buffer full.\n");
      profiler.bufferFull = TRUE;
    }
  }
}

