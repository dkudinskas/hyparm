#include "gptimer.h"
#include "clockManager.h"
#include "memoryConstants.h" // for BEAGLE_RAM_START/END
#include "pageTable.h" // for getPhysicalAddress()
#include "guestContext.h"
#include "memFunctions.h"
#include "beIntc.h"
#include "beGPTimer.h"

extern GCONTXT * getGuestContext(void);

struct GeneralPurposeTimer * gptimer;

void initGPTimer()
{
  // init function: setup device, reset register values to defaults!
  gptimer = (struct GeneralPurposeTimer*)mallocBytes(sizeof(struct GeneralPurposeTimer));
  if (gptimer == 0)
  {
    serial_ERROR("Failed to allocate general purpose timer.");
  }
  else
  {
    memset((void*)gptimer, 0x0, sizeof(struct GeneralPurposeTimer));
#ifdef GPTIMER_DBG
    serial_putstring("Initializing General Purpose timer at 0x");
    serial_putint((u32int)gptimer);
    serial_newline();
#endif
  }

  resetGPTimer();
}

void resetGPTimer(void)
{
  // reset to default register values
  gptimer->gptTiocpCfg = 0x00000000;
  gptimer->gptTistat   = 0x00000000;
  gptimer->gptTisr     = 0x00000000;
  gptimer->gptTier     = 0x00000000;
  gptimer->gptTwer     = 0x00000000;
  gptimer->gptTclr     = 0x00000000;
  gptimer->gptTcrr     = 0x00000000;
  gptimer->gptTldr     = 0x00000000;
  gptimer->gptTtgr     = 0xffffffff;
  gptimer->gptTwps     = 0x00000000;
  gptimer->gptTmar     = 0x00000000;
  gptimer->gptTcar1    = 0x00000000;
  gptimer->gptTsicr    = 0x00000004;
  gptimer->gptTcar2    = 0x00000000;
  gptimer->gptTpir     = 0x00000000;
  gptimer->gptTnir     = 0x00000000;
  gptimer->gptTcvr     = 0x00000000;
  gptimer->gptTocr     = 0x00000000;
  gptimer->gptTowr     = 0x00000000;
  gptimer->shadowValue = 0x00000000;
}


u32int loadGPTimer(device * dev, ACCESS_SIZE size, u32int address)
{
  if (size == BYTE)
  {
    serial_ERROR("GPT: loadGPTimer invalid access size - byte");
  }

  //We care about the real pAddr of the entry, not its vAddr
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

  u32int val = 0;
  
  u32int regOffs = phyAddr - GPTIMER1;

  switch (regOffs)
  {
    case GPT_REG_TIOCP_CFG:
      val = loadFromGPTimer(2, regOffs);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": load from config register value 0x");
      serial_putint(val);
      serial_newline();
#endif
      break;
    case GPT_REG_TISTAT:
      val = loadFromGPTimer(2, regOffs);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": load from status register value 0x");
      serial_putint(val);
      serial_newline();
#endif
      break;
    case GPT_REG_TISR:
      val = loadFromGPTimer(2, regOffs);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": load from irq status register value 0x");
      serial_putint(val);
      serial_newline();
#endif
      break;
    case GPT_REG_TIER:
      val = loadFromGPTimer(2, regOffs);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": load from irq enable register value 0x");
      serial_putint(val);
      serial_newline();
#endif
      break;
    case GPT_REG_TWER:
      val = loadFromGPTimer(2, regOffs);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": load from wakeup enable register value 0x");
      serial_putint(val);
      serial_newline();
#endif
      break;
    case GPT_REG_TCLR:
      val = loadFromGPTimer(2, regOffs);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": load from conrol register value 0x");
      serial_putint(val);
      serial_newline();
#endif
      break;
    case GPT_REG_TCRR:
      val = loadFromGPTimer(2, regOffs);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": load internal clock register value 0x");
      serial_putint(val);
      serial_newline();
#endif
      break;
    case GPT_REG_TLDR:
      val = loadFromGPTimer(2, regOffs);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": load counter reload value 0x");
      serial_putint(val);
      serial_newline();
#endif
      break;
    case GPT_REG_TTGR:
      val = loadFromGPTimer(2, regOffs);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": load trigger register value 0x");
      serial_putint(val);
      serial_newline();
#endif
      break;
    case GPT_REG_TWPS:
      val = loadFromGPTimer(2, regOffs);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": load from write-posted pending status register value 0x");
      serial_putint(val);
      serial_newline();
#endif
      break;
    case GPT_REG_TMAR:
      val = loadFromGPTimer(2, regOffs);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": load match register value 0x");
      serial_putint(val);
      serial_newline();
#endif
      break;
    case GPT_REG_TCAR1:
      val = loadFromGPTimer(2, regOffs);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": load first captured counter value 0x");
      serial_putint(val);
      serial_newline();
#endif
      break;
    case GPT_REG_TSICR:
      val = loadFromGPTimer(2, regOffs);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": load interface control register value 0x");
      serial_putint(val);
      serial_newline();
#endif
      break;
    case GPT_REG_TCAR2:
      val = loadFromGPTimer(2, regOffs);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": load second captured counter value 0x");
      serial_putint(val);
      serial_newline();
