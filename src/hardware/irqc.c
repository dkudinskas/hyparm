#include "irqc.h"
#include "serial.h"
#include "gptimer.h"

void intcDumpRegisters()
{
  u32int indexN = 0;
  u32int regPtr = 0;

  regPtr = MPU_INTC | REG_INTCPS_SYSCONFIG;
  serial_putstring("INTC: sysconfig reg ");
  serial_putint(*((u32int*)regPtr) );
  serial_newline();

  regPtr = MPU_INTC | REG_INTCPS_SYSSTATUS;
  serial_putstring("INTC: status reg ");
  serial_putint(*((u32int*)regPtr));
  serial_newline();

  regPtr = MPU_INTC | REG_INTCPS_SIR_IRQ;
  serial_putstring("INTC: current active irq reg ");
  serial_putint(*((u32int*)regPtr));
  serial_newline();

  regPtr = MPU_INTC | REG_INTCPS_SIR_FIQ;
  serial_putstring("INTC: current active fiq reg ");
  serial_putint(*((u32int*)regPtr));
  serial_newline();

  regPtr = MPU_INTC | REG_INTCPS_CONTROL;
  serial_putstring("INTC: control reg ");
  serial_putint(*((u32int*)regPtr));
  serial_newline();

  regPtr = MPU_INTC | REG_INTCPS_PROTECTION;
  serial_putstring("INTC: protection reg ");
  serial_putint(*((u32int*)regPtr));
  serial_newline();

  regPtr = MPU_INTC | REG_INTCPS_IDLE;
  serial_putstring("INTC: idle reg ");
  serial_putint(*((u32int*)regPtr));
  serial_newline();

  regPtr = MPU_INTC | REG_INTCPS_IRQ_PRIORITY;
  serial_putstring("INTC: current active irq priority ");
  serial_putint(*((u32int*)regPtr));
  serial_newline();

  regPtr = MPU_INTC | REG_INTCPS_FIQ_PRIORITY;
  serial_putstring("INTC: current active fiq priority ");
  serial_putint(*((u32int*)regPtr));
  serial_newline();

  regPtr = MPU_INTC | REG_INTCPS_THRESHOLD;
  serial_putstring("INTC: priority threshold ");
  serial_putint(*((u32int*)regPtr));
  serial_newline();

  serial_putstring("INTC: interrupt status before masking:");
  serial_newline();
  for (indexN = 0; indexN < INTCPS_NR_OF_BANKS; indexN++)
  {
    regPtr = MPU_INTC | (REG_INTCPS_ITRn + 0x20 * indexN);
    serial_putint(*((u32int*)regPtr));
  }
  serial_newline();

  serial_putstring("INTC: interrupt mask:");
  serial_newline();
  for (indexN = 0; indexN < INTCPS_NR_OF_BANKS; indexN++)
  {
    regPtr = MPU_INTC | (REG_INTCPS_MIRn + 0x20 * indexN);
    serial_putint(*((u32int*)regPtr));
  }
  serial_newline();

  serial_putstring("INTC: pending IRQ:");
  serial_newline();
  for (indexN = 0; indexN < INTCPS_NR_OF_BANKS; indexN++)
  {
    regPtr = MPU_INTC | (REG_INTCPS_PENDING_IRQn + 0x20 * indexN);
    serial_putint(*((u32int*)regPtr));
  }
  serial_newline();

  serial_putstring("INTC: pending FIQ:");
  serial_newline();
  for (indexN = 0; indexN < INTCPS_NR_OF_BANKS; indexN++)
  {
    regPtr = MPU_INTC | (REG_INTCPS_PENDING_FIQn + 0x20 * indexN);
    serial_putint(*((u32int*)regPtr));
  }
  serial_newline();

  serial_putstring("INTC: interrupt steering/priority dump:");
  serial_newline();
  for (indexN = 0; indexN < INTCPS_NR_OF_INTERRUPTS/8; indexN++)
  {
    regPtr = MPU_INTC | (REG_INTCPS_ILRm+indexN*0x20);
    serial_putint(*((u32int*)regPtr));
    serial_putstring(" ");
    regPtr = MPU_INTC | (REG_INTCPS_ILRm+0x4+indexN*0x20);
    serial_putint(*((u32int*)regPtr));
    serial_putstring(" ");
    regPtr = MPU_INTC | (REG_INTCPS_ILRm+0x8+indexN*0x20);
    serial_putint(*((u32int*)regPtr));
    serial_putstring(" ");
    regPtr = MPU_INTC | (REG_INTCPS_ILRm+0xC+indexN*0x20);
    serial_putint(*((u32int*)regPtr));
    serial_putstring(" ");
    regPtr = MPU_INTC | (REG_INTCPS_ILRm+0x10+indexN*0x20);
    serial_putint(*((u32int*)regPtr));
    serial_putstring(" ");
    regPtr = MPU_INTC | (REG_INTCPS_ILRm+0x14+indexN*0x20);
    serial_putint(*((u32int*)regPtr));
    serial_putstring(" ");
    regPtr = MPU_INTC | (REG_INTCPS_ILRm+0x18+indexN*0x20);
    serial_putint(*((u32int*)regPtr));
    serial_putstring(" ");
    regPtr = MPU_INTC | (REG_INTCPS_ILRm+0x1C+indexN*0x20);
    serial_putint(*((u32int*)regPtr));
    serial_newline();
  }
}

