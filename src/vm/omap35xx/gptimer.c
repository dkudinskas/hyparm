#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "drivers/beagle/beGPTimer.h"
#include "drivers/beagle/beIntc.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/clockManager.h"
#include "vm/omap35xx/gptimer.h"


// base addresses
#define GPTIMER1        0x48318000
#define GPTIMER2        0x49032000
#define GPTIMER3        0x49034000
#define GPTIMER4        0x49036000
#define GPTIMER5        0x49038000
#define GPTIMER6        0x4903A000
#define GPTIMER7        0x4903C000
#define GPTIMER8        0x4903E000
#define GPTIMER9        0x49040000
#define GPTIMER10       0x48086000
#define GPTIMER11       0x48088000
#define GPTIMER12       0x48304000

// register offsets and bit values
#define GPT_REG_TIOCP_CFG      0x010 // CONFIG register
#define GPT_TIOCP_CFG_RESERVED      0xFFFFFCC0
#define GPT_TIOCP_CFG_CLOCKACTIVITY 0x00000300
#define GPT_TIOCP_CFG_EMUFREE       0x00000020
#define GPT_TIOCP_CFG_IDLEMODE      0x00000018
#define GPT_TIOCP_CFG_ENABLEWAKEUP  0x00000004
#define GPT_TIOCP_CFG_SOFTRESET     0x00000002
#define GPT_TIOCP_CFG_AUTOIDLE      0x00000001

#define GPT_REG_TISTAT         0x014 // STATUS register
#define GPT_TISTAT_RESERVED         0xFFFFFFFE
#define GPT_TISTAT_RESETDONE        0x00000001

#define GPT_REG_TISR           0x018 // INTERRUPT status
#define GPT_TISR_RESERVED           0xFFFFFFF8
#define GPT_TISR_CAPTURE            0x00000004
#define GPT_TISR_OVERFLOW           0x00000002
#define GPT_TISR_MATCH              0x00000001

#define GPT_REG_TIER           0x01C // INTERRUPT enable
#define GPT_TIER_RESERVED           0xFFFFFFF8
#define GPT_TIER_CAPTURE            0x00000004
#define GPT_TIER_OVERFLOW           0x00000002
#define GPT_TIER_MATCH              0x00000001

#define GPT_REG_TWER           0x020 // WAKE UP enable
#define GPT_TWER_RESERVED           0xFFFFFFF8
#define GPT_TWER_CAPTURE            0x00000004
#define GPT_TWER_OVERFLOW           0x00000002
#define GPT_TWER_MATCH              0x00000001

#define GPT_REG_TCLR           0x024 // CONTROL
#define GPT_TCLR_RESERVED           0xFFFF8000
#define GPT_TCLR_GPO_CFG            0x00004000
#define GPT_TCLR_CAPT_MODE          0x00002000
#define GPT_TCLR_PULSE_TOGGLE       0x00001000
#define GPT_TCLR_TRIGGER_MODE       0x00000C00
#define GPT_TCLR_TRANS_CAPT         0x00000300
#define GPT_TCLR_PWM                0x00000080
#define GPT_TCLR_COMPARE_ENABLE     0x00000040
#define GPT_TCLR_PRESCALER_ENABLE   0x00000020
#define GPT_TCLR_TRIGGER_OUTPUT     0x0000001C
#define GPT_TCLR_AUTORELOAD         0x00000002 // ONE SHOT mode - stopped after OVF
#define GPT_TCLR_START_STOP         0x00000001

#define GPT_REG_TCRR           0x028 // INTERNAL clock register value
#define GPT_TCRR_COUNTER_VALUE      0xFFFFFFFF

#define GPT_REG_TLDR           0x02C // LOAD register value
#define GPT_TLDR_LOAD_VALUE         0xFFFFFFFF

#define GPT_REG_TTGR           0x030 // TRIGGER value
#define GPT_TTGR_TRIGGER_VALUE      0xFFFFFFFF

#define GPT_REG_TWPS           0x034 // WRITE-Posted pending
#define GPT_TWPS_RESERVED           0xFFFFFC00
#define GPT_TWPS_W_PEND_TOWR        0x00000200
#define GPT_TWPS_W_PEND_TOCR        0x00000100
#define GPT_TWPS_W_PEND_TCVR        0x00000080
#define GPT_TWPS_W_PEND_TNIR        0x00000040
#define GPT_TWPS_W_PEND_TPIR        0x00000020
#define GPT_TWPS_W_PEND_TMAR        0x00000010
#define GPT_TWPS_W_PEND_TTGR        0x00000008
#define GPT_TWPS_W_PEND_TLDR        0x00000004
#define GPT_TWPS_W_PEND_TCRR        0x00000002
#define GPT_TWPS_W_PEND_TCLR        0x00000001

#define GPT_REG_TMAR           0x038 // MATCH register value
#define GPT_TMAR_MATCH_VALUE        0xFFFFFFFF

#define GPT_REG_TCAR1          0x03C // FIRST captured counter
#define GPT_TCAR1_CAPTURE_VALUE     0xFFFFFFFF

#define GPT_REG_TSICR          0x040 // INTERFACE control
#define GPT_TSICR_RESERVED          0xFFFFFFF9
#define GPT_TSICR_POSTED            0x00000004
#define GPT_TSICR_SFTRESET          0x00000002

#define GPT_REG_TCAR2          0x044 // SECOND captured counter
#define GPT_TCAR2_CAPTURE_VALUE     0xFFFFFFFF

