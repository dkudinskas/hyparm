#include "common/debug.h"
#include "common/memFunctions.h"

#include "drivers/beagle/beGPTimer.h"

#include "hardware/serial.h"


static inline bool gptBEidValid(u32int id);
static inline bool gptBEisExtended(u32int id);
static inline u32int gptBEgetBaseAddr(u32int id);
static inline u32int gptBEregRead(u32int id, u32int reg);
static inline void gptBEregWrite(u32int id, u32int reg, u32int val);

struct GeneralPurposeTimerBE* gpts[BE_GPTIMER_COUNT];

void gptBEInit(u32int id)
{
  struct GeneralPurposeTimerBE* addr 
     = (struct GeneralPurposeTimerBE*)mallocBytes(sizeof(struct GeneralPurposeTimerBE));

  if (addr == 0)
  {
    DIE_NOW(0, "Failed to allocate GPT_BE.");
  }
  else
  {
    gpts[id-1] = addr;
    memset((void*)addr, 0x0, sizeof(struct GeneralPurposeTimerBE));
    gpts[id-1]->baseAddress = gptBEgetBaseAddr(id);
 
#ifdef GPTIMER_BE_DBG
    serial_putstring("GPT_BE");
    serial_putint_nozeros(id);
    serial_putstring(": Initializing at 0x");
    serial_putint((u32int)gpts[id-1]);
    serial_newline();
#endif
  }
  // reset the device
  gptBEReset(id);
}

void gptBEReset(u32int id)
{
#ifdef GPTIMER_BE_DBG
  serial_putstring("GPT_BE");
  serial_putint_nozeros(id);
  serial_putstring(": software reset");
#endif

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
#ifdef GPTIMER_BE_DBG
  serial_putstring("... done.");
  serial_newline();
#endif
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
#ifdef GPTIMER_BE_DBG
  serial_putstring("GPT");
  serial_putint_nozeros(id);
  serial_putstring("_BE: load from register ");
  serial_putint(reg);
  serial_putstring(" value ");
  serial_putint(value);
  serial_newline();
#endif
  return value;
}

void storeToGPTimer(u32int id, u32int reg, u32int value)
{
  gptBEregWrite(id, reg, value);
#ifdef GPTIMER_BE_DBG
  serial_putstring("GPT");
  serial_putint_nozeros(id);
  serial_putstring("_BE: store to register ");
  serial_putint(reg);
  serial_putstring(" value ");
  serial_putint(value);
  serial_newline();
#endif
}


static inline u32int gptBEregRead(u32int id, u32int reg)
{
  volatile u32int * regPtr = (volatile u32int *) (gpts[id-1]->baseAddress + reg);
  return  *regPtr;
} 

static inline void gptBEregWrite(u32int id, u32int reg, u32int val)
{
  volatile u32int * regPtr = (volatile u32int *) (gpts[id-1]->baseAddress + reg);
  *regPtr = val;
} 


void gptBEClearOverflowInterrupt(u32int id)
{
  gptBEregWrite(id, GPT_REG_TISR, GPT_TISR_OVERFLOW);
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
#ifdef GPTIMER_BE_DBG
  serial_putstring("GPT_BE");
  serial_putint_nozeros(id);
  serial_putstring(": enable.");
  serial_newline();
#endif

  if (gpts[id-1]->enabled == FALSE)
  {
    gpts[id-1]->enabled = TRUE;
  }
#ifdef GPTIMER_BE_DBG
  else
  {
    serial_putstring("gpt_BE");
    serial_putint_nozeros(id);
    serial_putstring(": already enabled");
    serial_newline();
  }
#endif
}

void gptBESet10msTick(u32int id)
{
  if (!gptBEisExtended(id))
  {
    DIE_NOW(0, "GPTIMER: gptSetUp1msTick not supported on this timer");
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
  while ( (gptBEregRead(id, GPT_REG_TISR) & GPT_TISR_OVERFLOW) != GPT_TISR_OVERFLOW)
  {
    // do nothing
  }
}

void gptBEWaitForReset(u32int id)
{
  while ( (gptBEregRead(id, GPT_REG_TISTAT) & GPT_TISTAT_RESETDONE) != GPT_TISTAT_RESETDONE)
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
      DIE_NOW(0, "GPT_BE: invalid id");
  }
  return base;
}


u32int getInternalCounterVal(u32int clkId)
{
  return gptBEregRead(clkId, GPT_REG_TCRR);
}


void gptBEDumpRegisters(u32int id)
{
  serial_putstring("----- REGDUMP GPT");
  serial_putint_nozeros(id);
  serial_putstring("_BE");
  serial_putstring(" -----");
  serial_newline();
  
  serial_putstring("Config: ");
  serial_putint(gptBEregRead(id, GPT_REG_TIOCP_CFG));
  serial_newline();

  serial_putstring("Status: ");
  serial_putint(gptBEregRead(id, GPT_REG_TISTAT));
  serial_newline();

  serial_putstring("Interrupt Status: ");
  serial_putint(gptBEregRead(id, GPT_REG_TISR));
  serial_newline();

  serial_putstring("Interrupt Enable: ");
  serial_putint(gptBEregRead(id, GPT_REG_TIER));
  serial_newline();

  serial_putstring("Wakeup Enable: ");
  serial_putint(gptBEregRead(id, GPT_REG_TWER));
  serial_newline();

  serial_putstring("Control: ");
  serial_putint(gptBEregRead(id, GPT_REG_TCLR));
  serial_newline();

  serial_putstring("Internal Clock Register value: ");
  serial_putint(gptBEregRead(id, GPT_REG_TCRR));
  serial_newline();

  serial_putstring("Internal Load Register value: ");
  serial_putint(gptBEregRead(id, GPT_REG_TLDR));
  serial_newline();

  serial_putstring("Internal Trigger Register Value: ");
  serial_putint(gptBEregRead(id, GPT_REG_TTGR));
  serial_newline();

  serial_putstring("Write-posted pending: ");
  serial_putint(gptBEregRead(id, GPT_REG_TWPS));
  serial_newline();

  serial_putstring("Internal Match Register Value: ");
  serial_putint(gptBEregRead(id, GPT_REG_TMAR));
  serial_newline();

  serial_putstring("Capture one Value: ");
  serial_putint(gptBEregRead(id, GPT_REG_TCAR1));
  serial_newline();

  serial_putstring("Interface control: ");
  serial_putint(gptBEregRead(id, GPT_REG_TSICR));
  serial_newline();

  serial_putstring("Capture two Value: ");
  serial_putint(gptBEregRead(id, GPT_REG_TCAR2));
  serial_newline();

  if (gptBEisExtended(id))
  {
    serial_putstring("Positive increment value: ");
    serial_putint(gptBEregRead(id, GPT_REG_TPIR));
    serial_newline();

    serial_putstring("Negative increment value: ");
    serial_putint(gptBEregRead(id, GPT_REG_TNIR));
    serial_newline();

    serial_putstring("Clock Value: ");
    serial_putint(gptBEregRead(id, GPT_REG_TCVR));
    serial_newline();

    serial_putstring("Overflow masker: ");
    serial_putint(gptBEregRead(id, GPT_REG_TOCR));
    serial_newline();

    serial_putstring("Overflow wrapper: ");
    serial_putint(gptBEregRead(id, GPT_REG_TOWR));
    serial_newline();

    serial_putstring("-----------------------");
    serial_newline();
  }
}
