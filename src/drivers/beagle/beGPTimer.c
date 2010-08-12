#include "beGPTimer.h"
#include "memFunctions.h"

#define GPT_REG_PTR(id,register)           ((u32int *)(gpts[id].baseAddress | (register)))
#define GPT_REG_ENABLE(id,register,mask)   *GPT_REG_PTR(id,register) |= (mask)
#define GPT_REG_DISABLE(id,register,mask)  *GPT_REG_PTR(id,register) &= ~(mask)
#define GPT_REG_READ(id,register)          (*GPT_REG_PTR(id,register))
#define GPT_REG_WRITE(id,register,value)   *GPT_REG_PTR(id,register) = (value)

// GPTIMER1, GPTIMER2 and GPTIMER10 have more registers
#define GPT_EXT(id)                        ((id & 0xfffffffe) == id) || (id == 9)

gptStruct *gpts;

void gptInit()
{
  const int mallocSize = GPTIMER_COUNT * sizeof(gptStruct);;
  gpts = (gptStruct *)mallocBytes(mallocSize);
  memset((void *)gpts, 0x0, mallocSize);
#if FALSE != 0
#error This code assumes FALSE=0
#endif
  gpts[ 0].baseAddress = GPTIMER1;
  gpts[ 1].baseAddress = GPTIMER2;
  gpts[ 2].baseAddress = GPTIMER3;
  gpts[ 3].baseAddress = GPTIMER4;
  gpts[ 4].baseAddress = GPTIMER5;
  gpts[ 5].baseAddress = GPTIMER6;
  gpts[ 6].baseAddress = GPTIMER7;
  gpts[ 7].baseAddress = GPTIMER8;
  gpts[ 8].baseAddress = GPTIMER9;
  gpts[ 9].baseAddress = GPTIMER10;
  gpts[10].baseAddress = GPTIMER11;
  gpts[11].baseAddress = GPTIMER12;
#if GPTIMER_COUNT != 12
#error This code only initializes 11 timers
#endif
  // enable GPTIMER1
  gptEnable(0);
  gptReset(0);

//+..
//  gptSetOverflowInterrupt(gptNumber, TRUE);
//+..


}

void gptEnable(u32int id)
{
  if (gpts[id].enabled)
  {
    return;
  }
/*
  omap_dm_clk_enable(timer->fclk);
  omap_dm_clk_enable(timer->iclk);
*/
  gpts[id].enabled = TRUE;
}

void gptReset(u32int id)
{
  u32int reg;
/*
  if ( timer != &dm_timers[0])
  {
*/
  GPT_REG_WRITE(id, GPT_REG_TSICR, GPT_TSICR_POSTED | GPT_TSICR_SFTRESET);
  gptWaitForReset(id);
/*
  }
  omap_dm_timer_set_source(timer, OMAP_TIMER_SRC_32_KHZ);
*/
  reg = GPT_REG_READ(id, GPT_REG_TIOCP_CFG);
  reg |= 0x02 << 3;  /* Set to smart-idle mode */
  reg |= 0x2 << 8;   /* Set clock activity to perserve f-clock on idle */

  /*
   * Enable wake-up only for GPT1 on OMAP2 CPUs.
   * FIXME: All timers should have wake-up enabled and clear
   * PRCM status.
   * /
  if (timer == &dm_timers[0])
{*/
  reg |= 1 << 2;
/*}*/
  GPT_REG_WRITE(id, GPT_REG_TIOCP_CFG, reg);

  /* Match hardware reset default of posted mode
  omap_dm_timer_write_reg(timer, OMAP_TIMER_IF_CTRL_REG,
      OMAP_TIMER_CTRL_POSTED);*/
  GPT_REG_WRITE(id, GPT_REG_TSICR, GPT_TSICR_POSTED);
  gpts[id].posted = 1;
}

void gptWaitForReset(u32int id)
{
  u32int c;
  c = 0;
  while (!(GPT_REG_READ(id, GPT_REG_TISTAT) & GPT_TISTAT_RESETDONE)) {
    if (++c > 100000) {
      serial_ERROR("GPTIMER: failed to reset.");
    }
  }
}

/*void gptModifyRegister(u32int *address, u32int mask, bool enable)
{
  u32int value = *address;
  if (enable)
  {
    value |= mask;
  }
  else
  {
    value &= ~mask;
  }
  *address = value;
}

void gptSetSource(u32int gptNumber)
{
}
*/


/*

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
  GPT_REG_ENABLE(id, GPT_REG_TIER, GPT_TIER_OVERFLOW);
  toggleTimer(gptNumber, TRUE);
}

u32int gpt10OvfIt()
{
  return (*((u32int*)(getCorrectBaseAddr(10) | GPT_REG_TISR)) & GPT_TISR_OVERFLOW);
}

void deassertInterrupt(u32int gptNumber)
{
  u32int gptBase = getCorrectBaseAddr(gptNumber);
  u32int * regAddr = (u32int*)(gptBase | GPT_REG_TISR);
  u32int isrValue = *regAddr;
  isrValue |= (GPT_TISR_CAPTURE | GPT_TISR_OVERFLOW | GPT_TISR_MATCH);
  *regAddr = isrValue;
}*/

void dumpGptRegisters(/*u32int gptNumber*/)
{
  u32int gptBase = GPTIMER1;
  u32int * regAddr = 0;

  serial_putstring("-----REGDUMP for GPTimer");
//  serial_putint(gptNumber);
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

