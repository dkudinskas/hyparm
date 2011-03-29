#include "common/debug.h"
#include "common/memFunctions.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/sdma.h"
#include "vm/omap35xx/serial.h"

#include "memoryManager/pageTable.h" // for getPhysicalAddress()


extern GCONTXT * getGuestContext(void);
struct Sdma * sdma;

void initSdma()
{
  // init function: setup device, reset register values to defaults!
  sdma = (struct Sdma*)mallocBytes(sizeof(struct Sdma));
  if (sdma == 0)
  {
    DIE_NOW(0, "Failed to allocate sdma.");
  }
  else
  {
    memset((void*)sdma, 0x0, sizeof(struct Sdma));
#ifdef SDMA_DBG
    serial_putstring("Initializing Sdma at 0x");
    serial_putint((u32int)sdma);
    serial_putstring(" size ");
    serial_putint(sizeof(struct Sdma));
    serial_newline();
#endif
  }

  resetSdma();
}

void resetSdma()
{
  sdma->irqStatusL0      = 0x00000000;
  sdma->irqStatusL1      = 0x00000000;
  sdma->irqStatusL2      = 0x00000000;
  sdma->irqStatusL3      = 0x00000000;
  sdma->irqEnableL0      = 0x00000000;
  sdma->irqEnableL1      = 0x00000000;
  sdma->irqEnableL2      = 0x00000000;
  sdma->irqEnableL3      = 0x00000000;
  sdma->sysStatus        = 0x00000001;
  sdma->ocpSysConfig     = 0x00000000;
  sdma->caps0            = 0x000C0000;
  sdma->caps2            = 0x000001ff;
  sdma->caps3            = 0x000000f3;
  sdma->caps4            = 0x00003ffe;
  sdma->gcr              = 0x00010010;
  // init indexed registers
  int i = 0;
  for (i = 0; i < 32; i++)
  {
    sdma->chIndexedRegs[i].ccr       = 0x00000000;
    sdma->chIndexedRegs[i].clnkCtrl  = 0x00000000;
    sdma->chIndexedRegs[i].cicr      = 0x00000600;
    sdma->chIndexedRegs[i].csr       = 0x00000000;
    sdma->chIndexedRegs[i].csdp      = 0x00000000;
    sdma->chIndexedRegs[i].cen       = 0x00000000;
    sdma->chIndexedRegs[i].cfn       = 0x00000000;
    sdma->chIndexedRegs[i].cssa      = 0x00000000;
    sdma->chIndexedRegs[i].cdsa      = 0x00000000;
    sdma->chIndexedRegs[i].csei      = 0x00000000;
    sdma->chIndexedRegs[i].csfi      = 0x00000000;
    sdma->chIndexedRegs[i].cdei      = 0x00000000;
    sdma->chIndexedRegs[i].cdfi      = 0x00000000;
    sdma->chIndexedRegs[i].csac      = 0x00000000;
    sdma->chIndexedRegs[i].cdac      = 0x00000000;
    sdma->chIndexedRegs[i].ccen      = 0x00000000;
    sdma->chIndexedRegs[i].ccfn      = 0x00000000;
    sdma->chIndexedRegs[i].color     = 0x00000000;
  }
}


u32int loadSdma(device * dev, ACCESS_SIZE size, u32int address)
{
  //We care about the real pAddr of the entry, not its vAddr
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);
  u32int value = 0;
  u32int regOffs = phyAddr - SDMA;
  bool found = FALSE;
  switch (regOffs)
  {
    case SDMA_REVISION:
      value = SDMA_REVISION_NUMBER;
      found = TRUE;
      break;
    case SDMA_IRQSTATUS_L0:
    case SDMA_IRQSTATUS_L1:
    case SDMA_IRQSTATUS_L2:
    case SDMA_IRQSTATUS_L3:
    case SDMA_IRQENABLE_L0:
    case SDMA_IRQENABLE_L1:
    case SDMA_IRQENABLE_L2:
    case SDMA_IRQENABLE_L3:
    case SDMA_SYSSTATUS:
    case SDMA_OCP_SYSCONFIG:
    case SDMA_CAPS_0:
    case SDMA_CAPS_2:
    case SDMA_CAPS_3:
    case SDMA_CAPS_4:
    case SDMA_GCR:
      dumpGuestContext(getGuestContext());
      serial_putstring("loadSdma reg ");
      serial_putint_nozeros(regOffs);
      serial_newline();
      DIE_NOW(0, "SDMA: load from unimplemented register.");
      break;
  } // switch ends

  if (found)
  {
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

  // if we didnt hit the previous switch, maybe its one of the indexed registers
  u32int indexedRegOffs = (regOffs - 0x80) % 0x60;
  switch (indexedRegOffs+0x80)
  {
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
      dumpGuestContext(getGuestContext());
      serial_putstring("loadSdma indexed reg ");
      serial_putint(indexedRegOffs+0x80);
      serial_putstring(" reg ");
      serial_putint(regOffs);
      serial_newline();
      DIE_NOW(0, "SDMA: load from unimplemented register.");
      break;
    default:
      dumpGuestContext(getGuestContext());
      serial_putstring("loadSdma indexed reg ");
      serial_putint(indexedRegOffs+0x80);
      serial_putstring(" reg ");
      serial_putint(regOffs);
      serial_newline();
      DIE_NOW(0, "SDMA: load from undefined register.");
  }
}


