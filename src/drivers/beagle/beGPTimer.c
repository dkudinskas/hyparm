#include "beGPTimer.h"
#include "memFunctions.h"


#define GPT_VALID(id)                      ((id >= 0) && (id < GPTIMER_COUNT))
#define GPT_EXTENDED(id)                   ((id == 0) || (id == 1) || (id == 9))

#define GPT_REG_PTR(id,register)           ((volatile u32int *)(gpts[id].baseAddress | (register)))
#define GPT_REG_ENABLE(id,register,mask)   *GPT_REG_PTR(id,register) |= (mask)
#define GPT_REG_DISABLE(id,register,mask)  *GPT_REG_PTR(id,register) &= ~(mask)
#define GPT_REG_READ(id,register)          (*GPT_REG_PTR(id,register))
#define GPT_REG_WRITE(id,register,value)   *GPT_REG_PTR(id,register) = (value)

#define GPT_WAIT_FOR(condition,error)      { u32int c = 0; while (!(condition)) { if (++c > 0xfffffffe) { serial_ERROR(error); } } }


gptStruct *gpts;


void gptClearOverflowInterrupt(u32int id)
{
  GPT_REG_WRITE(id, GPT_REG_TISR, GPT_TISR_OVERFLOW);
}

void gptDisableOverflowInterrupt(u32int id)
{
  GPT_REG_DISABLE(id, GPT_REG_TIER, GPT_TIER_OVERFLOW);
}

void gptEnable(u32int id)
{/*
  if (gpts[id].enabled)
  {
    return;
  }
/ *
  omap_dm_clk_enable(timer->fclk);
  omap_dm_clk_enable(timer->iclk);
* /
  gpts[id].enabled = TRUE;*/
}

void gptEnableOverflowInterrupt(u32int id)
{
  GPT_REG_ENABLE(id, GPT_REG_TIER, GPT_TIER_OVERFLOW);
}

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
  //gptEnable(0);
  //gptReset(0);
}

void gptReset(u32int id)
{
  u32int reg;
  GPT_REG_WRITE(id, GPT_REG_TSICR, GPT_TSICR_POSTED | GPT_TSICR_SFTRESET);
  gptWaitForReset(id);
  // TODO
  //gptSetSource(id, 0/*OMAP_TIMER_SRC_32_KHZ*/);

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

  // match hardware reset default of posted mode
  GPT_REG_WRITE(id, GPT_REG_TSICR, GPT_TSICR_POSTED);
  
  // TODO
  //gpts[id].posted = 1;
}

void gptSet10msTick(u32int id)
{
  if (!GPT_EXTENDED(id))
  {
    serial_ERROR("GPTIMER: gptSetUp1msTick not supported on this timer");
  }
  GPT_REG_WRITE(id, GPT_REG_TLDR, GPT_REG_TLDR_LOAD_VALUE_1MS);
  GPT_REG_WRITE(id, GPT_REG_TPIR, GPT_TPIR_POS_INC_1MS);
  GPT_REG_WRITE(id, GPT_REG_TNIR, GPT_TNIR_NEG_INC_1MS);
  GPT_REG_WRITE(id, GPT_REG_TOWR, GPT_TOWR_OVF_WRAPPING & GPT_REG_TOWR_OVF_10MS);
  // write to trigger register - thus triggering internal counter value reset to TLDR
  GPT_REG_WRITE(id, GPT_REG_TTGR, 1); // any value...
  // set autoreload
  GPT_REG_ENABLE(id, GPT_REG_TCLR, GPT_TCLR_AUTORELOAD);
}

void gptStart(u32int id)
{
  GPT_REG_ENABLE(id, GPT_REG_TCLR, GPT_TCLR_START_STOP);
}

void gptStop(u32int id)
{
  GPT_REG_DISABLE(id, GPT_REG_TCLR, GPT_TCLR_START_STOP);
}

void gptWaitForOverflowInterrupt(u32int id)
{
  GPT_WAIT_FOR(GPT_REG_READ(id, GPT_REG_TISR) & GPT_TISR_OVERFLOW, "GPTIMER: did not get interrupt.");
}

void gptWaitForReset(u32int id)
{
  GPT_WAIT_FOR(GPT_REG_READ(id, GPT_REG_TISTAT) & GPT_TISTAT_RESETDONE, "GPTIMER: failed to reset.");
}

void gptDumpRegisters(u32int id)
{
  serial_putstring("-----REGDUMP for GPTimer");
  serial_putint(id + 1);
  serial_putstring("-----");
  serial_newline();
  
  serial_putstring("Config: ");
  serial_putint(GPT_REG_READ(id, GPT_REG_TIOCP_CFG));
  serial_newline();

  serial_putstring("Status: ");
  serial_putint(GPT_REG_READ(id, GPT_REG_TISTAT));
  serial_newline();

  serial_putstring("Interrupt Status: ");
  serial_putint(GPT_REG_READ(id, GPT_REG_TISR));
  serial_newline();

  serial_putstring("Interrupt Enable: ");
  serial_putint(GPT_REG_READ(id, GPT_REG_TIER));
  serial_newline();

  serial_putstring("Wakeup Enable: ");
  serial_putint(GPT_REG_READ(id, GPT_REG_TWER));
  serial_newline();

  serial_putstring("Control: ");
  serial_putint(GPT_REG_READ(id, GPT_REG_TCLR));
  serial_newline();

  serial_putstring("Internal Clock Register value: ");
  serial_putint(GPT_REG_READ(id, GPT_REG_TCRR));
  serial_newline();

  serial_putstring("Internal Load Register value: ");
  serial_putint(GPT_REG_READ(id, GPT_REG_TLDR));
  serial_newline();

  serial_putstring("Internal Trigger Register Value: ");
  serial_putint(GPT_REG_READ(id, GPT_REG_TTGR));
  serial_newline();

  serial_putstring("Write-posted pending: ");
  serial_putint(GPT_REG_READ(id, GPT_REG_TWPS));
  serial_newline();

  serial_putstring("Internal Match Register Value: ");
  serial_putint(GPT_REG_READ(id, GPT_REG_TMAR));
  serial_newline();

  serial_putstring("Capture one Value: ");
  serial_putint(GPT_REG_READ(id, GPT_REG_TCAR1));
  serial_newline();

  serial_putstring("Interface control: ");
  serial_putint(GPT_REG_READ(id, GPT_REG_TSICR));
  serial_newline();

  serial_putstring("Capture two Value: ");
  serial_putint(GPT_REG_READ(id, GPT_REG_TCAR2));
  serial_newline();

  if (GPT_EXTENDED(id))
  {
    serial_putstring("Positive increment value: ");
    serial_putint(GPT_REG_READ(id, GPT_REG_TPIR));
    serial_newline();

    serial_putstring("Negative increment value: ");
    serial_putint(GPT_REG_READ(id, GPT_REG_TNIR));
    serial_newline();

    serial_putstring("Clock Value: ");
    serial_putint(GPT_REG_READ(id, GPT_REG_TCVR));
    serial_newline();

    serial_putstring("Overflow masker: ");
    serial_putint(GPT_REG_READ(id, GPT_REG_TOCR));
    serial_newline();

    serial_putstring("Overflow wrapper: ");
    serial_putint(GPT_REG_READ(id, GPT_REG_TOWR));
    serial_newline();

    serial_putstring("--------------------------------");
    serial_newline();
  }
}
