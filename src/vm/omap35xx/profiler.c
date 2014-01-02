#include "common/debug.h"
#include "common/stddef.h"

#include "common/profiler.h"

#include "guestManager/guestExceptions.h"

#include "vm/omap35xx/hardwareLibrary.h"
#include "vm/omap35xx/profiler.h"

void initProfilerInt()
{
  // do nothing
}

u32int loadProfilerInt(GCONTXT *context, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  u32int val;
  bool found = FALSE;
  
  u32int reg = phyAddr - PROFILER;
  switch (reg)
  {
    case PROFILER_CONTROL_REG:
    {
      found = TRUE;
      val = (profiler.enabled) ? PROFILER_CONTROL_REG_RUNNING : 0;
      break;
    }
    case PROFILER_SAMPLE_SIZE_REG:
    {
      found = TRUE;
      val = (profiler.counter < PROFILER_ENTRY_NUM) ? profiler.counter : PROFILER_ENTRY_NUM;
      break;
    }
    case PROFILER_DATA_REG:
    {
      found = TRUE;
      val = profiler.entries[profiler.readIndex];
      profiler.readIndex = (profiler.readIndex + 1) % PROFILER_ENTRY_NUM;
      break;
    }
    default:
      printf("Profiler Interface: reg 0x%x\n", reg);
      DIE_NOW(NULL, "Read from unknown register");
  }
  
  if (!found)
  {
    printf("phyAddr: 0x%x\n", phyAddr);
    DIE_NOW(NULL, "Unknown register");
  }
  
  return val;
}

void storeProfilerInt(GCONTXT *context, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  u32int reg = phyAddr - PROFILER;
  switch (reg)
  {
    case PROFILER_CONTROL_REG:
      // pause & resume
      if (value & PROFILER_CONTROL_REG_RUNNING)
      {
        if (!profiler.enabled) 
        {
          profilerResume();
        }
      }
      else
      {
        if (profiler.enabled)
        {
          profilerPause();
          if (profiler.bufferFull)
          {
            profiler.readIndex = profiler.writeIndex;
          } else {
            profiler.readIndex = 0;
          }
        }
      }
      
      // Reset bit
      if (!profiler.enabled && (value & PROFILER_CONTROL_REG_RESET))
      {
        profilerReset();
      }
      
      break;
    default:
      DIE_NOW(NULL, "Unknown register");
  }
}