void initialiseInterruptController()
{
  int i = 0, m = 0;
  u32int prioritySteering = 0;
  u32int * regPtr = 0;

  serial_putstring("Initializing interrupt controller...");
  // set register access protection (priviledged modes only)!
  regPtr = (u32int*)(MPU_INTC | REG_INTCPS_PROTECTION);
  *regPtr = INTCPS_PROTECTION_PROTECTION;

  // mask interrupts (all)
  for (i = 0; i < INTCPS_NR_OF_BANKS; i++)
  {
    regPtr = (u32int*)(MPU_INTC | (REG_INTCPS_MIR_SETn + 0x20*i));
    *regPtr = INTCPS_MIR_SETn_MIRSET;
  }

  // set all priorities to 0x10 (arbitrary)
  prioritySteering = 0x10 << 2;
  for (m = 0; m < INTCPS_NR_OF_INTERRUPTS; m++)
  {
    regPtr = (u32int*)(MPU_INTC | (REG_INTCPS_ILRm + m*0x4));
    *regPtr = prioritySteering;
  }

  // disable interrupt priority threshold
  regPtr = (u32int*)(MPU_INTC | REG_INTCPS_THRESHOLD);
  *regPtr = 0xFF;

  // use GPTimer10 (out of 1/2/10 capable of doing 1ms ticks)
  setupMsTick(10);

  serial_putstring("done");
  serial_newline();
}


void unmaskInterrupt(u32int interruptNumber)
{
  u32int * regPtr = 0;
  u32int bitMask = 0;
  u32int bankNumber = 0;

  bankNumber = interruptNumber / INTCPS_INTERRUPTS_PER_BANK;
  bitMask = 1 << (interruptNumber % INTCPS_INTERRUPTS_PER_BANK);

  regPtr = (u32int*)((MPU_INTC) | (REG_INTCPS_MIR_CLEARn + bankNumber*0x20));
  *regPtr = bitMask;
}


void maskInterrupt(u32int interruptNumber)
{
  u32int * regPtr = 0;
  u32int bitMask = 0;
  u32int bankNumber = 0;

  bankNumber = interruptNumber / INTCPS_INTERRUPTS_PER_BANK;
  bitMask = 1 << (interruptNumber % INTCPS_INTERRUPTS_PER_BANK);

  regPtr = (u32int*)((MPU_INTC) | (REG_INTCPS_MIR_SETn + bankNumber*0x20));
  *regPtr = bitMask;
}

u32int getIrqNumber()
{
  u32int irqNumber = 0;
  u32int * regPtr = 0;
  regPtr = (u32int*)(MPU_INTC | REG_INTCPS_SIR_IRQ);
  irqNumber = *regPtr & INTCPS_SIR_IRQ_ACTIVEIRQ;
  return irqNumber;
}

void resetAndNewIrq()
{
  u32int * regPtr = 0;
  regPtr = (u32int*)(MPU_INTC | REG_INTCPS_CONTROL);
  *regPtr = INTCPS_CONTROL_NEWIRQAGR;
}

