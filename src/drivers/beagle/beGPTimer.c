#include "common/debug.h"
#include "common/memFunctions.h"
#include "common/stddef.h"

#include "drivers/beagle/beGPTimer.h"


struct GeneralPurposeTimerBE
{
  u32int baseAddress;
  bool enabled;
  //bool enabled;
  //bool posted;
};

static struct GeneralPurposeTimerBE *gpts[BE_GPTIMER_COUNT];


static inline bool gptBEidValid(u32int id);
static inline bool gptBEisExtended(u32int id);
static inline u32int gptBEgetBaseAddr(u32int id);
static inline u32int gptBEregRead(u32int id, u32int reg);
static inline void gptBEregWrite(u32int id, u32int reg, u32int val);


void gptBEInit(u32int id)
{
  struct GeneralPurposeTimerBE *gpt
     = (struct GeneralPurposeTimerBE *)mallocBytes(sizeof(struct GeneralPurposeTimerBE));

  if (!gpt)
  {
    DIE_NOW(NULL, "Failed to allocate GPT_BE.");
  }

  memset(gpt, 0, sizeof(struct GeneralPurposeTimerBE));
  gpt->baseAddress = gptBEgetBaseAddr(id);

  gpts[id - 1] = gpt;

  DEBUG(PP_OMAP_35XX_GPTIMER, "GPT_BE%x: Initializing at %p" EOL, id, gpts[id-1]);

  // reset the device
  gptBEReset(id);
}

void gptBEReset(u32int id)
{
  DEBUG(PP_OMAP_35XX_GPTIMER, "GPT_BE%x: software reset... ", id);

  // reset the device
  gptBEregWrite(id, GPT_REG_TSICR, GPT_TSICR_SFTRESET);
  // .. and put into posted mode
  gptBEregWrite(id, GPT_REG_TSICR, gptBEregRead(id, GPT_REG_TSICR)|GPT_TSICR_POSTED);

  // now lets configure it:
  u32int configReg = gptBEregRead(id, GPT_REG_TIOCP_CFG);
  // set to smartidle
  configReg &= ~GPT_TIOCP_CFG_IDLEMODE;
  configReg |= GPT_TIOCP_CFG_IDLEMODE_SMART;

  // set clock activity to perserve f-clock on idle
  configReg &= ~GPT_TIOCP_CFG_CLOCKACTIVITY;
  configReg |= GPT_TIOCP_CFG_CLOCKACTIVITY_FCLK;

  // Enable wake-up, if core clock (1)
  if (id == 1)
  {
    configReg |= GPT_TIOCP_CFG_ENABLEWAKEUP;
  }

  gptBEregWrite(id, GPT_REG_TIOCP_CFG, configReg);

  DEBUG(PP_OMAP_35XX_GPTIMER, "done." EOL);
}

static inline bool gptBEidValid(u32int id)
{
  return (id > 0) && (id <= BE_GPTIMER_COUNT);
}

static inline bool gptBEisExtended(u32int id)
{
  return ((id == 1) || (id == 2) || (id == 10));
}

u32int loadFromGPTimer(u32int id, u32int reg)
{
  u32int value = gptBEregRead(id, reg);
  DEBUG(PP_OMAP_35XX_GPTIMER, "GPT%x_BE: load from register %#x value %#x" EOL, id, reg, value);
  return value;
}

void storeToGPTimer(u32int id, u32int reg, u32int value)
{
  gptBEregWrite(id, reg, value);
  DEBUG(PP_OMAP_35XX_GPTIMER, "GPT%x_BE: store to register %#x value %#x" EOL, id, reg, value);
}

static inline u32int gptBEregRead(u32int id, u32int reg)
{
  return *(volatile u32int *)(gpts[id - 1]->baseAddress + reg);
}

static inline void gptBEregWrite(u32int id, u32int reg, u32int val)
{
  *(volatile u32int *)(gpts[id - 1]->baseAddress + reg) = val;
}

void gptBEClearOverflowInterrupt(u32int id)
{
  gptBEregWrite(id, GPT_REG_TISR, GPT_TISR_OVERFLOW);
}

void gptBEClearMatchInterrupt(u32int id)
{
  gptBEregWrite(id, GPT_REG_TISR, GPT_TISR_MATCH);
}

void gptBEDisableOverflowInterrupt(u32int id)
{
  u32int regVal = gptBEregRead(id, GPT_REG_TIER);
  gptBEregWrite(id, GPT_REG_TIER, regVal & ~GPT_TIER_OVERFLOW);
}

void gptBEEnableOverflowInterrupt(u32int id)
{
  u32int regVal = gptBEregRead(id, GPT_REG_TIER);
  gptBEregWrite(id, GPT_REG_TIER, regVal | GPT_TIER_OVERFLOW);
}

void gptBEEnable(u32int id)
{
  DEBUG(PP_OMAP_35XX_GPTIMER, "GPT_BE%x: enable." EOL, id);

  if (!gpts[id - 1]->enabled)
  {
    gpts[id - 1]->enabled = TRUE;
  }
  else
  {
    DEBUG(PP_OMAP_35XX_GPTIMER, "gpt_BE%x: already enabled" EOL, id);
  }
}

