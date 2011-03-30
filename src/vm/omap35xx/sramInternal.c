#include "common/debug.h"

#include "vm/omap35xx/sdram.h"
#include "vm/omap35xx/sramInternal.h"

#include "memoryManager/memoryConstants.h" // for BEAGLE_RAM_START/END
#include "memoryManager/pageTable.h" // for getPhysicalAddress()


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

void storeSramInternal(device * dev, ACCESS_SIZE size, u32int address, u32int value)
{
  //We care about the real physical address of the entry, not its virtual address
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

#ifdef SRAM_INTERNAL_DBG
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
        DEBUG_STRING(dev->deviceName);
        DEBUG_STRING(": register guest exception handler address, halfword access ");
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
        DEBUG_STRING(dev->deviceName);
        DEBUG_STRING(": register guest exception handler address, byte access ");
        DIE_NOW(0, "UNIMPLEMENTED");
      } 
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

void registerGuestHandler(GCONTXT* gc, u32int address, u32int value)
{
#ifdef SRAM_INTERNAL_DBG
  DEBUG_STRING("INTERNAL SRAM: guest registering ");
#endif
  switch(address)
  {
    case UNDEFINED_EXCEPTION_ADDR:
#ifdef SRAM_INTERNAL_DBG
      DEBUG_STRING(" UNDEF handler at address ");
      DEBUG_INT(value);
#endif
      gc->guestUndefinedHandler = value;
      break;
    case SWI_EXCEPTION_ADDR:
#ifdef SRAM_INTERNAL_DBG
      DEBUG_STRING(" SWI handler at address ");
      DEBUG_INT(value);
#endif
      gc->guestSwiHandler = value;
      break;
    case PREFETCH_ABT_EXCEPTION_ADDR:
#ifdef SRAM_INTERNAL_DBG
      DEBUG_STRING(" P_ABT handler at address ");
      DEBUG_INT(value);
#endif
      gc->guestPrefAbortHandler = value;
      break;
    case DATA_ABT_EXCEPTION_ADDR:
#ifdef SRAM_INTERNAL_DBG
      DEBUG_STRING(" D_ABT handler at address ");
      DEBUG_INT(value);
#endif
      gc->guestDataAbortHandler = value;
      break;
    case UNUSED_EXCEPTION_ADDR:
#ifdef SRAM_INTERNAL_DBG
      DEBUG_STRING(" UNUSED handler at address ");
      DEBUG_INT(value);
#endif
      gc->guestUnusedHandler = value;
      break;
    case IRQ_EXCEPTION_ADDR:
#ifdef SRAM_INTERNAL_DBG
      DEBUG_STRING(" IRQ handler at address ");
      DEBUG_INT(value);
#endif
      gc->guestIrqHandler = value;
      break;
    case FIQ_EXCEPTION_ADDR:
#ifdef SRAM_INTERNAL_DBG
      DEBUG_STRING(" FIQ handler at address ");
      DEBUG_INT(value);
#endif
      gc->guestFiqHandler = value;
      break;
    default:
#ifdef SRAM_INTERNAL_DBG
      DEBUG_STRING(" a new handler instruction ");
      DEBUG_INT(value);
      DEBUG_STRING("  at ");
      DEBUG_INT(address);
#endif
      break;
      // not an actual write to guest exception vector.. just let it go.
  } // switch ends
#ifdef SRAM_INTERNAL_DBG
  DEBUG_NEWLINE();
#endif
}

