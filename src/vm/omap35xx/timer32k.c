#include "common/debug.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/timer32k.h"

#include "memoryManager/memoryConstants.h" // for BEAGLE_RAM_START/END
#include "memoryManager/pageTable.h" // for getPhysicalAddress()


extern GCONTXT * getGuestContext(void);

static u32int timer32SysconfReg = 0;
static u32int counterVal = 0;

u32int loadTimer32k(device * dev, ACCESS_SIZE size, u32int address)
{
  u32int val = 0;

  //We care about the real physical address of the entry, not its virtual address
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

#ifdef TIMER32K_DBG
  printf(dev->deviceName);
  printf(" load from physical address: %08x, vAddr %08x, aSize %x\n",
          phyAddr, address, (u32int)size);
#endif

  if (size == WORD)
  {
    u32int regAddr = phyAddr - TIMER_32K; 
    if (regAddr == REG_TIMER_32K_SYSCONFIG)
    {
      val = timer32SysconfReg;
#ifdef TIMER32K_DBG
      printf(dev->deviceName);
      printf(" load sys cfg value %x\n", val);
#endif
    }
    else if (regAddr == REG_TIMER_32K_COUNTER)
    {
      // for now, just load the real counter value.
      volatile u32int * memPtr = (u32int*)address;
      val = *memPtr;
      val = val >> 5;
#ifdef TIMER32K_DBG
      printf(dev->deviceName);
      printf(" load counter value %x\n", val);
#endif
    }
    else
    {
      printf(dev->deviceName);
      printf(" load from physical address: %08x, vAddr %08x\n", phyAddr, address);
      DIE_NOW(gc, "Invalid register!");
    }
  }
  else
  {
    printf(dev->deviceName);
    printf(" load from physical address: %08x, vAddr %08x\n", phyAddr, address);
    DIE_NOW(gc, "Invalid register access size (non32bit)");
  }
  return val;
}

void storeTimer32k(device * dev, ACCESS_SIZE size, u32int address, u32int value)
{
  DIE_NOW(0, "32k timer store unimplemented.");
}

void initTimer32k()
{
  // sysconf value is emulated. Reset to zero
  timer32SysconfReg = 0;
  volatile u32int * memPtr = (u32int*)(TIMER32K_BASE+REG_TIMER_32K_COUNTER);
  counterVal = *memPtr;
}
