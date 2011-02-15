#include "sdram.h"
#include "memoryConstants.h" // for BEAGLE_RAM_START/END
#include "pageTable.h" // for getPhysicalAddress()
#include "guestContext.h"
#include "dataMoveInstr.h"
#include "debug.h"
#include "memFunctions.h"

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
    serial_putstring("Sdram instance at 0x");
    serial_putint((u32int)sdram);
    serial_newline();
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
    serial_putstring("Store trace at 0x");
    serial_putint((u32int)storeTrace);
    serial_newline();
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
  serial_putstring("Store trace: ");
  serial_newline();

  u32int i = 0;
  for (i = 0; i < MEGABYTE_COUNT; i++)
  {
    if (sdram->storeCounters[i] != 0)
    {
      serial_putint(i << 20);
      serial_putstring(": ");
      serial_putint(sdram->storeCounters[i]);
      serial_newline();
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
  serial_putstring(dev->deviceName);
  serial_putstring(" load from physical address: 0x");
  serial_putint(phyAddr);
  serial_putstring(", virtual address: 0x");
  serial_putint(address);
  serial_putstring(" access size ");
  serial_putint((u32int)size);
  serial_newline();
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
      serial_putstring(dev->deviceName);
      serial_putstring(" load from physical address: 0x");
      serial_putint(phyAddr);
      serial_putstring(", virtual address: 0x");
      serial_putint(address);
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
  serial_putstring(dev->deviceName);
  serial_putstring(" store to physical address: 0x");
  serial_putint(phyAddr);
  serial_putstring(", virtual address: 0x");
  serial_putint(address);
  serial_putstring(" access size ");
  serial_putint((u32int)size);
  serial_putstring(" value ");
  serial_putint(value);
  serial_newline();
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
      serial_putstring(dev->deviceName);
      serial_putstring(" store to physical address: 0x");
      serial_putint(phyAddr);
      serial_putstring(", virtual address: 0x");
      serial_putint(address);
      serial_putstring(" access size ");
      serial_putint((u32int)size);
      serial_putstring(" value ");
      serial_putint(value);
      serial_newline();
      DIE_NOW(0, " invalid access size.");
  }
}
