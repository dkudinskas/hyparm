#include "common/debug.h"
#include "common/stddef.h"

#include "common/profiler.h"

#include "drivers/beagle/beGPTimer.h"
#include "drivers/beagle/beProfiler.h"

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
  if (profiler.enabled) 
  {
    profiler.entries[profiler.writeIndex] = address;
    profiler.writeIndex = (profiler.writeIndex + 1) % PROFILER_ENTRY_NUM;
    profiler.counter++;

    if (!profiler.bufferFull && (profiler.counter >= PROFILER_ENTRY_NUM)) 
    {
      profiler.bufferFull = TRUE;
    }
  }
}

