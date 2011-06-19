#include "common/debug.h"
#include "common/memFunctions.h"

#include "drivers/beagle/beClockMan.h"


struct ClockManagerBE * clkManBE;

static inline u32int clkManRegReadBE(u32int module, u32int regOffs);
static inline void clkManRegWriteBE(u32int module, u32int regOffs, u32int value);

void clkManBEInit()
{
  clkManBE = (struct ClockManagerBE*)mallocBytes(sizeof(struct ClockManagerBE));
  if (clkManBE == 0)
  {
    DIE_NOW(0, "Failed to allocate CLK_MAN_BE.");
  }
  else
  {
    memset((void*)clkManBE, 0x0, sizeof(struct ClockManagerBE));
#ifdef BE_CLK_MAN_DBG
    printf("Initializing CLK_MAN_BE at %x\n", (u32int)clkManBE);
#endif
  }

  clkManBE->initialized = TRUE;

}

static inline u32int clkManRegReadBE(u32int module, u32int regOffs)
{
  u32int * regPtr = (u32int*)(module | regOffs);
  volatile u32int value = *regPtr;
  return value;
}

static inline void clkManRegWriteBE(u32int module, u32int regOffs, u32int value)
{
  volatile u32int * regPtr = (u32int*)(module | regOffs);
  *regPtr = value;
}

void toggleTimerFclk(u32int clockID, bool enable)
{
#ifdef BE_CLK_MAN_DBG
    printf("CLK_MAN_BE: toggleTimerFclk(): timer %x", clockID);
    printf((enable) ? " enable\n" : " disable\n");
#endif

  if (clockID == 1)
  {
    u32int regVal = clkManRegReadBE(WKUP_CM, CM_FCLKEN_WKUP);
    if (enable)
    {
      regVal |= CM_FCLKEN_WKUP_ENGPT1;
    }
    else
    {
      regVal &= ~CM_FCLKEN_WKUP_ENGPT1;
    }
    clkManRegWriteBE(WKUP_CM, CM_FCLKEN_WKUP, regVal);
  }
  else
  {
    u32int regVal = clkManRegReadBE(PER_CM, CM_FCLKEN_PER);

    switch (clockID)
    {
      case 2:
        regVal = (enable) ? (regVal | CM_FCLKEN_PER_GPT2) : (regVal & ~CM_FCLKEN_PER_GPT2);
        break;
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
        DIE_NOW(0, "CLK_MAN_BE: toggleTimerFclk() unimplemented for clock ID");
      default:
        DIE_NOW(0, "CLK_MAN_BE: setclockSource() invalid clock ID");
    }
    clkManRegWriteBE(PER_CM, CM_FCLKEN_PER, regVal);
  } // else ends
}

void setClockSource(u32int clockID, bool sysClock)
{
#ifdef BE_CLK_MAN_DBG
    printf("CLK_MAN_BE: setClockSource for timer %x to ", clockID);
    printf((sysClock) ? "sysClock\n" : "32kH clock\n");
#endif

  if (clockID == 1)
  {
    u32int regVal = clkManRegReadBE(WKUP_CM, CM_CLKSEL_WKUP);
    if (sysClock)
    {
      regVal |= CM_CLKSEL_WKUP_GPT1;
    }
    else
    {
      regVal &= ~CM_CLKSEL_WKUP_GPT1;
    }
    clkManRegWriteBE(WKUP_CM, CM_CLKSEL_WKUP, regVal);
  }
  else
  {
    u32int regVal = clkManRegReadBE(PER_CM, CM_CLKSEL_PER);

    switch (clockID)
    {
      case 2:
        regVal = (sysClock) ? (regVal | CM_CLKSEL_PER_GPT2) : (regVal & ~CM_CLKSEL_PER_GPT2);
        break;
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
        DIE_NOW(0, "CLK_MAN_BE: setclockSource() unimplemented for clock ID");
      default:
        DIE_NOW(0, "CLK_MAN_BE: setclockSource() invalid clock ID");
    }
    clkManRegWriteBE(PER_CM, CM_CLKSEL_PER, regVal);
  } // else ends
}

void cmDisableDssClocks()
{
  clkManRegWriteBE(DSS_CM, CM_FCLKEN_DSS, 0);
  clkManRegWriteBE(DSS_CM, CM_ICLKEN_DSS, 0);
}
