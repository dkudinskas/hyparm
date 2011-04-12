#include "common/debug.h"
#include "common/memFunctions.h"

#include "drivers/beagle/beIntc.h"
#include "drivers/beagle/beGPTimer.h"

static inline u32int intcRegReadBE(u32int regOffs);
static inline void intcRegWriteBE(u32int regOffs, u32int value);

struct InterruptControllerBE * intcBE;

void intcBEInit()
{
  intcBE = (struct InterruptControllerBE*)mallocBytes(sizeof(struct InterruptControllerBE));
  if (intcBE == 0)
  {
    DIE_NOW(0, "Failed to allocate INTC_BE.");
  }
  else
  {
    memset((void*)intcBE, 0x0, sizeof(struct InterruptControllerBE));
#ifdef BE_INTC_DBG
    printf("Initializing INTC_BE at %x\n", (u32int)intcBE);
#endif
  }

  intcBE->enabled = TRUE;
  intcBE->baseAddress = 0x48200000; // Section 10.4.1 p1200
  intcBE->size = 0x1000; // 4 KB

  u32int i = 0, m = 0;
  // soft reset
#ifdef BE_INTC_DBG
  printf("INTC_BE: soft reset ...");
#endif
  u32int conf = intcRegReadBE(REG_INTCPS_SYSCONFIG);
  conf |= INTCPS_SYSCONFIG_SOFTRESET;
  intcRegWriteBE(REG_INTCPS_SYSCONFIG, conf);

  while (!(intcRegReadBE(REG_INTCPS_SYSSTATUS) & INTCPS_SYSSTATUS_SOFTRESET))
  {
#ifdef BE_INTC_DBG
    printf(".");
#endif
  }
#ifdef BE_INTC_DBG
  printf(" done\n");
#endif
   
  // intc autoidle
  intcRegWriteBE(REG_INTCPS_SYSCONFIG, INTCPS_SYSCONFIG_AUTOIDLE);

  // set register access protection (priviledged modes only)!
  intcRegWriteBE(REG_INTCPS_PROTECTION, INTCPS_PROTECTION_PROTECTION);
  
  // mask interrupts (all)
  for (i = 0; i < INTCPS_NR_OF_BANKS; i++)
  {
    intcRegWriteBE(REG_INTCPS_MIR_SETn + 0x20*i, INTCPS_MIR_SETn_MIRSET);
  }
  // set all priorities to 0x0 (arbitrary)
  for (m = 0; m < INTCPS_NR_OF_INTERRUPTS; m++)
  {
    intcRegWriteBE(REG_INTCPS_ILRm+m*0x4, 0);
  }

  // disable interrupt priority threshold
  intcRegWriteBE(REG_INTCPS_THRESHOLD, INTCPS_THRESHOLD_FLAG);
}


static inline u32int intcRegReadBE(u32int regOffs)
{
  volatile u32int * regPtr = (u32int*)(intcBE->baseAddress | regOffs);
  return *regPtr;
}

static inline void intcRegWriteBE(u32int regOffs, u32int value)
{
  volatile u32int * regPtr = (u32int*)(intcBE->baseAddress | regOffs);
  *regPtr = value;
}


void unmaskInterruptBE(u32int interruptNumber)
{
  u32int bitMask = 0;
  u32int bankNumber = 0;

  if ((interruptNumber < 0) || (interruptNumber >= INTCPS_NR_OF_INTERRUPTS))
  {
    DIE_NOW(0, "INTC_BE: mask interrupt number out of range.");
  }
  bankNumber = interruptNumber / INTCPS_INTERRUPTS_PER_BANK;
  bitMask = 1 << (interruptNumber % INTCPS_INTERRUPTS_PER_BANK);
  intcRegWriteBE(REG_INTCPS_MIR_CLEARn + bankNumber*0x20, bitMask);
}

void maskInterruptBE(u32int interruptNumber)
{
  u32int bitMask = 0;
  u32int bankNumber = 0;

  if ((interruptNumber < 0) || (interruptNumber >= INTCPS_NR_OF_INTERRUPTS))
  {
    DIE_NOW(0, "INTC_BE: mask interrupt number out of range.");
  }
  bankNumber = interruptNumber / INTCPS_INTERRUPTS_PER_BANK;
  bitMask = 1 << (interruptNumber % INTCPS_INTERRUPTS_PER_BANK);
  intcRegWriteBE(REG_INTCPS_MIR_SETn + bankNumber*0x20, bitMask);
}

