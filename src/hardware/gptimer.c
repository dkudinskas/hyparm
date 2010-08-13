#include "gptimer.h"
#include "clockManager.h"
#include "memoryConstants.h" // for BEAGLE_RAM_START/END
#include "pageTable.h" // for getPhysicalAddress()
#include "guestContext.h"
#include "memFunctions.h"
#include "beIntc.h"

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
      val = gptimer->gptTiocpCfg & ~GPT_TIOCP_CFG_RESERVED;
      break;
    case GPT_REG_TSICR:
      val = gptimer->gptTsicr & GPT_TSICR_POSTED; // all other bits dont care
      break;
    case GPT_REG_TWPS:
      val = gptimer->gptTwps & GPT_TWPS_RESERVED;
      break;
    case GPT_REG_TIER:
      val = gptimer->gptTier & GPT_TIER_RESERVED;
      break;
    case GPT_REG_TWER:
      val = gptimer->gptTwer & GPT_TWER_RESERVED;
      break;
    case GPT_REG_TOCR:
      val = gptimer->gptTocr & GPT_TOCR_RESERVED;
      break;
    case GPT_REG_TCLR:
      val = gptimer->gptTclr & GPT_TCLR_RESERVED;
      break;
    case GPT_REG_TCRR:
      val = gptimer->gptTcrr;
      break;
    case GPT_REG_TLDR:
      val = gptimer->gptTldr;
      break;
    case GPT_REG_TISTAT:
    case GPT_REG_TISR:
    case GPT_REG_TTGR:
    case GPT_REG_TMAR:
    case GPT_REG_TCAR1:
    case GPT_REG_TCAR2:
    case GPT_REG_TPIR:
    case GPT_REG_TNIR:
    case GPT_REG_TCVR:
    case GPT_REG_TOWR:
      serial_putstring(dev->deviceName);
      serial_putstring(" load from pAddr: 0x");
      serial_putint(phyAddr);
      serial_putstring(", vAddr: 0x");
      serial_putint(address);
      serial_putstring(" access size ");
      serial_putint((u32int)size);
      serial_putstring(" val ");
      serial_putint(val);
      serial_newline();
      serial_ERROR("GPT: unimplemented register load.");
      break;
    default:
      serial_ERROR("GPT: load from undefined register.");
  } // switch ends  


#ifdef GPTIMER_DBG
  serial_putstring(dev->deviceName);
  serial_putstring(" load from pAddr: 0x");
  serial_putint(phyAddr);
  serial_putstring(", vAddr: 0x");
  serial_putint(address);
  serial_putstring(" access size ");
  serial_putint((u32int)size);
  serial_putstring(" val ");
  serial_putint(val);
  serial_newline();
#endif

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

#ifdef GPTIMER_DBG
  serial_putstring(dev->deviceName);
  serial_putstring(" store to pAddr: 0x");
  serial_putint(phyAddr);
  serial_putstring(", vAddr: 0x");
  serial_putint(address);
  serial_putstring(" aSize ");
  serial_putint((u32int)size);
  serial_putstring(" val ");
  serial_putint(value);
  serial_newline();
