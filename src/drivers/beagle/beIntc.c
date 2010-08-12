#include "beIntc.h"
#include "memFunctions.h"

struct InterruptControllerBE * intcBE;

void initialiseInterruptController()
{
  intcBE = (struct InterruptControllerBE*)mallocBytes(sizeof(struct InterruptControllerBE));
  if (intcBE == 0)
  {
    serial_ERROR("Failed to allocate INTC_BE.");
  }
  else
  {
    memset((void*)intcBE, 0x0, sizeof(struct InterruptControllerBE));
#ifdef BE_INTC_DBG
    serial_putstring("Initializing INTC_BE at 0x");
    serial_putint((u32int)intcBE);
    serial_newline();
#endif
  }

  intcBE->enabled = TRUE;
  intcBE->baseAddress = 0x48200000; // Section 10.4.1 p1200
  intcBE->size = 0x1000; // 4 KB

  u32int i = 0, m = 0;
  // soft reset
  serial_putstring("INTC_BE: soft reset ...");
  u32int conf = intcRegRead(REG_INTCPS_SYSCONFIG);
  conf |= INTCPS_SYSCONFIG_SOFTRESET;
  intcRegWrite(REG_INTCPS_SYSCONFIG, conf);

  while (!(intcRegRead(REG_INTCPS_SYSSTATUS) & INTCPS_SYSSTATUS_SOFTRESET))
  {
    serial_putstring(".");
  }
  serial_putstring(" done");
  serial_newline();
   
  // intc autoidle
  intcRegWrite(REG_INTCPS_SYSCONFIG, INTCPS_SYSCONFIG_AUTOIDLE);

  // set register access protection (priviledged modes only)!
  intcRegWrite(REG_INTCPS_PROTECTION, INTCPS_PROTECTION_PROTECTION);
  
  // mask interrupts (all)
  for (i = 0; i < INTCPS_NR_OF_BANKS; i++)
  {
    intcRegWrite(REG_INTCPS_MIR_SETn + 0x20*i, INTCPS_MIR_SETn_MIRSET);
  }
  // set all priorities to 0x0 (arbitrary)
  for (m = 0; m < INTCPS_NR_OF_INTERRUPTS; m++)
  {
    intcRegWrite(REG_INTCPS_ILRm+m*0x4, 0);
  }

  // disable interrupt priority threshold
  intcRegWrite(REG_INTCPS_THRESHOLD, INTCPS_THRESHOLD_FLAG);
}


u32int intcRegRead(u32int regOffs)
{
  u32int * regPtr = (u32int*)(intcBE->baseAddress | regOffs);
  volatile u32int value = *regPtr;
  return value;
}

void intcRegWrite(u32int regOffs, u32int value)
{
  volatile u32int * regPtr = (u32int*)(intcBE->baseAddress | regOffs);
  *regPtr = value;
}


void unmaskInterrupt(u32int interruptNumber)
{
  u32int bitMask = 0;
  u32int bankNumber = 0;

  if ((interruptNumber < 0) || (interruptNumber >= INTCPS_NR_OF_INTERRUPTS))
  {
    serial_ERROR("INTC_BE: mask interrupt number out of range.");
  }
  bankNumber = interruptNumber / INTCPS_INTERRUPTS_PER_BANK;
  bitMask = 1 << (interruptNumber % INTCPS_INTERRUPTS_PER_BANK);
  intcRegWrite(REG_INTCPS_MIR_CLEARn + bankNumber*0x20, bitMask);
}

void maskInterrupt(u32int interruptNumber)
{
  u32int bitMask = 0;
  u32int bankNumber = 0;

  if ((interruptNumber < 0) || (interruptNumber >= INTCPS_NR_OF_INTERRUPTS))
  {
    serial_ERROR("INTC_BE: mask interrupt number out of range.");
  }
  bankNumber = interruptNumber / INTCPS_INTERRUPTS_PER_BANK;
  bitMask = 1 << (interruptNumber % INTCPS_INTERRUPTS_PER_BANK);
  intcRegWrite(REG_INTCPS_MIR_SETn + bankNumber*0x20, bitMask);
}

u32int getIrqNumber()
{
  return intcRegRead(REG_INTCPS_SIR_IRQ) & INTCPS_SIR_IRQ_ACTIVEIRQ;
}

void acknowledgeIrq()
{
  intcRegWrite(REG_INTCPS_CONTROL, INTCPS_CONTROL_NEWIRQAGR);
}

