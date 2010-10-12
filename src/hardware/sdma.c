#include "sdma.h"
#include "pageTable.h" // for getPhysicalAddress()
#include "guestContext.h"
#include "memFunctions.h"

extern GCONTXT * getGuestContext(void);

struct Sdma * sdma;

void initSdma()
{
  // init function: setup device, reset register values to defaults!
  sdma = (struct Sdma*)mallocBytes(sizeof(struct Sdma));
  if (sdma == 0)
  {
    serial_ERROR("Failed to allocate uart.");
  }
  else
  {
    memset((void*)sdma, 0x0, sizeof(struct Sdma));
#ifdef UART_DBG
    serial_putstring("Initializing Sdma at 0x");
    serial_putint((u32int)sdma);
    serial_newline();
#endif
  }

  resetSdma();
}

void resetSdma()
{
  sdma->temp        = 0x00000000;
}


u32int loadSdma(device * dev, ACCESS_SIZE size, u32int address)
{

  //We care about the real pAddr of the entry, not its vAddr
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);
  u32int value = 0;
  u32int regOffs = phyAddr - SDMA;

  switch (regOffs)
  {
    case SDMA_REVISION:
      value = SDMA_REVISION_NUMBER;
      break;
/*
    case SDMA_IRQSTATUS_Lj:
    case SDMA_IRQENABLE_Lj:
    case SDMA_SYSSTATUS:
    case SDMA_OCP_SYSCONFIG:
    case SDMA_CAPS_0:
    case SDMA_CAPS_2:
    case SDMA_CAPS_3:
    case SDMA_CAPS_4:
    case SDMA_GCR:
    case SDMA_CCRi:
    case SDMA_CLNK_CTRLi:
    case SDMA_CICRi:
    case SDMA_CSRi:
    case SDMA_CSDPi:
    case SDMA_CENi:
    case SDMA_CFNi:
    case SDMA_CSSAi:
    case SDMA_CDSAi:
    case SDMA_CSEIi:
    case SDMA_CSFIi:
    case SDMA_CDEIi:
    case SDMA_CDFIi:
    case SDMA_CSACi:
    case SDMA_CDACi:
    case SDMA_CCENi:
    case SDMA_CCFNi:
    case SDMA_COLORi:
*/
    default:
      dumpGuestContext(getGuestContext());
      serial_putstring("loadSdma reg ");
      serial_putint_nozeros(regOffs);
      serial_newline();
      serial_ERROR("SDMA: load from undefined register.");
  } // switch ends
  
#ifdef SDMA_DBG
  serial_putstring(dev->deviceName);
  serial_putstring(": load from address ");
  serial_putint(address);
  serial_putstring(" reg ");
  serial_putint_nozeros(regOffs);
  serial_putstring(" value ");
  serial_putint(value);
  serial_newline(); 
#endif

 return value;
}

void storeSdma(device * dev, ACCESS_SIZE size, u32int address, u32int value)
{
  //We care about the real pAddr of the entry, not its vAddr
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);
  u32int regOffs = phyAddr - SDMA;

#ifdef SDMA_DBG
  serial_putstring(dev->deviceName);
  serial_putstring(": store to address ");
  serial_putint(address);
  serial_putstring(" reg ");
  serial_putint_nozeros(regOffs);
  serial_putstring(" value ");
  serial_putint(value);
  serial_newline(); 
#endif
  switch (regOffs)
  {
    default:
      dumpGuestContext(getGuestContext());
      serial_putstring("storeSdma reg ");
      serial_putint_nozeros(regOffs);
      serial_putstring(" value ");
      serial_putint(value);
      serial_newline();
      serial_ERROR("SDMA: store to undefined register.");
  } // switch ends

}
