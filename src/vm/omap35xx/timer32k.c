#include "common/debug.h"
#include "common/stddef.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/timer32k.h"


static u32int timer32SysconfReg = 0;
static u32int counterVal = 0;


u32int loadTimer32k(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  u32int val = 0;

  DEBUG(VP_OMAP_35XX_TIMER32K, "%s load from physical address: %#.8x, vAddr %#.8x, aSize %#x" EOL,
      dev->deviceName, phyAddr, virtAddr, (u32int)size);

  if (size == WORD)
  {
    u32int regAddr = phyAddr - TIMER_32K; 
    if (regAddr == REG_TIMER_32K_SYSCONFIG)
    {
      val = timer32SysconfReg;
      DEBUG(VP_OMAP_35XX_TIMER32K, "%s load sys cfg value %#x" EOL, dev->deviceName, val);
    }
    else if (regAddr == REG_TIMER_32K_COUNTER)
    {
      // for now, just load the real counter value.
      volatile u32int * memPtr = (u32int*)virtAddr;
      val = *memPtr;
      val = val >> 5;
      DEBUG(VP_OMAP_35XX_TIMER32K, "%s load counter value %#x" EOL, dev->deviceName, val);
    }
    else
    {
      printf("%s load from physical address: %#.8x, vAddr %#.8x" EOL, dev->deviceName, phyAddr,
          virtAddr);
      DIE_NOW(NULL, "Invalid register!");
    }
  }
  else
  {
    printf("%s load from physical address: %#.8x, vAddr %#.8x" EOL, dev->deviceName, phyAddr,
        virtAddr);
    DIE_NOW(NULL, "Invalid register access size (non32bit)");
  }
  return val;
}

void storeTimer32k(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  DIE_NOW(NULL, "32k timer store unimplemented.");
}

void initTimer32k()
{
  // sysconf value is emulated. Reset to zero
  timer32SysconfReg = 0;
  volatile u32int * memPtr = (u32int *)(TIMER32K_BASE + REG_TIMER_32K_COUNTER);
  counterVal = *memPtr;
}
