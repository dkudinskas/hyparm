#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/timer32k.h"


#define TIMER32K_BASE              0x48320000

#define REG_TIMER_32K_SYSCONFIG       0x0004
#define REG_TIMER_32K_COUNTER         0x0010


void initTimer32k(virtualMachine *vm)
{
  struct SynchronizedTimer32k *timer32k = (struct SynchronizedTimer32k *)calloc(1, sizeof(struct SynchronizedTimer32k));
  if (timer32k == NULL)
  {
    DIE_NOW(NULL, "Failed to allocate 32Khz sync timer struct.");
  }

  DEBUG(VP_OMAP_35XX_TIMER32K, "Initializing 32kHz synchronized timer at %p" EOL, timer32k);

  // sysconf value is emulated. Reset to zero
  timer32k->timer32SysconfReg = 0;
  volatile u32int * memPtr = (u32int *)(TIMER32K_BASE + REG_TIMER_32K_COUNTER);
  timer32k->counterVal = *memPtr;

  vm->timer32k = timer32k;
}

u32int loadTimer32k(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  struct SynchronizedTimer32k *const timer32k = context->vm.timer32k;
  u32int val = 0;

  DEBUG(VP_OMAP_35XX_TIMER32K, "%s load from physical address: %#.8x, vAddr %#.8x, aSize %#x" EOL,
      dev->deviceName, phyAddr, virtAddr, (u32int)size);

  if (size == WORD)
  {
    u32int regAddr = phyAddr - TIMER_32K; 
    if (regAddr == REG_TIMER_32K_SYSCONFIG)
    {
      val = timer32k->timer32SysconfReg;
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
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
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

void storeTimer32k(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
}

