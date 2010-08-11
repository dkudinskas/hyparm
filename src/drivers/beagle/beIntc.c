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
  intcBE->intcRevision = (u32int*)( MPU_INTC + REG_INTCPS_REVISION);
  intcBE->intcSysConfig = (u32int*)( MPU_INTC + REG_INTCPS_SYSCONFIG);
  intcBE->intcSysStatus = (u32int*)( MPU_INTC + REG_INTCPS_SYSSTATUS);
  intcBE->intcSirIrq = (u32int*)( MPU_INTC + REG_INTCPS_SIR_IRQ);
  intcBE->intcSirFiq = (u32int*)( MPU_INTC + REG_INTCPS_SIR_FIQ);
  intcBE->intcControl = (u32int*)( MPU_INTC + REG_INTCPS_CONTROL);
  intcBE->intcProtection = (u32int*)( MPU_INTC + REG_INTCPS_PROTECTION);
  intcBE->intcIdle = (u32int*)( MPU_INTC + REG_INTCPS_IDLE);
  intcBE->intcIrqPriority = (u32int*)( MPU_INTC + REG_INTCPS_IRQ_PRIORITY);
  intcBE->intcFiqPriority = (u32int*)( MPU_INTC + REG_INTCPS_FIQ_PRIORITY);
  intcBE->intcThreshold = (u32int*)( MPU_INTC + REG_INTCPS_THRESHOLD);
  intcBE->intcItrN = (u32int*)( MPU_INTC + REG_INTCPS_ITRn);
  intcBE->intcMirN = (u32int*)( MPU_INTC + REG_INTCPS_MIRn);
  intcBE->intcMirClearN = (u32int*)( MPU_INTC + REG_INTCPS_MIR_CLEARn);
  intcBE->intcMirSetN = (u32int*)( MPU_INTC + REG_INTCPS_MIR_SETn);
  intcBE->intcIsrSetN = (u32int*)( MPU_INTC + REG_INTCPS_ISR_SETn);
  intcBE->intcIsrClearN = (u32int*)( MPU_INTC + REG_INTCPS_ISR_CLEARn);
  intcBE->intcPendingIrqN = (u32int*)( MPU_INTC + REG_INTCPS_PENDING_IRQn);
  intcBE->intcPendingFiqN = (u32int*)( MPU_INTC + REG_INTCPS_PENDING_FIQn);
  intcBE->intcIlrM = (u32int*)( MPU_INTC + REG_INTCPS_ILRm);

  u32int * regPtr = 0;
  u32int i = 0, m = 0;

  // set register access protection (priviledged modes only)!
  regPtr = (u32int*)(MPU_INTC | REG_INTCPS_PROTECTION);
  *regPtr = INTCPS_PROTECTION_PROTECTION;
  // mask interrupts (all)
  for (i = 0; i < INTCPS_NR_OF_BANKS; i++)
  {
    regPtr = (u32int*)(MPU_INTC | (REG_INTCPS_MIR_SETn + 0x20*i));
    *regPtr = INTCPS_MIR_SETn_MIRSET;
  }
  // set all priorities to 0x0 (arbitrary)
  for (m = 0; m < INTCPS_NR_OF_INTERRUPTS; m++)
  {
    regPtr = (u32int*)(MPU_INTC | (REG_INTCPS_ILRm + m*0x4));
    *regPtr = 0x0;
  }

  // disable interrupt priority threshold
  regPtr = (u32int*)(MPU_INTC | REG_INTCPS_THRESHOLD);
  *regPtr = INTCPS_THRESHOLD_FLAG;

/*
  regPtr = (u32int*)(MPU_INTC | REG_INTCPS_PROTECTION);
  *regPtr = INTCPS_PROTECTION_PROTECTION;

  // mask interrupts (all)
  for (i = 0; i < INTCPS_NR_OF_BANKS; i++)
  {
    regPtr = (u32int*)(MPU_INTC | (REG_INTCPS_MIR_SETn + 0x20*i));
    *regPtr = INTCPS_MIR_SETn_MIRSET;
  }

  // set all priorities to 0x10 (arbitrary)
  prioritySteering = 0x0 << 2;
  for (m = 0; m < INTCPS_NR_OF_INTERRUPTS; m++)
  {
    regPtr = (u32int*)(MPU_INTC | (REG_INTCPS_ILRm + m*0x4));
    *regPtr = prioritySteering;
  }

  // disable interrupt priority threshold
  regPtr = (u32int*)(MPU_INTC | REG_INTCPS_THRESHOLD);
  *regPtr = 0xFF;
*/
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

