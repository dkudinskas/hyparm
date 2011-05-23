#include "common/debug.h"

#include "drivers/beagle/be32kTimer.h"


u32int beLoadTimer32k(u32int address)
{
  u32int val = 0;
  volatile u32int * memPtr = (u32int*)address;
  val = *memPtr;
  return val;
}

void beInitTimer32k(void)
{
  // do nothing
}

u32int getCounterVal(void)
{
  u32int val = beLoadTimer32k(TIMER32K_BASE+REG_TIMER_32K_COUNTER);
#ifdef BE_TIMER32K_DBG
  printf("BE_32KTIMER: load counter val %x\n");
#endif
  return val;
}

/* Use the 32kHz timer to wait a specified number of milliseconds */
void mdelay32k(u32int x)
{
  u32int c = beLoadTimer32k(TIMER32K_BASE+REG_TIMER_32K_COUNTER);
  u32int end_count = c + 32 * x; //32 ticks per millisecond
  while (c < end_count)
  {
    c = beLoadTimer32k(TIMER32K_BASE+REG_TIMER_32K_COUNTER);
  }
}