#endif
      break;
    case GPT_REG_TPIR:
      val = loadFromGPTimer(2, regOffs);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": load positive increment value 0x");
      serial_putint(val);
      serial_newline();
#endif
      break;
    case GPT_REG_TNIR:
      val = loadFromGPTimer(2, regOffs);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": load negative increment value 0x");
      serial_putint(val);
      serial_newline();
#endif
      break;
    case GPT_REG_TCVR:
      val = loadFromGPTimer(2, regOffs);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": load counter pir/nir selection register value 0x");
      serial_putint(val);
      serial_newline();
#endif
      break;
    case GPT_REG_TOCR:
      val = loadFromGPTimer(2, regOffs);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": load overflow masking register value 0x");
      serial_putint(val);
      serial_newline();
#endif
      break;
    case GPT_REG_TOWR:
      val = loadFromGPTimer(2, regOffs);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": load number of masked overflows, value 0x");
      serial_putint(val);
      serial_newline();
#endif
      break;
    default:
      serial_ERROR("GPT: load from undefined register.");
  } // switch ends
  
 return val;
}

void storeGPTimer(device * dev, ACCESS_SIZE size, u32int address, u32int value)
{
  if (size == BYTE)
  {
    serial_ERROR("GPT: storeGPTimer invalid access size - byte");
  }

  //We care about the real pAddr of the entry, not its vAddr
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

  u32int regOffs = phyAddr - GPTIMER1;

  switch (regOffs)
  {
    case GPT_REG_TIOCP_CFG:
      storeToGPTimer(2, regOffs, value);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": store to config register value 0x");
      serial_putint(value);
      serial_newline();
#endif
      break;
    case GPT_REG_TISTAT:
      storeToGPTimer(2, regOffs, value);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": store to status register value 0x");
      serial_putint(value);
      serial_newline();
#endif
      break;
    case GPT_REG_TISR:
      storeToGPTimer(2, regOffs, value);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": store to irq status register value 0x");
      serial_putint(value);
      serial_newline();
#endif
      break;
    case GPT_REG_TIER:
      storeToGPTimer(2, regOffs, value);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": store to irq enable register value 0x");
      serial_putint(value);
      serial_newline();
#endif
      break;
    case GPT_REG_TWER:
      storeToGPTimer(2, regOffs, value);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": store to wakeup enable register value 0x");
      serial_putint(value);
      serial_newline();
#endif
      break;
    case GPT_REG_TCLR:
      storeToGPTimer(2, regOffs, value);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": store to conrol register value 0x");
      serial_putint(value);
      serial_newline();
#endif
      break;
    case GPT_REG_TCRR:
    {
      // make sure we dont throw irq's too often...
      u32int adjustedValue = value << 12;
      storeToGPTimer(2, regOffs, adjustedValue);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": store to internal clock register value 0x");
      serial_putint(value);
      serial_newline();
#endif
      break;
    }
    case GPT_REG_TLDR:
    {
      // make sure we dont throw irq's too often...
      u32int adjustedValue = value << 12;
      storeToGPTimer(2, regOffs, adjustedValue);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": store to counter reload value 0x");
      serial_putint(value);
      serial_newline();
#endif
      break;
    }
    case GPT_REG_TTGR:
      storeToGPTimer(2, regOffs, value);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": store to trigger register value 0x");
      serial_putint(value);
      serial_newline();
#endif
      break;
    case GPT_REG_TWPS:
      storeToGPTimer(2, regOffs, value);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": store to write-posted pending status register value 0x");
      serial_putint(value);
      serial_newline();
#endif
      break;
    case GPT_REG_TMAR:
      storeToGPTimer(2, regOffs, value);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": store to match register value 0x");
      serial_putint(value);
      serial_newline();
#endif
      break;
    case GPT_REG_TCAR1:
      storeToGPTimer(2, regOffs, value);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": store to first captured counter value 0x");
      serial_putint(value);
      serial_newline();
#endif
      break;
    case GPT_REG_TSICR:
      storeToGPTimer(2, regOffs, value);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": store to interface control register value 0x");
      serial_putint(value);
      serial_newline();
#endif
      break;
    case GPT_REG_TCAR2:
      storeToGPTimer(2, regOffs, value);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": store to second captured counter value 0x");
      serial_putint(value);
      serial_newline();
#endif
      break;
    case GPT_REG_TPIR:
      storeToGPTimer(2, regOffs, value);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": store to positive increment value 0x");
      serial_putint(value);
      serial_newline();
#endif
      break;
    case GPT_REG_TNIR:
      storeToGPTimer(2, regOffs, value);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": store to negative increment value 0x");
      serial_putint(value);
      serial_newline();
#endif
      break;
    case GPT_REG_TCVR:
      storeToGPTimer(2, regOffs, value);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": store to counter pir/nir selection register value 0x");
      serial_putint(value);
      serial_newline();
#endif
      break;
    case GPT_REG_TOCR:
      storeToGPTimer(2, regOffs, value);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": store to overflow masking register value 0x");
      serial_putint(value);
      serial_newline();
#endif
      break;
    case GPT_REG_TOWR:
      storeToGPTimer(2, regOffs, value);
#ifdef GPTIMER_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(": store to number of masked overflows, value 0x");
      serial_putint(value);
      serial_newline();
#endif
      break;
    default:
      serial_ERROR("GPT: store to undefined register.");
  } // switch ends

}