void intcDumpRegisters()
{
  u32int indexN = 0, indexM = 0;
  
  serial_putstring("INTC: Revision ");
  serial_putint(*(intcBE->intcRevision) );
  serial_newline();

  serial_putstring("INTC: sysconfig reg ");
  serial_putint(*(intcBE->intcSysConfig) );
  serial_newline();

  serial_putstring("INTC: intcSysStatus reg ");
  serial_putint(*(intcBE->intcSysStatus) );
  serial_newline();

  serial_putstring("INTC: current active irq reg ");
  serial_putint(*(intcBE->intcSirIrq) );
  serial_newline();

  serial_putstring("INTC: current active fiq reg ");
  serial_putint(*(intcBE->intcSirFiq) );
  serial_newline();

  serial_putstring("INTC: control reg ");
  serial_putint(*(intcBE->intcControl) );
  serial_newline();

  serial_putstring("INTC: protection reg ");
  serial_putint(*(intcBE->intcProtection) );
  serial_newline();

  serial_putstring("INTC: idle reg ");
  serial_putint(*(intcBE->intcIdle) );
  serial_newline();

  serial_putstring("INTC: current active irq priority ");
  serial_putint(*(intcBE->intcIrqPriority) );
  serial_newline();

  serial_putstring("INTC: current active fiq priority ");
  serial_putint(*(intcBE->intcFiqPriority) );
  serial_newline();

  serial_putstring("INTC: priority threshold ");
  serial_putint(*(intcBE->intcThreshold) );
  serial_newline();

  serial_putstring("INTC: interrupt status before masking:");
  serial_newline();
  for (indexN = 0; indexN < INTCPS_NR_OF_BANKS; indexN++)
  {
    u32int regAddr = ((u32int)intcBE->intcItrN) + 0x20*indexN;
    serial_putint(*(u32int*)regAddr);
  }
  serial_newline();

  serial_putstring("INTC: interrupt mask:");
  serial_newline();
  for (indexN = 0; indexN < INTCPS_NR_OF_BANKS; indexN++)
  {
    u32int regAddr = ((u32int)intcBE->intcMirN) + 0x20*indexN;
    serial_putint(*(u32int*)regAddr);
  }
  serial_newline();

  serial_putstring("INTC: pending IRQ:");
  serial_newline();
  for (indexN = 0; indexN < INTCPS_NR_OF_BANKS; indexN++)
  {
    u32int regAddr = ((u32int)intcBE->intcPendingIrqN) + 0x20*indexN;
    serial_putint(*(u32int*)regAddr);
  }
  serial_newline();

  serial_putstring("INTC: pending FIQ:");
  serial_newline();
  for (indexN = 0; indexN < INTCPS_NR_OF_BANKS; indexN++)
  {
    u32int regAddr = ((u32int)intcBE->intcPendingFiqN) + 0x20*indexN;
    serial_putint(*(u32int*)regAddr);
  }
  serial_newline();

  serial_putstring("INTC: interrupt steering/priority dump:");
  serial_newline();
  for (indexM = 0; indexM < INTCPS_NR_OF_INTERRUPTS/8; indexM++)
  {
    u32int regAddr = ((u32int)intcBE->intcIlrM) + 0x20*indexM;
    serial_putint(*(u32int*)regAddr);
    serial_putstring(" ");
    regAddr = ((u32int)intcBE->intcIlrM) + 0x4 + 0x20*indexM;
    serial_putint(*(u32int*)regAddr);
    serial_putstring(" ");
    regAddr = ((u32int)intcBE->intcIlrM) + 0x8 + 0x20*indexM;
    serial_putint(*(u32int*)regAddr);
    serial_putstring(" ");
    regAddr = ((u32int)intcBE->intcIlrM) + 0xc + 0x20*indexM;
    serial_putint(*(u32int*)regAddr);
    serial_putstring(" ");
    regAddr = ((u32int)intcBE->intcIlrM) + 0x10 + 0x20*indexM;
    serial_putint(*(u32int*)regAddr);
    serial_putstring(" ");
    regAddr = ((u32int)intcBE->intcIlrM) + 0x14 + 0x20*indexM;
    serial_putint(*(u32int*)regAddr);
    serial_putstring(" ");
    regAddr = ((u32int)intcBE->intcIlrM) + 0x18 + 0x20*indexM;
    serial_putint(*(u32int*)regAddr);
    serial_putstring(" ");
    regAddr = ((u32int)intcBE->intcIlrM) + 0x1c + 0x20*indexM;
    serial_putint(*(u32int*)regAddr);
    serial_newline();
  }
}

/*
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
  for (indexN = INTCPS_NR_OF_BANKS-1; indexN >= 0 ; indexN--)
  {
    regPtr = MPU_INTC | (REG_INTCPS_ITRn + 0x20 * indexN);
    serial_putint(*((u32int*)regPtr));
  }
  serial_newline();

  serial_putstring("INTC: interrupt mask:");
  serial_newline();
  for (indexN = INTCPS_NR_OF_BANKS-1; indexN >= 0 ; indexN--)
  {
    regPtr = MPU_INTC | (REG_INTCPS_MIRn + 0x20 * indexN);
    serial_putint(*((u32int*)regPtr));
  }
  serial_newline();

  serial_putstring("INTC: pending IRQ:");
  serial_newline();
  for (indexN = INTCPS_NR_OF_BANKS-1; indexN >= 0 ; indexN--)
  {
    regPtr = MPU_INTC | (REG_INTCPS_PENDING_IRQn + 0x20 * indexN);
    serial_putint(*((u32int*)regPtr));
  }
  serial_newline();

  serial_putstring("INTC: pending FIQ:");
  serial_newline();
  for (indexN = INTCPS_NR_OF_BANKS-1; indexN >= 0 ; indexN--)
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
*/
