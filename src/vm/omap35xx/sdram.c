#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/sdram.h"


struct SdramController * sdram;

void initSdram(void)
{
  sdram = (struct SdramController *)malloc(sizeof(struct SdramController));
  if (sdram == 0)
  {
    DIE_NOW(NULL, "Failed to allocate SDRAM instance");
  }
  else
  {
    memset((void*)sdram, 0x0, sizeof(struct SdramController));
    DEBUG(VP_OMAP_35XX_SDRAM, "Sdram instance at %p" EOL, sdram);
  }

  sdram->enabled = 1;

#ifdef SDRAM_STORE_COUNTER
  u32int * storeTrace = (u32int*)malloc(MEGABYTE_COUNT * sizeof(u32int));
  if (storeTrace == 0)
  {
    DIE_NOW(NULL, "Failed to allocate store trace.");
  }
  else
  {
    memset((void*)storeTrace, 0x0, MEGABYTE_COUNT*sizeof(u32int));
    printf("Store trace at %.8x" EOL, (u32int)storeTrace);
  }
  sdram->storeCounters = storeTrace;

  u32int y = 0;
  for (y = 0; y < MEGABYTE_COUNT; y++)
  {
    sdram->storeCounters[y] = 0;
  }
#endif
}

void dumpSdramStats()
{
#ifdef SDRAM_STORE_COUNTER
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


u32int loadSdram(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
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


void storeSdram(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  DEBUG(VP_OMAP_35XX_SDRAM, "%s store to physical address: %#.8x, vAddr %#.8x, aSize %#x, val %#.8x"
      EOL, dev->deviceName, phyAddr, virtAddr, (u32int)size, value);
//  fprintf("%s store to physical address: %#.8x, vAddr %#.8x, aSize %#x, val %#.8x\n",
//         dev->deviceName, phyAddr, virtAddr, (u32int)size, value);

#ifdef SDRAM_STORE_COUNTER
  u32int index = (virtAddr >> 20) & 0xFFF;
  sdram->storeCounters[index] = sdram->storeCounters[index] + 1;
#endif

  switch (size)
  {
    case WORD:
    {
      // I presume page table edits only happen in full word accesses... dont they?
      if (isAddrInPageTable(getGuestContext()->pageTables->guestPhysical, phyAddr))
      {
        pageTableEdit(virtAddr, value);
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
