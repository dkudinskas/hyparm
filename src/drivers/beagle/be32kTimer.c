#include "common/debug.h"

#include "drivers/beagle/be32kTimer.h"

#include "vm/omap35xx/serial.h"


static u32int baseVA = 0;

u32int beLoadTimer32k(u32int address)
{
  u32int val = 0;
  volatile u32int * memPtr = (u32int*)address;
  val = *memPtr;
  return val;
}

void beInitTimer32k(void)
{
  baseVA = findVAforPA(TIMER32K_BASE);
  if (baseVA == 0)
  {
    DIE_NOW(0, "could not find VA for 32kTimer PA");
  }
}

u32int getCounterVal(void)
{
  u32int addr = findVAforPA(TIMER32K_BASE+REG_TIMER_32K_COUNTER);
  if (addr == 0)
  {
    DIE_NOW(0, "could not find VA for 32kTimer PA");
  }
  else
  {
    u32int val = beLoadTimer32k(addr);
#ifdef BE_TIMER32K_DBG
    serial_putstring("BE_32KTIMER: load counter val ");
    serial_putint_nozeros(val);
    serial_newline();
#endif
    return val;
  }
  return 0;
}
