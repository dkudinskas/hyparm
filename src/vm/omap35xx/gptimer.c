#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "drivers/beagle/beGPTimer.h"
#include "drivers/beagle/beIntc.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/clockManager.h"
#include "vm/omap35xx/gptimer.h"
#include "vm/omap35xx/gptimerInternals.h"


static void resetGPTimer(struct GeneralPurposeTimer *gptimer);


void initGPTimer(virtualMachine *vm)
{
  struct GeneralPurposeTimer *gptimer = (struct GeneralPurposeTimer *)calloc(1, sizeof(struct GeneralPurposeTimer));
  if (gptimer == NULL)
  {
    DIE_NOW(NULL, "Failed to allocate general purpose timer.");
  }
  vm->gptimer = gptimer;

  DEBUG(VP_OMAP_35XX_GPTIMER, "Initializing General Purpose timer at %p" EOL, gptimer);
  resetGPTimer(gptimer);
}

void resetGPTimer(struct GeneralPurposeTimer *gptimer)
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


u32int loadGPTimer(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  if (size == BYTE)
  {
    DIE_NOW(NULL, "GPT: loadGPTimer invalid access size - byte");
  }

  u32int val = 0;
  u32int regOffs = phyAddr - GPTIMER1;
  switch (regOffs)
  {
    case GPT_REG_TIOCP_CFG:
    {
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load config register value %#.8x" EOL, dev->deviceName, val);
      break;
    }
    case GPT_REG_TISTAT:
    {
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load status register value %#.8x" EOL, dev->deviceName, val);
      break;
    }
    case GPT_REG_TISR:
    {
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load irq status register value %#.8x" EOL, dev->deviceName,
          val);
      break;
    }
    case GPT_REG_TIER:
    {
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load irq enable register value %#.8x" EOL,
          dev->deviceName, val);
      break;
    }
    case GPT_REG_TWER:
    {
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load wakeup enable register value %#.8x" EOL,
          dev->deviceName, val);
      break;
    }
    case GPT_REG_TCLR:
    {
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load conrol register value %#.8x" EOL, dev->deviceName, val);
      break;
    }
    case GPT_REG_TCRR:
    {
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load internal clock register value %#.8x" EOL,
          dev->deviceName, val);
      break;
    }
    case GPT_REG_TLDR:
    {
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load counter reload value %#.8x" EOL, dev->deviceName, val);
      break;
    }
    case GPT_REG_TTGR:
    {
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load trigger register value %#.8x" EOL, dev->deviceName,
          val);
      break;
    }
    case GPT_REG_TWPS:
    {
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load write-posted pending status register value %#.8x" EOL,
          dev->deviceName, val);
      break;
    }
    case GPT_REG_TMAR:
    {
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load match register value %#.8x" EOL, dev->deviceName, val);
      break;
    }
    case GPT_REG_TCAR1:
    {
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load first captured counter value %#.8x" EOL,
          dev->deviceName, val);
      break;
    }
    case GPT_REG_TSICR:
    {
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load interface control register value %#.8x" EOL,
          dev->deviceName, val);
      break;
    }
    case GPT_REG_TCAR2:
    {
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load second captured counter value %#.8x" EOL,
          dev->deviceName, val);
      break;
    }
    case GPT_REG_TPIR:
    {
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load positive increment value %#.8x" EOL, dev->deviceName,
          val);
      break;
    }
    case GPT_REG_TNIR:
    {
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load negative increment value %#.8x" EOL, dev->deviceName,
          val);
      break;
    }
    case GPT_REG_TCVR:
    {
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load counter pir/nir selection register value %#.8x" EOL,
          dev->deviceName, val);
      break;
    }
    case GPT_REG_TOCR:
    {
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load overflow masking register value %#.8x" EOL,
          dev->deviceName, val);
      break;
    }
    case GPT_REG_TOWR:
    {
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load number of masked overflows value %#.8x" EOL,
          dev->deviceName, val);
      break;
    }
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  } // switch ends

 return val;
}


void storeGPTimer(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  if (size == BYTE)
  {
    DIE_NOW(NULL, "GPT: storeGPTimer invalid access size - byte");
  }

  u32int regOffs = phyAddr - GPTIMER1;

  switch (regOffs)
  {
    case GPT_REG_TIOCP_CFG:
    {
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to config register value %#.8x" EOL,
          dev->deviceName, value);
      break;
    }
    case GPT_REG_TISTAT:
    {
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to status register value %#.8x" EOL,
          dev->deviceName, value);
      break;
    }
    case GPT_REG_TISR:
    {
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to irq status register value %#.8x" EOL,
          dev->deviceName, value);
      break;
    }
    case GPT_REG_TIER:
    {
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to irq enable register value %#.8x" EOL,
          dev->deviceName, value);
      break;
    }
    case GPT_REG_TWER:
    {
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to wakeup enable register value %#.8x" EOL,
          dev->deviceName, value);
      break;
    }
    case GPT_REG_TCLR:
    {
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to conrol register value %#.8x" EOL, dev->deviceName,
          value);
      break;
    }
    case GPT_REG_TCRR:
    {
      // make sure we dont throw irq's too often...
      u32int adjustedValue = value << 12;
      storeToGPTimer(2, regOffs, adjustedValue);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to internal clock register value %#.8x" EOL,
          dev->deviceName, value);
      break;
    }
    case GPT_REG_TLDR:
    {
      // make sure we dont throw irq's too often...
      u32int adjustedValue = value << 12;
      storeToGPTimer(2, regOffs, adjustedValue);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to counter reload value %#.8x" EOL, dev->deviceName,
          value);
      break;
    }
    case GPT_REG_TTGR:
    {
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to trigger register value %#.8x" EOL, dev->deviceName,
          value);
      break;
    }
    case GPT_REG_TWPS:
    {
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to write-posted pending status register value %#.8x"
          EOL, dev->deviceName, value);
      break;
    }
    case GPT_REG_TMAR:
    {
#ifdef CONFIG_GUEST_FREERTOS
      if (context->os == GUEST_OS_FREERTOS)
      {
        /*
         * FIXME: Use a higher TMAR value to make sure that Guest is ready to accept interrupts.
         * This needs some fixing but should work for now. FreeRTOS set the TMAR value to
         * 0x6590. This is way too low for hypervisor to prepare guest CPSR.
         */
        value = value << 3;
      }
#endif
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to match register value %#.8x" EOL,
          dev->deviceName, value);
      break;
    }
    case GPT_REG_TCAR1:
    {
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to first captured counter value %#.8x" EOL,
          dev->deviceName, value);
      break;
    }
    case GPT_REG_TSICR:
    {
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to interface control register value %#.8x" EOL,
          dev->deviceName, value);
      break;
    }
    case GPT_REG_TCAR2:
    {
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to second captured counter value %#.8x" EOL,
          dev->deviceName, value);
      break;
    }
    case GPT_REG_TPIR:
    {
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to positive increment value %#.8x" EOL,
          dev->deviceName, value);
      break;
    }
    case GPT_REG_TNIR:
    {
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to negative increment value %#.8x" EOL,
          dev->deviceName, value);
      break;
    }
    case GPT_REG_TCVR:
    {
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to counter pir/nir selection register value %#.8x" EOL,
          dev->deviceName, value);
      break;
    }
    case GPT_REG_TOCR:
    {
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to overflow masking register value %#.8x" EOL,
          dev->deviceName, value);
      break;
    }
    case GPT_REG_TOWR:
    {
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to number of masked overflows value %#.8x" EOL,
          dev->deviceName, value);
      break;
    }
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  } // switch ends
}

