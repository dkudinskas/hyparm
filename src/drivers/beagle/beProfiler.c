#include "common/debug.h"
#include "common/stddef.h"
#include "common/profiler.h"

#include "drivers/beagle/beGPTimer.h"
#include "drivers/beagle/beClockMan.h"
#include "drivers/beagle/beIntc.h"
#include "drivers/beagle/beProfiler.h"

void profilerBEInit()
{
  printf("profilerBEInit.()\n");
  setInterruptMapping(PROFILER_GPT_IRQ, INTCPS_FIQNIRQ_FIQ);
  setClockSource(PROFILER_GPT, FALSE);
  toggleTimerFclk(PROFILER_GPT, TRUE);
  gptBEInit(PROFILER_GPT);
  gptBESetPeriod(PROFILER_GPT);
  gptBEEnableOverflowInterrupt(PROFILER_GPT);
  unmaskInterruptBE(PROFILER_GPT_IRQ);
  profiler.enabled = TRUE;
  gptBEStart(PROFILER_GPT);
}