#define GPT_REG_TPIR           0x048 // POSITIVE Increment value (TCVR+TPIR)
#define GPT_TPIR_POS_INC            0xFFFFFFFF

#define GPT_REG_TNIR           0x04C // NEGATIVE Increment value (TCVR-TPIC)
#define GPT_TNIR_NEG_INC            0xFFFFFFFF

#define GPT_REG_TCVR           0x050 // CLOCK value register
#define GPT_TCVR_CLOCK              0xFFFFFFFF

#define GPT_REG_TOCR           0x054 // MASK tick irq for a selected nr of ticks
#define GPT_TOCR_RESERVED           0xFF000000
#define GPT_TOCR_OVF_COUNTER        0x00FFFFFF

#define GPT_REG_TOWR           0x058 // NUMBER Of masked overflow interrupt events
#define GPT_TOWR_RESERVED           0xFF000000
#define GPT_TOWR_OVF_WRAPPING       0x00FFFFFF


/* values for 1 milisecond clock generation */
#define TPIR1MS   232000
#define TNIR1MS  -768000
#define TLDR1MS  0xFFFFFFE0


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
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load config register value %#.8x" EOL, dev->deviceName, val);
      break;
    case GPT_REG_TISTAT:
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load status register value %#.8x" EOL, dev->deviceName, val);
      break;
    case GPT_REG_TISR:
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load irq status register value %#.8x" EOL, dev->deviceName,
          val);
      break;
    case GPT_REG_TIER:
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load irq enable register value %#.8x" EOL,
          dev->deviceName, val);
      break;
    case GPT_REG_TWER:
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load wakeup enable register value %#.8x" EOL,
          dev->deviceName, val);
      break;
    case GPT_REG_TCLR:
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load conrol register value %#.8x" EOL, dev->deviceName, val);
      break;
    case GPT_REG_TCRR:
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load internal clock register value %#.8x" EOL,
          dev->deviceName, val);
      break;
    case GPT_REG_TLDR:
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load counter reload value %#.8x" EOL, dev->deviceName, val);
      break;
    case GPT_REG_TTGR:
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load trigger register value %#.8x" EOL, dev->deviceName,
          val);
      break;
    case GPT_REG_TWPS:
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load write-posted pending status register value %#.8x" EOL,
          dev->deviceName, val);
      break;
    case GPT_REG_TMAR:
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load match register value %#.8x" EOL, dev->deviceName, val);
      break;
    case GPT_REG_TCAR1:
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load first captured counter value %#.8x" EOL,
          dev->deviceName, val);
      break;
    case GPT_REG_TSICR:
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load interface control register value %#.8x" EOL,
          dev->deviceName, val);
      break;
    case GPT_REG_TCAR2:
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load second captured counter value %#.8x" EOL,
          dev->deviceName, val);
      break;
    case GPT_REG_TPIR:
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load positive increment value %#.8x" EOL, dev->deviceName,
          val);
      break;
    case GPT_REG_TNIR:
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load negative increment value %#.8x" EOL, dev->deviceName,
          val);
      break;
    case GPT_REG_TCVR:
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load counter pir/nir selection register value %#.8x" EOL,
          dev->deviceName, val);
      break;
    case GPT_REG_TOCR:
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load overflow masking register value %#.8x" EOL,
          dev->deviceName, val);
      break;
    case GPT_REG_TOWR:
      val = loadFromGPTimer(2, regOffs);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: load number of masked overflows value %#.8x" EOL,
          dev->deviceName, val);
      break;
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
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to config register value %#.8x" EOL,
          dev->deviceName, value);
      break;
    case GPT_REG_TISTAT:
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to status register value %#.8x" EOL,
          dev->deviceName, value);
      break;
    case GPT_REG_TISR:
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to irq status register value %#.8x" EOL,
          dev->deviceName, value);
      break;
    case GPT_REG_TIER:
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to irq enable register value %#.8x" EOL,
          dev->deviceName, value);
      break;
    case GPT_REG_TWER:
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to wakeup enable register value %#.8x" EOL,
          dev->deviceName, value);
      break;
    case GPT_REG_TCLR:
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to conrol register value %#.8x" EOL, dev->deviceName,
          value);
      break;
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
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to trigger register value %#.8x" EOL, dev->deviceName,
          value);
      break;
    case GPT_REG_TWPS:
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to write-posted pending status register value %#.8x"
          EOL, dev->deviceName, value);
      break;
    case GPT_REG_TMAR:
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
    case GPT_REG_TCAR1:
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to first captured counter value %#.8x" EOL,
          dev->deviceName, value);
      break;
    case GPT_REG_TSICR:
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to interface control register value %#.8x" EOL,
          dev->deviceName, value);
      break;
    case GPT_REG_TCAR2:
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to second captured counter value %#.8x" EOL,
          dev->deviceName, value);
      break;
    case GPT_REG_TPIR:
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to positive increment value %#.8x" EOL,
          dev->deviceName, value);
      break;
    case GPT_REG_TNIR:
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to negative increment value %#.8x" EOL,
          dev->deviceName, value);
      break;
    case GPT_REG_TCVR:
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to counter pir/nir selection register value %#.8x" EOL,
          dev->deviceName, value);
      break;
    case GPT_REG_TOCR:
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to overflow masking register value %#.8x" EOL,
          dev->deviceName, value);
      break;
    case GPT_REG_TOWR:
      storeToGPTimer(2, regOffs, value);
      DEBUG(VP_OMAP_35XX_GPTIMER, "%s: store to number of masked overflows value %#.8x" EOL,
          dev->deviceName, value);
      break;
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  } // switch ends
}

