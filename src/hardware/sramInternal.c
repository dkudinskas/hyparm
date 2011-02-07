#include "sramInternal.h"
#include "debug.h"
#include "sdram.h"
#include "memoryConstants.h" // for BEAGLE_RAM_START/END
#include "pageTable.h" // for getPhysicalAddress()
#include "dataMoveInstr.h"

extern GCONTXT * getGuestContext(void);

u32int loadSramInternal(device * dev, ACCESS_SIZE size, u32int address)
{
  DIE_NOW(0, "SRAM_INTERNAL load unimplemented.");
  u32int val = 0;

  //We care about the real physical address of the entry, not its virtual address
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

#ifdef SRAM_INTERNAL_DBG
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

void storeSramInternal(device * dev, ACCESS_SIZE size, u32int address, u32int value)
{
  //We care about the real physical address of the entry, not its virtual address
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

#ifdef SRAM_INTERNAL_DBG
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
      if ( (phyAddr >= 0x4020FFC8) && (phyAddr <= 0x4020FFFC) )
      {
        registerGuestHandler(gc, phyAddr, value);
      }
      else
      {
        // store the value...
        u32int * memPtr = (u32int*)address;
        *memPtr = value;
      }
      break;
    }
    case HALFWORD:
    {
      if ( (phyAddr >= 0x4020FFC8) && (phyAddr <= 0x4020FFFE) )
      {
        serial_putstring(dev->deviceName);
        serial_putstring(": register guest exception handler address, halfword access ");
        DIE_NOW(0, "UNIMPLEMENTED");
      } 
      // store the value...
      u16int * memPtr = (u16int*)address;
      *memPtr = (u16int)value;
      break;
    }
    case BYTE:
    {
      if ( (phyAddr >= 0x4020FFC8) && (phyAddr <= 0x4020FFFF) )
      {
        serial_putstring(dev->deviceName);
        serial_putstring(": register guest exception handler address, byte access ");
        DIE_NOW(0, "UNIMPLEMENTED");
      } 
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

void registerGuestHandler(GCONTXT* gc, u32int address, u32int value)
{
#ifdef SRAM_INTERNAL_DBG
  serial_putstring("INTERNAL SRAM: guest registering ");
#endif
  switch(address)
  {
    case UNDEFINED_EXCEPTION_ADDR:
#ifdef SRAM_INTERNAL_DBG
      serial_putstring(" UNDEF handler at address ");
      serial_putint(value);
#endif
      gc->guestUndefinedHandler = value;
      break;
    case SWI_EXCEPTION_ADDR:
#ifdef SRAM_INTERNAL_DBG
      serial_putstring(" SWI handler at address ");
      serial_putint(value);
#endif
      gc->guestSwiHandler = value;
      break;
    case PREFETCH_ABT_EXCEPTION_ADDR:
#ifdef SRAM_INTERNAL_DBG
      serial_putstring(" P_ABT handler at address ");
      serial_putint(value);
#endif
      gc->guestPrefAbortHandler = value;
      break;
    case DATA_ABT_EXCEPTION_ADDR:
#ifdef SRAM_INTERNAL_DBG
      serial_putstring(" D_ABT handler at address ");
      serial_putint(value);
#endif
      gc->guestDataAbortHandler = value;
      break;
    case UNUSED_EXCEPTION_ADDR:
#ifdef SRAM_INTERNAL_DBG
      serial_putstring(" UNUSED handler at address ");
      serial_putint(value);
#endif
      gc->guestUnusedHandler = value;
      break;
    case IRQ_EXCEPTION_ADDR:
#ifdef SRAM_INTERNAL_DBG
      serial_putstring(" IRQ handler at address ");
      serial_putint(value);
#endif
      gc->guestIrqHandler = value;
      break;
    case FIQ_EXCEPTION_ADDR:
#ifdef SRAM_INTERNAL_DBG
      serial_putstring(" FIQ handler at address ");
      serial_putint(value);
#endif
      gc->guestFiqHandler = value;
      break;
    default:
#ifdef SRAM_INTERNAL_DBG
      serial_putstring(" a new handler instruction ");
      serial_putint(value);
      serial_putstring("  at ");
      serial_putint(address);
#endif
      break;
      // not an actual write to guest exception vector.. just let it go.
  } // switch ends
#ifdef SRAM_INTERNAL_DBG
  serial_newline();
#endif
}

