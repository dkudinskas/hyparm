#include "beGPTimer.h"

u32int getCorrectBaseAddr(u32int gptNumber)
{
  switch (gptNumber)
  {
    case 1:
      return GPTIMER1;
    case 2:
      return GPTIMER2;
    case 3:
      return GPTIMER3;
    case 4:
      return GPTIMER4;
    case 5:
      return GPTIMER5;
    case 6:
      return GPTIMER6;
    case 7:
      return GPTIMER7;
    case 8:
      return GPTIMER8;
    case 9:
      return GPTIMER9;
    case 10:
      return GPTIMER10;
    case 11:
      return GPTIMER11;
    case 12:
      return GPTIMER12;
    default:
      serial_ERROR("GPTIMER: invalid timer selected.");
      return 0; //cannot get here
  };
}

void toggleOverflowInterrupt(u32int gptNumber, bool enable)
{
  u32int gptBase = getCorrectBaseAddr(gptNumber);
  u32int * ierAddr = (u32int*)(gptBase | GPT_REG_TIER);
  u32int regVal = *ierAddr;
  if (enable)
  {
    regVal |= GPT_TIER_OVERFLOW;
  }
  else
  {
    regVal &= ~GPT_TIER_OVERFLOW;
  }
  *ierAddr = regVal;
}

void toggleTimer(u32int gptNumber, bool enable)
{
  u32int gptBase = getCorrectBaseAddr(gptNumber);
  u32int * clrAddr = (u32int*)(gptBase | GPT_REG_TCLR);
  u32int regVal = *clrAddr;
  if (enable)
  {
    regVal |= (GPT_TCLR_START_STOP | GPT_TCLR_AUTORELOAD);
  }
  else
  {
    regVal &= ~(GPT_TCLR_START_STOP | GPT_TCLR_AUTORELOAD);
  }
  *clrAddr = regVal;
}

void setupMsTick(u32int gptNumber)
{
  u32int gptBase = getCorrectBaseAddr(gptNumber);
  u32int * ttrgAddr = (u32int*)(gptBase | GPT_REG_TTGR);
  u32int * tpirAddr = (u32int*)(gptBase | GPT_REG_TPIR);
  u32int * tnirAddr = (u32int*)(gptBase | GPT_REG_TNIR);
  u32int * tldrAddr = (u32int*)(gptBase | GPT_REG_TLDR);
  u32int * towrAddr = (u32int*)(gptBase | GPT_REG_TOWR);
  //u32int regVal = 0;


  if ( (gptNumber != 1) && (gptNumber != 2) && (gptNumber != 10) )
  {
    // this GPTimer cannot generate 1ms ticks
    serial_ERROR("setupMsTick: invalid timer selected!");
  }

  *tpirAddr = TPIR1MS;
  *tnirAddr = TNIR1MS;
  *tldrAddr = TLDR1MS;
  *towrAddr = 5000; // interrupt on every 1000 overflows

  // write to trigger register - thus triggering internal counter value reset to TLDR
  *ttrgAddr = 1; // any value...

  // enable overflow interrupt, and start the timer
  toggleOverflowInterrupt(gptNumber, TRUE);
  toggleTimer(gptNumber, TRUE);
}

void deassertInterrupt(u32int gptNumber)
{
  u32int gptBase = getCorrectBaseAddr(gptNumber);
  u32int * regAddr = (u32int*)(gptBase | GPT_REG_TISR);
  u32int isrValue = *regAddr;
  isrValue |= (GPT_TISR_CAPTURE | GPT_TISR_OVERFLOW | GPT_TISR_MATCH);
  *regAddr = isrValue;
}

void dumpGptRegisters(u32int gptNumber)
{
  u32int gptBase = getCorrectBaseAddr(gptNumber);
  u32int * regAddr = 0;

  serial_putstring("-----REGDUMP for GPTimer");
  serial_putint(gptNumber);
  serial_putstring("-----");
  serial_newline();

  regAddr = (u32int*)(gptBase | GPT_REG_TIOCP_CFG);
  serial_putstring("Config: ");
  serial_putint(*regAddr);
  serial_newline();

  regAddr = (u32int*)(gptBase | GPT_REG_TISTAT);
  serial_putstring("Status: ");
  serial_putint(*regAddr);
  serial_newline();

  regAddr = (u32int*)(gptBase | GPT_REG_TISR);
  serial_putstring("Interrupt Status: ");
  serial_putint(*regAddr);
  serial_newline();

  regAddr = (u32int*)(gptBase | GPT_REG_TIER);
  serial_putstring("Interrupt Enable: ");
  serial_putint(*regAddr);
  serial_newline();

  regAddr = (u32int*)(gptBase | GPT_REG_TWER);
  serial_putstring("Wakeup Enable: ");
  serial_putint(*regAddr);
  serial_newline();

  regAddr = (u32int*)(gptBase | GPT_REG_TCLR);
  serial_putstring("Control: ");
  serial_putint(*regAddr);
  serial_newline();

  regAddr = (u32int*)(gptBase | GPT_REG_TCRR);
  serial_putstring("Internal Clock Register value: ");
  serial_putint(*regAddr);
  serial_newline();

  regAddr = (u32int*)(gptBase | GPT_REG_TLDR);
  serial_putstring("Internal Load Register value: ");
  serial_putint(*regAddr);
  serial_newline();

  regAddr = (u32int*)(gptBase | GPT_REG_TTGR);
  serial_putstring("Internal Trigger Register Value: ");
  serial_putint(*regAddr);
  serial_newline();

  regAddr = (u32int*)(gptBase | GPT_REG_TWPS);
  serial_putstring("Write-posted pending: ");
  serial_putint(*regAddr);
  serial_newline();

  regAddr = (u32int*)(gptBase | GPT_REG_TMAR);
  serial_putstring("Internal Match Register Value: ");
  serial_putint(*regAddr);
  serial_newline();

  regAddr = (u32int*)(gptBase | GPT_REG_TCAR1);
  serial_putstring("Capture one Value: ");
  serial_putint(*regAddr);
  serial_newline();

  regAddr = (u32int*)(gptBase | GPT_REG_TSICR);
  serial_putstring("Interface control: ");
  serial_putint(*regAddr);
  serial_newline();

  regAddr = (u32int*)(gptBase | GPT_REG_TSICR);
  serial_putstring("Interface control: ");
  serial_putint(*regAddr);
  serial_newline();

  regAddr = (u32int*)(gptBase | GPT_REG_TCAR2);
  serial_putstring("Capture two Value: ");
  serial_putint(*regAddr);
  serial_newline();

  regAddr = (u32int*)(gptBase | GPT_REG_TPIR);
  serial_putstring("Positive increment value: ");
  serial_putint(*regAddr);
  serial_newline();

  regAddr = (u32int*)(gptBase | GPT_REG_TNIR);
  serial_putstring("Negative increment value: ");
  serial_putint(*regAddr);
  serial_newline();

  regAddr = (u32int*)(gptBase | GPT_REG_TCVR);
  serial_putstring("Clock Value: ");
  serial_putint(*regAddr);
  serial_newline();

  regAddr = (u32int*)(gptBase | GPT_REG_TOCR);
  serial_putstring("Overflow masker: ");
  serial_putint(*regAddr);
  serial_newline();

  regAddr = (u32int*)(gptBase | GPT_REG_TOWR);
  serial_putstring("Overflow wrapper: ");
  serial_putint(*regAddr);
  serial_newline();

  serial_putstring("--------------------------------");
  serial_newline();
}

