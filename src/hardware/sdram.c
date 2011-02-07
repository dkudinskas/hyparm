#include "sdram.h"
#include "memoryConstants.h" // for BEAGLE_RAM_START/END
#include "pageTable.h" // for getPhysicalAddress()
#include "guestContext.h"
#include "dataMoveInstr.h"
#include "debug.h"

extern GCONTXT * getGuestContext(void);

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
