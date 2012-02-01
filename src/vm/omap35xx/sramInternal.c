#include "common/debug.h"
#include "common/stddef.h"

#include "vm/omap35xx/sdram.h"
#include "vm/omap35xx/sramInternal.h"

#include "memoryManager/memoryConstants.h" // for BEAGLE_RAM_START/END
#include "memoryManager/pageTable.h" // for getPhysicalAddress()


u32int loadSramInternal(device * dev, ACCESS_SIZE size, u32int address)
{
  DIE_NOW(NULL, "SRAM_INTERNAL load unimplemented.");
  u32int val = 0;

  //We care about the real physical address of the entry, not its virtual address
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

  DEBUG(VP_OMAP_35XX_SRAM, "%s load from physical address: %#.8x, virtual address: %#.8x aSize %#x"
      EOL, dev->deviceName, phyAddr, address, (u32int)size);

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
    {
      printf("%s load from pAddr: %#.8x, vAddr: %#.8x" EOL, dev->deviceName, phyAddr, address);
      DIE_NOW(gc, "Invalid access size.");
    }
  }
  return val;
}

void storeSramInternal(device * dev, ACCESS_SIZE size, u32int address, u32int value)
{
  //We care about the real physical address of the entry, not its virtual address
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

  DEBUG(VP_OMAP_35XX_SRAM, "%s store to pAddr: %#.8x, vAddr %#.8x, aSize %#x, val %#.8x" EOL,
      dev->deviceName, phyAddr, address, (u32int)size, value);

  switch (size)
  {
    case WORD:
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
    case HALFWORD:
    {
      if ( (phyAddr >= 0x4020FFC8) && (phyAddr <= 0x4020FFFE) )
      {
        printf("%s: register guest exception handler address, halfword access" EOL,
            dev->deviceName);
        DIE_NOW(gc, "UNIMPLEMENTED");
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
        printf("%s: register guest exception handler address, byte access" EOL, dev->deviceName);
        DIE_NOW(gc, "UNIMPLEMENTED");
      }
      // store the value...
      u8int * memPtr = (u8int*)address;
      *memPtr = (u8int)value;
      break;
    }
    default:
      printf("%s store to pAddr %#.8x, vAddr %#.8x, aSize %#x, val %#.8x" EOL, dev->deviceName,
          phyAddr, address, (u32int)size, value);
      DIE_NOW(gc, "Invalid access size.");
  }
}

void registerGuestHandler(GCONTXT *gc, u32int address, u32int value)
{
  DEBUG(VP_OMAP_35XX_SRAM, "INTERNAL SRAM: guest registering ");
  switch (address)
  {
    case UNDEFINED_EXCEPTION_ADDR:
      DEBUG(VP_OMAP_35XX_SRAM, " UNDEF handler at address %#.8x" EOL, value);
      gc->guestUndefinedHandler = value;
      break;
    case SWI_EXCEPTION_ADDR:
      DEBUG(VP_OMAP_35XX_SRAM, " SWI handler at address %#.8x" EOL, value);
      gc->guestSwiHandler = value;
      break;
    case PREFETCH_ABT_EXCEPTION_ADDR:
      DEBUG(VP_OMAP_35XX_SRAM, " P_ABT handler at address %#.8x" EOL, value);
      gc->guestPrefAbortHandler = value;
      break;
    case DATA_ABT_EXCEPTION_ADDR:
      DEBUG(VP_OMAP_35XX_SRAM, " D_ABT handler at address %#.8x" EOL, value);
      gc->guestDataAbortHandler = value;
      break;
    case UNUSED_EXCEPTION_ADDR:
      DEBUG(VP_OMAP_35XX_SRAM, " UNUSED handler at address %#.8x" EOL, value);
      gc->guestUnusedHandler = value;
      break;
    case IRQ_EXCEPTION_ADDR:
      DEBUG(VP_OMAP_35XX_SRAM, " IRQ handler at address %#.8x" EOL, value);
      gc->guestIrqHandler = value;
      break;
    case FIQ_EXCEPTION_ADDR:
      DEBUG(VP_OMAP_35XX_SRAM, " FIQ handler at address %#.8x" EOL, value);
      gc->guestFiqHandler = value;
      break;
    default:
      DEBUG(VP_OMAP_35XX_SRAM, " a new handler instruction %#x at %#.8x" EOL, value, address);
  } /* switch (address) */
}

