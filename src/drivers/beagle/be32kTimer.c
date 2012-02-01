#include "common/debug.h"

#include "drivers/beagle/be32kTimer.h"


#define TIMER32K_BASE              0x48320000

#define REG_TIMER_32K_SYSCONFIG        0x0004
#define REG_TIMER_32K_COUNTER          0x0010


static inline u32int beLoadTimer32k(u32int address);


static inline u32int beLoadTimer32k(u32int address)
{
  return *(volatile u32int *)address;
}

u32int getCounterVal(void)
{
  u32int val = beLoadTimer32k(TIMER32K_BASE + REG_TIMER_32K_COUNTER);
  DEBUG(PP_OMAP_35XX_TIMER32K, "BE_32KTIMER: load counter val %#x" EOL, val);
  return val;
}

/* Use the 32kHz timer to wait a specified number of milliseconds */
void mdelay32k(u32int x)
{
  u32int c = beLoadTimer32k(TIMER32K_BASE + REG_TIMER_32K_COUNTER);
  u32int endCount = c + 32 * x; //32 ticks per millisecond
  while (c < endCount)
  {
    c = beLoadTimer32k(TIMER32K_BASE + REG_TIMER_32K_COUNTER);
  }
}