#endif
  
  u32int regOffs = phyAddr - GPTIMER1;
  switch (regOffs)
  {
    case GPT_REG_TIOCP_CFG:
      if (value & GPT_TIOCP_CFG_SOFTRESET)
      {
#ifdef GPTIMER_DBG
        serial_putstring("GPTIMER soft reset");
        serial_newline();
#endif
      }
      else
      {
        gptimer->gptTiocpCfg = value & ~GPT_TIOCP_CFG_RESERVED;
      }
      break;
    case GPT_REG_TSICR:
      if (value & GPT_TSICR_SFTRESET)
      {
        serial_ERROR("GPTIMER: reset software functional register. which??");
      }
      else
      {
        gptimer->gptTsicr = value & GPT_TSICR_POSTED; // all other bits dont care
      }
      break;
    case GPT_REG_TWPS:
      serial_ERROR("GPTimer writing to a R/O register (TWPS)");
      break;
    case GPT_REG_TIER:
      value = value & ~GPT_TIER_RESERVED;
      if (value != gptimer->gptTier)
      {
        if ( ((gptimer->gptTier & GPT_TIER_CAPTURE) == GPT_TIER_CAPTURE) &&
             ((value & GPT_TIER_CAPTURE) == 0) )
        {
          serial_putstring("GPTimer: disabling capture interrupt.");
          serial_newline();
        }
        else if ( ((gptimer->gptTier & GPT_TIER_CAPTURE) == 0) &&
             ((value & GPT_TIER_CAPTURE) == GPT_TIER_CAPTURE) )
        {
          serial_putstring("GPTimer: enabling capture interrupt.");
          serial_newline();
        }
        if ( ((gptimer->gptTier & GPT_TIER_OVERFLOW) == GPT_TIER_OVERFLOW) &&
             ((value & GPT_TIER_OVERFLOW) == 0) )
        {
          serial_putstring("GPTimer: disabling overflow interrupt.");
          serial_newline();
        }
        else if ( ((gptimer->gptTier & GPT_TIER_OVERFLOW) == 0) &&
             ((value & GPT_TIER_OVERFLOW) == GPT_TIER_OVERFLOW) )
        {
          serial_putstring("GPTimer: enabling overflow interrupt.");
          serial_newline();
        }
        
        if ( ((gptimer->gptTier & GPT_TIER_MATCH) == GPT_TIER_MATCH) &&
             ((value & GPT_TIER_MATCH) == 0) )
        {
          serial_putstring("GPTimer: disabling match interrupt.");
          serial_newline();
        }
        else if ( ((gptimer->gptTier & GPT_TIER_MATCH) == 0) &&
             ((value & GPT_TIER_MATCH) == GPT_TIER_MATCH) )
        {
          serial_putstring("GPTimer: enabling match interrupt.");
          serial_newline();
        }
      }
      gptimer->gptTier = value;
      break;
    case GPT_REG_TWER:
      value &= ~GPT_TWER_RESERVED;
      if (value != gptimer->gptTwer)
      {
        if ( ((gptimer->gptTwer & GPT_TWER_CAPTURE) == GPT_TWER_CAPTURE) &&
             ((value & GPT_TWER_CAPTURE) == 0) )
        {
          serial_putstring("GPTimer: disabling capture wakeup event.");
          serial_newline();
        }
        else if ( ((gptimer->gptTwer & GPT_TIER_CAPTURE) == 0) &&
             ((value & GPT_TWER_CAPTURE) == GPT_TWER_CAPTURE) )
        {
          serial_putstring("GPTimer: enabling capture wakeup event.");
          serial_newline();
        }
        
        if ( ((gptimer->gptTwer & GPT_TWER_OVERFLOW) == GPT_TWER_OVERFLOW) &&
             ((value & GPT_TIER_OVERFLOW) == 0) )
        {
          serial_putstring("GPTimer: disabling overflow wakeup event.");
          serial_newline();
        }
        else if ( ((gptimer->gptTwer & GPT_TWER_OVERFLOW) == 0) &&
             ((value & GPT_TWER_OVERFLOW) == GPT_TWER_OVERFLOW) )
        {
          serial_putstring("GPTimer: enabling overflow wakeup event.");
          serial_newline();
        }
        
        if ( ((gptimer->gptTwer & GPT_TWER_MATCH) == GPT_TWER_MATCH) &&
             ((value & GPT_TWER_MATCH) == 0) )
        {
          serial_putstring("GPTimer: disabling match wakeup event.");
          serial_newline();
        }
        else if ( ((gptimer->gptTwer & GPT_TWER_MATCH) == 0) &&
             ((value & GPT_TWER_MATCH) == GPT_TWER_MATCH) )
        {
          serial_putstring("GPTimer: enabling match wakeup event.");
          serial_newline();
        }
      }
      gptimer->gptTwer = value;
      break;
    case GPT_REG_TOCR:
      value &= ~GPT_TOCR_RESERVED;
      if (value != gptimer->gptTocr)
      {
        serial_putstring("GPTimer: setting overflow counter register to ");
        serial_putint(value);
        gptimer->gptTocr = value;
      }
      break;
    case GPT_REG_TCLR:
      value &= ~GPT_TCLR_RESERVED;
      if (value != gptimer->gptTclr)
      {
        if ( ((gptimer->gptTclr & GPT_TCLR_START_STOP) == GPT_TCLR_START_STOP) &&
             ((value & GPT_TCLR_START_STOP) == 0) )
        {
          serial_ERROR("GPTimer: stopping the counter!");
        }
        else if ( ((gptimer->gptTclr & GPT_TCLR_START_STOP) == 0) &&
             ((value & GPT_TCLR_START_STOP) == GPT_TCLR_START_STOP) )
        {
          serial_ERROR("GPTimer: starting the counter!");
        }
        
        if ( ((gptimer->gptTclr & GPT_TCLR_AUTORELOAD) == GPT_TCLR_AUTORELOAD) &&
             ((value & GPT_TCLR_AUTORELOAD) == 0) )
        {
          serial_ERROR("GPTimer: disableing auto reload!!");
        }
        else if ( ((gptimer->gptTclr & GPT_TCLR_AUTORELOAD) == 0) &&
             ((value & GPT_TCLR_AUTORELOAD) == GPT_TCLR_AUTORELOAD) )
        {
          serial_ERROR("GPTimer: enabling auto reload!");
        }
        serial_ERROR("halt");
      }
    case GPT_REG_TCRR:
      serial_putstring("GPTimer: setting internal counter value to ");
      serial_putint(value);
      serial_newline();
      value = gptimer->gptTcrr;
      break;
    case GPT_REG_TLDR:
      serial_putstring("GPTimer: setting counter load value to ");
      serial_putint(value);
      serial_newline();
      gptimer->gptTldr = value;
      break;
    case GPT_REG_TISTAT:
    case GPT_REG_TISR:
    case GPT_REG_TTGR:
    case GPT_REG_TMAR:
    case GPT_REG_TCAR1:
    case GPT_REG_TCAR2:
    case GPT_REG_TPIR:
    case GPT_REG_TNIR:
    case GPT_REG_TCVR:
    case GPT_REG_TOWR:
      serial_ERROR("GPT: unimplemented register store.");
      break;
    default:
      serial_ERROR("GPT: store to undefined register.");
  } // switch ends
}