void storeSdma(device * dev, ACCESS_SIZE size, u32int address, u32int value)
{
  //We care about the real pAddr of the entry, not its vAddr
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);
  u32int regOffs = phyAddr - SDMA;
  bool found = FALSE;

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
    case SDMA_REVISION:
      DIE_NOW(0, "SDMA storing to revision register (read only)");
      break;
    case SDMA_GCR:
      if (sdma->gcr != value)
      {
        DIE_NOW(0, "SDMA storing value to GCR!");
      }
      sdma->gcr = value;
      found = TRUE;
      break;
    case SDMA_IRQSTATUS_L0:
    case SDMA_IRQSTATUS_L1:
    case SDMA_IRQSTATUS_L2:
    case SDMA_IRQSTATUS_L3:
    case SDMA_IRQENABLE_L0:
    case SDMA_IRQENABLE_L1:
    case SDMA_IRQENABLE_L2:
    case SDMA_IRQENABLE_L3:
    case SDMA_SYSSTATUS:
    case SDMA_OCP_SYSCONFIG:
    case SDMA_CAPS_0:
    case SDMA_CAPS_2:
    case SDMA_CAPS_3:
    case SDMA_CAPS_4:
      dumpGuestContext(getGuestContext());
      serial_putstring("storeSdma reg ");
      serial_putint_nozeros(regOffs);
      serial_putstring(" value ");
      serial_putint(value);
      serial_newline();
      DIE_NOW(0, "SDMA: store to unimplemented register.");
      break;
  } // switch ends

  if (found)
  {
    return;
  }

  // if we didnt hit the previous switch, maybe its one of the indexed registers
  u32int channelIndex = getChannelNumber(regOffs);
  u32int indexedRegOffs = (regOffs - 0x80) % 0x60;
  switch (indexedRegOffs+0x80)
  {
    case SDMA_CCRi:
      if ((value & SDMA_CCRi_ENABLE) == SDMA_CCRi_ENABLE)
      {
        serial_putstring(dev->deviceName);
        serial_putstring(": Warning - enabling channel ");
        serial_putint(channelIndex);
        serial_newline();
      }
      sdma->chIndexedRegs[channelIndex].ccr = value;
      break;
    case SDMA_CLNK_CTRLi:
      sdma->chIndexedRegs[channelIndex].clnkCtrl = value;
      break;
    case SDMA_CICRi:
      sdma->chIndexedRegs[channelIndex].cicr = value;
      break;
    case SDMA_CSRi:
      sdma->chIndexedRegs[channelIndex].csr = value;
      break;
    case SDMA_CSDPi:
      sdma->chIndexedRegs[channelIndex].csdp = value;
      break;
    case SDMA_CENi:
      sdma->chIndexedRegs[channelIndex].cen = value;
      break;
    case SDMA_CFNi:
      sdma->chIndexedRegs[channelIndex].cfn = value;
      break;
    case SDMA_CSSAi:
      sdma->chIndexedRegs[channelIndex].cssa = value;
      break;
    case SDMA_CDSAi:
      sdma->chIndexedRegs[channelIndex].cdsa = value;
      break;
    case SDMA_CSEIi:
      sdma->chIndexedRegs[channelIndex].csei = value;
      break;
    case SDMA_CSFIi:
      sdma->chIndexedRegs[channelIndex].csfi = value;
      break;
    case SDMA_CDEIi:
      sdma->chIndexedRegs[channelIndex].cdei = value;
      break;
    case SDMA_CDFIi:
      sdma->chIndexedRegs[channelIndex].cdfi = value;
      break;
    case SDMA_CSACi:
#ifdef SDMA_DBG
      serial_putstring("SDMA: WARNING: store to R/O register SDMA_CSACi");
      serial_newline();
#endif
      break; 
    case SDMA_CDACi:
      sdma->chIndexedRegs[channelIndex].cdac = value;
      break;
    case SDMA_CCENi:
#ifdef SDMA_DBG
      serial_putstring("SDMA: WARNING: store to R/O register SDMA_CCENi");
      serial_newline();
#endif
      break; 
    case SDMA_CCFNi:
#ifdef SDMA_DBG
      serial_putstring("SDMA: WARNING: store to R/O register SDMA_CCFNi");
      serial_newline();
#endif
      break; 
    case SDMA_COLORi:
      sdma->chIndexedRegs[channelIndex].color = value;
      break;
    default:
      dumpGuestContext(getGuestContext());
      serial_putstring("storeSdma indexed reg ");
      serial_putint_nozeros(indexedRegOffs+0x80);
      serial_putstring(" reg ");
      serial_putint(regOffs);
      serial_putstring(" value ");
      serial_putint(value);
      serial_newline();
      DIE_NOW(0, "SDMA: store to undefined register.");
  }
}

inline u32int getChannelNumber(u32int regOffs)
{
  return ((regOffs - 0x80) / 0x60);
}