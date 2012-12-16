#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "vm/omap35xx/sdram.h"


void initSdram(virtualMachine *vm)
{
  struct SdramController *sdram = (struct SdramController *)calloc(1, sizeof(struct SdramController));
  if (sdram == NULL)
  {
    DIE_NOW(NULL, "Failed to allocate SDRAM instance");
  }
  DEBUG(VP_OMAP_35XX_SDRAM, "Sdram instance at %p" EOL, sdram);

  sdram->enabled = 1;

  vm->sdram = sdram;

#ifdef CONFIG_SDRAM_STORE_COUNTER
  u32int *storeTrace = (u32int *)calloc(MEGABYTE_COUNT, sizeof(u32int));
  if (storeTrace == NULL)
  {
    DIE_NOW(NULL, "Failed to allocate store trace.");
  }
  printf("Store trace at %.8x" EOL, (u32int)storeTrace);
  sdram->storeCounters = storeTrace;

  u32int y = 0;
  for (y = 0; y < MEGABYTE_COUNT; y++)
  {
    sdram->storeCounters[y] = 0;
  }
#endif
}

void dumpSdramStats(struct SdramController *sdram)
{
#ifdef CONFIG_SDRAM_STORE_COUNTER
  printf("Store trace:" EOL);

  u32int i = 0;
  for (i = 0; i < MEGABYTE_COUNT; i++)
  {
    if (sdram->storeCounters[i] != 0)
    {
      printf("%x: %x" EOL, i << 20, sdram->storeCounters[i]);
    }
  }
#endif
}


u32int loadSdram(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  u32int val = 0;

  DEBUG(VP_OMAP_35XX_SDRAM, "%s load from physical address: %#.8x, vAddr %#.8x, access size %#x"
      EOL, dev->deviceName, phyAddr, virtAddr, (u32int)size);

  switch (size)
  {
    case WORD:
    {
      u32int * memPtr = (u32int*)virtAddr;
      val = *memPtr;
      break;
    }
    case HALFWORD:
    {
      u16int * memPtr = (u16int*)virtAddr;
      val = *memPtr;
      break;
    }
    case BYTE:
    {
      u8int * memPtr = (u8int*)virtAddr;
      val = *memPtr;
      break;
    }
    default:
      printf("%s load from physical address: %#.8x, vAddr %#.8x" EOL, dev->deviceName, phyAddr,
          virtAddr);
      DIE_NOW(NULL, "Invalid access size.");
  }
  return val;
}


void storeSdram(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
#ifdef CONFIG_SDRAM_STORE_COUNTER
  struct SdramController* sdram = context->vm.sdram;
#endif

  DEBUG(VP_OMAP_35XX_SDRAM, "%s store to physical address: %#.8x, vAddr %#.8x, aSize %#x, val %#.8x"
      EOL, dev->deviceName, phyAddr, virtAddr, (u32int)size, value);

#ifdef CONFIG_SDRAM_STORE_COUNTER
  u32int index = (virtAddr >> 20) & 0xFFF;
  sdram->storeCounters[index] = sdram->storeCounters[index] + 1;
#endif

  switch (size)
  {
    case WORD:
    {
      // I presume page table edits only happen in full word accesses... dont they?
      if (context->virtAddrEnabled)
      {
        if (isAddrInPageTable(context, context->pageTables->guestPhysical, phyAddr))
        {
          pageTableEdit(context, virtAddr, value);
        }
      }
      // store the value...
      u32int * memPtr = (u32int*)virtAddr;
      *memPtr = value;
      break;
    }
    case HALFWORD:
    {
      // store the value...
      u16int * memPtr = (u16int*)virtAddr;
      *memPtr = (u16int)value;
      break;
    }
    case BYTE:
    {
      // store the value...
      u8int * memPtr = (u8int*)virtAddr;
      *memPtr = (u8int)value;
      break;
    }
    default:
      printf("%s store to physical address: %.8x, vAddr %.8x, aSize %x, val %.8x" EOL,
          dev->deviceName, phyAddr, virtAddr, (u32int)size, value);
      DIE_NOW(NULL, "Invalid access size.");
  }
}