void gptBESet10msTick(u32int id)
{
  if (!gptBEisExtended(id))
  {
    DIE_NOW(NULL, "GPTIMER: gptSetUp1msTick not supported on this timer");
  }

  gptBEregWrite(id, GPT_REG_TLDR, GPT_REG_TLDR_LOAD_VALUE_1MS);
  gptBEregWrite(id, GPT_REG_TPIR, GPT_TPIR_POS_INC_1MS);
  gptBEregWrite(id, GPT_REG_TNIR, GPT_TNIR_NEG_INC_1MS);
  gptBEregWrite(id, GPT_REG_TOWR, GPT_TOWR_OVF_WRAPPING & GPT_REG_TOWR_OVF_10MS);
  // write to trigger register - thus triggering internal counter value reset to TLDR
  gptBEregWrite(id, GPT_REG_TTGR, 1); // any value...
  // set autoreload
  gptBEregWrite(id, GPT_REG_TCLR, (gptBEregRead(id, GPT_REG_TCLR) | GPT_TCLR_AUTORELOAD));
}

void gptBEStart(u32int id)
{
  gptBEregWrite(id, GPT_REG_TCLR, (gptBEregRead(id, GPT_REG_TCLR) | GPT_TCLR_START_STOP));
}

void gptBEStop(u32int id)
{
  gptBEregWrite(id, GPT_REG_TCLR, (gptBEregRead(id, GPT_REG_TCLR) & ~GPT_TCLR_START_STOP));
}

void gptBEWaitForOverflowInterrupt(u32int id)
{
  while ((gptBEregRead(id, GPT_REG_TISR) & GPT_TISR_OVERFLOW) != GPT_TISR_OVERFLOW)
  {
    // do nothing
  }
}

void gptBEWaitForReset(u32int id)
{
  while ((gptBEregRead(id, GPT_REG_TISTAT) & GPT_TISTAT_RESETDONE) != GPT_TISTAT_RESETDONE)
  {
    // do nothing
  }
}

static inline u32int gptBEgetBaseAddr(u32int id)
{
  u32int base = 0;
  switch(id)
  {
    case 1:
      base = GPTIMER1;
      break;
    case 2:
      base = GPTIMER2;
      break;
    case 3:
      base = GPTIMER3;
      break;
    case 4:
      base = GPTIMER4;
      break;
    case 5:
      base = GPTIMER5;
      break;
    case 6:
      base = GPTIMER6;
      break;
    case 7:
      base = GPTIMER7;
      break;
    case 8:
      base = GPTIMER8;
      break;
    case 9:
      base = GPTIMER9;
      break;
    case 10:
      base = GPTIMER10;
      break;
    case 11:
      base = GPTIMER11;
      break;
    case 12:
      base = GPTIMER12;
      break;
    default:
      DIE_NOW(NULL, "GPT_BE: invalid id");
  }
  return base;
}

u32int gptBEGetCounter(u32int clkId)
{
  return gptBEregRead(clkId, GPT_REG_TCRR);
}

void gptBEResetCounter(u32int id)
{
  gptBEregWrite(id, GPT_REG_TCRR, 0);
}

void gptBEDumpRegisters(u32int id)
{
  printf("----- REGDUMP GPT%x_BE -----", id);
  printf("Config: %#.8x" EOL, gptBEregRead(id, GPT_REG_TIOCP_CFG));
  printf("Status: %#.8x" EOL, gptBEregRead(id, GPT_REG_TISTAT));
  printf("Interrupt Status: %#.8x" EOL, gptBEregRead(id, GPT_REG_TISR));
  printf("Interrupt Enable: %#.8x" EOL, gptBEregRead(id, GPT_REG_TIER));
  printf("Wakeup Enable: %#.8x" EOL, gptBEregRead(id, GPT_REG_TWER));
  printf("Control: %#.8x" EOL, gptBEregRead(id, GPT_REG_TCLR));
  printf("Internal Clock Register value: %#.8x" EOL, gptBEregRead(id, GPT_REG_TCRR));
  printf("Internal Load Register value: %#.8x" EOL, gptBEregRead(id, GPT_REG_TLDR));
  printf("Internal Trigger Register Value: %#.8x" EOL, gptBEregRead(id, GPT_REG_TTGR));
  printf("Write-posted pending: %#.8x" EOL, gptBEregRead(id, GPT_REG_TWPS));
  printf("Internal Match Register Value: %#.8x" EOL, gptBEregRead(id, GPT_REG_TMAR));
  printf("Capture one Value: %#.8x" EOL, gptBEregRead(id, GPT_REG_TCAR1));
  printf("Interface control: %#.8x" EOL, gptBEregRead(id, GPT_REG_TSICR));
  printf("Capture two Value: %#.8x" EOL, gptBEregRead(id, GPT_REG_TCAR2));

  if (gptBEisExtended(id))
  {
    printf("Positive increment value: %#.8x" EOL, gptBEregRead(id, GPT_REG_TPIR));
    printf("Negative increment value: %#.8x" EOL, gptBEregRead(id, GPT_REG_TNIR));
    printf("Clock Value: %#.8x" EOL, gptBEregRead(id, GPT_REG_TCVR));
    printf("Overflow masker: %#.8x" EOL, gptBEregRead(id, GPT_REG_TOCR));
    printf("Overflow wrapper: %#.8x" EOL, gptBEregRead(id, GPT_REG_TOWR));
    printf("-----------------------");
  }
}
