#include "common/debug.h"
#include "common/memFunctions.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/sdram.h"

#include "memoryManager/memoryConstants.h" // for BEAGLE_RAM_START/END
#include "memoryManager/pageTable.h" // for getPhysicalAddress()


extern GCONTXT * getGuestContext(void);

struct SdramController * sdram;

void initSdram(void)
{
  sdram = (struct SdramController *)mallocBytes(sizeof(struct SdramController));
  if (sdram == 0)
  {
    DIE_NOW(0, "Failed to allocate SDRAM instance");
  }
  else
  {
    memset((void*)sdram, 0x0, sizeof(struct SdramController));
#ifdef SDRAM_DBG
    DEBUG_STRING("Sdram instance at 0x");
    DEBUG_INT((u32int)sdram);
    DEBUG_NEWLINE();
#endif
  }

  sdram->enabled = 1;


#ifdef SDRAM_STORE_COUNTER
  u32int * storeTrace = (u32int*)mallocBytes(MEGABYTE_COUNT * sizeof(u32int));
  if (storeTrace == 0)
  {
    DIE_NOW(0, "Failed to allocate store trace.");
  }
  else
  {
    memset((void*)storeTrace, 0x0, MEGABYTE_COUNT*sizeof(u32int));
    DEBUG_STRING("Store trace at 0x");
    DEBUG_INT((u32int)storeTrace);
    DEBUG_NEWLINE();
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
  DEBUG_STRING("Store trace: ");
  DEBUG_NEWLINE();

  u32int i = 0;
  for (i = 0; i < MEGABYTE_COUNT; i++)
  {
    if (sdram->storeCounters[i] != 0)
    {
      DEBUG_INT(i << 20);
      DEBUG_STRING(": ");
      DEBUG_INT(sdram->storeCounters[i]);
      DEBUG_NEWLINE();
    }
  }
#endif
}


u32int loadSdram(device * dev, ACCESS_SIZE size, u32int address)
{
  u32int val = 0;

  //We care about the real physical address of the entry, not its virtual address
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

#ifdef SDRAM_DBG
  DEBUG_STRING(dev->deviceName);
  DEBUG_STRING(" load from physical address: 0x");
  DEBUG_INT(phyAddr);
  DEBUG_STRING(", virtual address: 0x");
  DEBUG_INT(address);
  DEBUG_STRING(" access size ");
  DEBUG_INT((u32int)size);
  DEBUG_NEWLINE();
#endif

  switch (size)
  {
    case WORD:
    {
      u32int * memPtr = (u32int*)address;
      val = *memPtr;
      break;
    }
    case HALFWORD:
    {
      u16int * memPtr = (u16int*)address;
      val = *memPtr;
      break;
    }
    case BYTE:
    {
      u8int * memPtr = (u8int*)address;
      val = *memPtr;
      break;
    }
    default:
      DEBUG_STRING(dev->deviceName);
      DEBUG_STRING(" load from physical address: 0x");
      DEBUG_INT(phyAddr);
      DEBUG_STRING(", virtual address: 0x");
      DEBUG_INT(address);
      DIE_NOW(0, " invalid access size.");
  }
  return val;
}

void storeSdram(device * dev, ACCESS_SIZE size, u32int address, u32int value)
{
  //We care about the real physical address of the entry, not its virtual address
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

#ifdef SDRAM_DBG
  DEBUG_STRING(dev->deviceName);
  DEBUG_STRING(" store to physical address: 0x");
  DEBUG_INT(phyAddr);
  DEBUG_STRING(", virtual address: 0x");
  DEBUG_INT(address);
  DEBUG_STRING(" access size ");
  DEBUG_INT((u32int)size);
  DEBUG_STRING(" value ");
  DEBUG_INT(value);
  DEBUG_NEWLINE();
#endif

#ifdef SDRAM_STORE_COUNTER
  u32int index = (address >> 20) & 0xFFF;
  sdram->storeCounters[index] = sdram->storeCounters[index] + 1;
#endif
  
  switch (size)
  {
    case WORD:
    {
      // I presume page table edits only happen in full word accesses... dont they?
      if (isAddrInGuestPT(address))
      {
        pageTableEdit(address, value);
      }
      // store the value...
      u32int * memPtr = (u32int*)address;
      *memPtr = value;
      break;
    }
    case HALFWORD:
    {
      // store the value...
      u16int * memPtr = (u16int*)address;
      *memPtr = (u16int)value;
      break;
    }
    case BYTE:
    {
      // store the value...
      u8int * memPtr = (u8int*)address;
      *memPtr = (u8int)value;
      break;
    }
    default:
      DEBUG_STRING(dev->deviceName);
      DEBUG_STRING(" store to physical address: 0x");
      DEBUG_INT(phyAddr);
      DEBUG_STRING(", virtual address: 0x");
      DEBUG_INT(address);
      DEBUG_STRING(" access size ");
      DEBUG_INT((u32int)size);
      DEBUG_STRING(" value ");
      DEBUG_INT(value);
      DEBUG_NEWLINE();
      DIE_NOW(0, " invalid access size.");
  }
}
