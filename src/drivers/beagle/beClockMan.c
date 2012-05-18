#include "common/debug.h"
#include "common/memFunctions.h"
#include "common/stddef.h"
#include "common/string.h"

#include "drivers/beagle/beClockMan.h"


struct ClockManagerBE
{
  /* add stuff if needed */
  bool initialized;
};

static struct ClockManagerBE *clkManBE;


void clkManBEInit()
{
  clkManBE = (struct ClockManagerBE*)mallocBytes(sizeof(struct ClockManagerBE));
  if (clkManBE == NULL)
  {
    DIE_NOW(NULL, "Failed to allocate CLK_MAN_BE.");
  }
  else
  {
    memset(clkManBE, 0, sizeof(struct ClockManagerBE));
    DEBUG(PP_OMAP_35XX_CM, "Initializing CLK_MAN_BE at %p" EOL, clkManBE);
  }

  clkManBE->initialized = TRUE;
}

inline u32int clkManRegReadBE(u32int module, u32int regOffs)
{
  return *(volatile u32int *)(module | regOffs);
}

inline void clkManRegWriteBE(u32int module, u32int regOffs, u32int value)
{
  *(volatile u32int *)(module | regOffs) = value;
}

void toggleTimerFclk(u32int clockID, bool enable)
{
  DEBUG(PP_OMAP_35XX_CM, "(CLK_MAN_BE: toggleTimerFclk(): timer %#x %s" EOL, clockID,
      enable ? "enable" : "disable");

  if (clockID == 1)
  {
    u32int regVal = clkManRegReadBE(WKUP_CM, CM_FCLKEN_WKUP);
    regVal = enable ? (regVal | CM_FCLKEN_WKUP_ENGPT1) : (regVal & ~CM_FCLKEN_WKUP_ENGPT1);
    clkManRegWriteBE(WKUP_CM, CM_FCLKEN_WKUP, regVal);
  }
  else
  {
    u32int regVal = clkManRegReadBE(PER_CM, CM_FCLKEN_PER);

    switch (clockID)
    {
      case 2:
        regVal = enable ? (regVal | CM_FCLKEN_PER_GPT2) : (regVal & ~CM_FCLKEN_PER_GPT2);
        break;
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
        DIE_NOW(NULL, "CLK_MAN_BE: toggleTimerFclk() unimplemented for clock ID");
      default:
        DIE_NOW(NULL, "CLK_MAN_BE: setclockSource() invalid clock ID");
    }
    clkManRegWriteBE(PER_CM, CM_FCLKEN_PER, regVal);
  } // else ends
}

void setClockSource(u32int clockID, bool sysClock)
{
  DEBUG(PP_OMAP_35XX_CM, "CLK_MAN_BE: setClockSource for timer %#x to %s" EOL, clockID,
      sysClock ? "sysClock" : "32 kHz clock");

  if (clockID == 1)
  {
    u32int regVal = clkManRegReadBE(WKUP_CM, CM_CLKSEL_WKUP);
    regVal = sysClock ? (regVal | CM_CLKSEL_WKUP_GPT1) : (regVal & ~CM_CLKSEL_WKUP_GPT1);
    clkManRegWriteBE(WKUP_CM, CM_CLKSEL_WKUP, regVal);
  }
  else
  {
    u32int regVal = clkManRegReadBE(PER_CM, CM_CLKSEL_PER);

    switch (clockID)
    {
      case 2:
        regVal = sysClock ? (regVal | CM_CLKSEL_PER_GPT2) : (regVal & ~CM_CLKSEL_PER_GPT2);
        break;
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
        DIE_NOW(NULL, "CLK_MAN_BE: setclockSource() unimplemented for clock ID");
      default:
        DIE_NOW(NULL, "CLK_MAN_BE: setclockSource() invalid clock ID");
    }
    clkManRegWriteBE(PER_CM, CM_CLKSEL_PER, regVal);
  } // else ends
}

void cmDisableDssClocks()
{
  clkManRegWriteBE(DSS_CM, CM_FCLKEN_DSS, 0);
  clkManRegWriteBE(DSS_CM, CM_ICLKEN_DSS, 0);
}