void intcDumpRegisters()
{
  u32int indexN = 0, indexM = 0;
  
  serial_putstring("INTC: Revision ");
  serial_putint(intcRegRead(REG_INTCPS_REVISION));
  serial_newline();

  serial_putstring("INTC: sysconfig reg ");
  serial_putint(intcRegRead(REG_INTCPS_SYSCONFIG));
  serial_newline();

  serial_putstring("INTC: sysStatus reg ");
  serial_putint(intcRegRead(REG_INTCPS_SYSSTATUS));
  serial_newline();

  serial_putstring("INTC: current active irq reg ");
  serial_putint(intcRegRead(REG_INTCPS_SIR_IRQ));
  serial_newline();

  serial_putstring("INTC: current active fiq reg ");
  serial_putint(intcRegRead(REG_INTCPS_SIR_FIQ));
  serial_newline();

  serial_putstring("INTC: control reg ");
  serial_putint(intcRegRead(REG_INTCPS_CONTROL));
  serial_newline();

  serial_putstring("INTC: protection reg ");
  serial_putint(intcRegRead(REG_INTCPS_PROTECTION));
  serial_newline();

  serial_putstring("INTC: idle reg ");
  serial_putint(intcRegRead(REG_INTCPS_IDLE));
  serial_newline();

  serial_putstring("INTC: current active irq priority ");
  serial_putint(intcRegRead(REG_INTCPS_IRQ_PRIORITY));
  serial_newline();

  serial_putstring("INTC: current active fiq priority ");
  serial_putint(intcRegRead(REG_INTCPS_FIQ_PRIORITY));
  serial_newline();

  serial_putstring("INTC: priority threshold ");
  serial_putint(intcRegRead(REG_INTCPS_THRESHOLD));
  serial_newline();
//TODO: use this for timer test
  serial_putstring("INTC: interrupt status before masking:");
  serial_newline();
  for (indexN = 0; indexN < INTCPS_NR_OF_BANKS; indexN++)
  {
    serial_putint(intcRegRead(REG_INTCPS_ITRn + 0x20*indexN));
  }
  serial_newline();

  serial_putstring("INTC: interrupt mask:");
  serial_newline();
  for (indexN = 0; indexN < INTCPS_NR_OF_BANKS; indexN++)
  {
    serial_putint(intcRegRead(REG_INTCPS_MIRn + 0x20*indexN));
  }
  serial_newline();

  serial_putstring("INTC: pending IRQ:");
  serial_newline();
  for (indexN = 0; indexN < INTCPS_NR_OF_BANKS; indexN++)
  {
    serial_putint(intcRegRead(REG_INTCPS_PENDING_IRQn + 0x20*indexN));
  }
  serial_newline();

  serial_putstring("INTC: pending FIQ:");
  serial_newline();
  for (indexN = 0; indexN < INTCPS_NR_OF_BANKS; indexN++)
  {
    serial_putint(intcRegRead(REG_INTCPS_PENDING_FIQn + 0x20*indexN));
  }
  serial_newline();

  serial_putstring("INTC: interrupt steering/priority dump:");
  serial_newline();
  for (indexM = 0; indexM < INTCPS_NR_OF_INTERRUPTS/8; indexM++)
  {
    serial_putint(intcRegRead(REG_INTCPS_ILRm + 0x20*indexM));
    serial_putstring(" ");
    serial_putint(intcRegRead(REG_INTCPS_ILRm + 0x4 + 0x20*indexM));
    serial_putstring(" ");
    serial_putint(intcRegRead(REG_INTCPS_ILRm + 0x8 + 0x20*indexM));
    serial_putstring(" ");
    serial_putint(intcRegRead(REG_INTCPS_ILRm + 0xc + 0x20*indexM));
    serial_putstring(" ");
    serial_putint(intcRegRead(REG_INTCPS_ILRm + 0x10 + 0x20*indexM));
    serial_putstring(" ");
    serial_putint(intcRegRead(REG_INTCPS_ILRm + 0x14 + 0x20*indexM));
    serial_putstring(" ");
    serial_putint(intcRegRead(REG_INTCPS_ILRm + 0x18 + 0x20*indexM));
    serial_putstring(" ");
    serial_putint(intcRegRead(REG_INTCPS_ILRm + 0x1c + 0x20*indexM));
    serial_newline();
  }
}

