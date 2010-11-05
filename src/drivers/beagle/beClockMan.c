#include "beClockMan.h"
#include "memFunctions.h"
#include "types.h"

struct ClockManagerBE * clkManBE;

void clkManBEInit()
{
  clkManBE = (struct ClockManagerBE*)mallocBytes(sizeof(struct ClockManagerBE));
  if (clkManBE == 0)
  {
    serial_ERROR("Failed to allocate CLK_MAN_BE.");
  }
  else
  {
    memset((void*)clkManBE, 0x0, sizeof(struct ClockManagerBE));
#ifdef BE_CLK_MAN_DBG
    serial_putstring("Initializing CLK_MAN_BE at 0x");
    serial_putint((u32int)clkManBE);
    serial_newline();
#endif
  }

  clkManBE->initialized = TRUE;

}

inline u32int clkManRegReadBE(u32int module, u32int regOffs)
{
  u32int * regPtr = (u32int*)(module | regOffs);
  volatile u32int value = *regPtr;
  return value;
}

inline void clkManRegWriteBE(u32int module, u32int regOffs, u32int value)
{
  volatile u32int * regPtr = (u32int*)(module | regOffs);
  *regPtr = value;
}

void toggleTimerFclk(u32int clockID, bool enable)
{
#ifdef BE_CLK_MAN_DBG
    serial_putstring("CLK_MAN_BE: toggleTimerFclk(): timer ");
    serial_putint_nozeros(clockID);
    serial_putstring((enable) ? " enable" : " disable");
    serial_newline();
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
        serial_ERROR("CLK_MAN_BE: toggleTimerFclk() unimplemented for clock ID");
      default:
        serial_ERROR("CLK_MAN_BE: setclockSource() invalid clock ID");
    }
    clkManRegWriteBE(PER_CM, CM_FCLKEN_PER, regVal);
  } // else ends
}

void setClockSource(u32int clockID, bool sysClock)
{
#ifdef BE_CLK_MAN_DBG
    serial_putstring("CLK_MAN_BE: setClockSource for timer ");
    serial_putint_nozeros(clockID);
    serial_putstring(" to ");
    serial_putstring((sysClock) ? "sysClock" : "32kH clock");
    serial_newline();
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
        serial_ERROR("CLK_MAN_BE: setclockSource() unimplemented for clock ID");
      default:
        serial_ERROR("CLK_MAN_BE: setclockSource() invalid clock ID");
    }
    clkManRegWriteBE(PER_CM, CM_CLKSEL_PER, regVal);
  } // else ends
}

void cmDisableDssClocks()
{
  clkManRegWriteBE(DSS_CM, CM_FCLKEN_DSS, 0);
  clkManRegWriteBE(DSS_CM, CM_ICLKEN_DSS, 0);
}