u32int getIrqNumberBE()
{
  return intcRegReadBE(REG_INTCPS_SIR_IRQ) & INTCPS_SIR_IRQ_ACTIVEIRQ;
}

void acknowledgeIrqBE()
{
  intcRegWriteBE(REG_INTCPS_CONTROL, INTCPS_CONTROL_NEWIRQAGR);
}

void intcDumpRegistersBE()
{
  u32int indexN = 0, indexM = 0;
  
  printf("INTC_BE: Revision %08x\n", intcRegReadBE(REG_INTCPS_REVISION));
  printf("INTC_BE: sysconfig reg %08x\n", intcRegReadBE(REG_INTCPS_SYSCONFIG));
  printf("INTC_BE: sysStatus reg %08x\n", intcRegReadBE(REG_INTCPS_SYSSTATUS));
  printf("INTC_BE: current active irq reg %08x\n", intcRegReadBE(REG_INTCPS_SIR_IRQ));
  printf("INTC_BE: current active fiq reg %08x\n", intcRegReadBE(REG_INTCPS_SIR_FIQ));
  printf("INTC_BE: control reg %08x\n", intcRegReadBE(REG_INTCPS_CONTROL));
  printf("INTC_BE: protection reg %08x\n", intcRegReadBE(REG_INTCPS_PROTECTION));
  printf("INTC_BE: idle reg %08x\n", intcRegReadBE(REG_INTCPS_IDLE));
  printf("INTC_BE: current active irq priority %08x\n", intcRegReadBE(REG_INTCPS_IRQ_PRIORITY));
  printf("INTC_BE: current active fiq priority %08x\n", intcRegReadBE(REG_INTCPS_FIQ_PRIORITY));
  printf("INTC_BE: priority threshold %08x\n", intcRegReadBE(REG_INTCPS_THRESHOLD));
  printf("INTC_BE: interrupt status before masking:\n");
  for (indexN = 0; indexN < INTCPS_NR_OF_BANKS; indexN++)
  {
    printf("%08x", intcRegReadBE(REG_INTCPS_ITRn + 0x20*indexN));
  }
  printf("\n");

  printf("INTC_BE: interrupt mask:\n");
  for (indexN = 0; indexN < INTCPS_NR_OF_BANKS; indexN++)
  {
    printf("%08x", intcRegReadBE(REG_INTCPS_MIRn + 0x20*indexN));
  }
  printf("\n");

  printf("INTC_BE: pending IRQ:\n");
  for (indexN = 0; indexN < INTCPS_NR_OF_BANKS; indexN++)
  {
    printf("%08x", intcRegReadBE(REG_INTCPS_PENDING_IRQn + 0x20*indexN));
  }
  printf("\n");

  printf("INTC_BE: pending FIQ:\n");
  for (indexN = 0; indexN < INTCPS_NR_OF_BANKS; indexN++)
  {
    printf("%08x", intcRegReadBE(REG_INTCPS_PENDING_FIQn + 0x20*indexN));
  }
  printf("\n");

  printf("INTC_BE: interrupt steering/priority dump:\n");
  for (indexM = 0; indexM < INTCPS_NR_OF_INTERRUPTS/8; indexM++)
  {
    printf("%08x", intcRegReadBE(REG_INTCPS_ILRm + 0x20*indexM));
    printf(" ");
    printf("%08x", intcRegReadBE(REG_INTCPS_ILRm + 0x4 + 0x20*indexM));
    printf(" ");
    printf("%08x", intcRegReadBE(REG_INTCPS_ILRm + 0x8 + 0x20*indexM));
    printf(" ");
    printf("%08x", intcRegReadBE(REG_INTCPS_ILRm + 0xc + 0x20*indexM));
    printf(" ");
    printf("%08x", intcRegReadBE(REG_INTCPS_ILRm + 0x10 + 0x20*indexM));
    printf(" ");
    printf("%08x", intcRegReadBE(REG_INTCPS_ILRm + 0x14 + 0x20*indexM));
    printf(" ");
    printf("%08x", intcRegReadBE(REG_INTCPS_ILRm + 0x18 + 0x20*indexM));
    printf(" ");
    printf("%08x", intcRegReadBE(REG_INTCPS_ILRm + 0x1c + 0x20*indexM));
    printf("\n");
  }
}

