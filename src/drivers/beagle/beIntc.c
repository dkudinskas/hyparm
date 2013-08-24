#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "drivers/beagle/beIntc.h"


struct InterruptControllerBE
{
  bool enabled;
  u32int baseAddress;
  u32int size;
  /* add more if needed */
};

static struct InterruptControllerBE *intcBE;


static inline u32int intcRegReadBE(u32int regOffs);
static inline void intcRegWriteBE(u32int regOffs, u32int value);


void intcBEInit()
{
  intcBE = (struct InterruptControllerBE *)calloc(1, sizeof(struct InterruptControllerBE));
  if (intcBE == 0)
  {
    DIE_NOW(NULL, "Failed to allocate INTC_BE.");
  }

  DEBUG(PP_OMAP_35XX_INTC, "Initializing INTC_BE at %p" EOL, intcBE);

  intcBE->enabled = TRUE;
  intcBE->baseAddress = 0x48200000; // Section 10.4.1 p1200
  intcBE->size = 0x1000; // 4 KB

  DEBUG(PP_OMAP_35XX_INTC, "INTC_BE: soft reset");

  u32int conf = intcRegReadBE(REG_INTCPS_SYSCONFIG);
  conf |= INTCPS_SYSCONFIG_SOFTRESET;

  intcRegWriteBE(REG_INTCPS_SYSCONFIG, conf);

  while (!(intcRegReadBE(REG_INTCPS_SYSSTATUS) & INTCPS_SYSSTATUS_SOFTRESET))
  {
    DEBUG(PP_OMAP_35XX_INTC, ".");
  }
  DEBUG(PP_OMAP_35XX_INTC, " done" EOL);

  // intc autoidle
  intcRegWriteBE(REG_INTCPS_SYSCONFIG, INTCPS_SYSCONFIG_AUTOIDLE);

#ifdef CONFIG_HW_PASSTHROUGH
  // allow user mode access to interrupt controller
  intcRegWriteBE(REG_INTCPS_PROTECTION, 0);
#else
  // set register access protection (priviledged modes only)!
  intcRegWriteBE(REG_INTCPS_PROTECTION, INTCPS_PROTECTION_PROTECTION);
#endif

  int i;
  // mask interrupts (all)
  for (i = 0; i < INTCPS_NR_OF_BANKS; i++)
  {
    intcRegWriteBE(REG_INTCPS_MIR_SETn + 0x20*i, INTCPS_MIR_SETn_MIRSET);
  }
  // set all priorities to 0x0 (arbitrary)
  for (i = 0; i < INTCPS_NR_OF_INTERRUPTS; i++)
  {
    intcRegWriteBE(REG_INTCPS_ILRm + i * 0x4, 0);
  }

  // disable interrupt priority threshold
  intcRegWriteBE(REG_INTCPS_THRESHOLD, INTCPS_THRESHOLD_FLAG);
}

static inline u32int intcRegReadBE(u32int regOffs)
{
  return *(volatile u32int *)(intcBE->baseAddress | regOffs);
}

static inline void intcRegWriteBE(u32int regOffs, u32int value)
{
  *(volatile u32int *)(intcBE->baseAddress | regOffs) = value;
}

void unmaskInterruptBE(u32int interruptNumber)
{
  if (interruptNumber >= INTCPS_NR_OF_INTERRUPTS)
  {
    DIE_NOW(NULL, "INTC_BE: mask interrupt number out of range.");
  }

  u32int bankNumber = interruptNumber / INTCPS_INTERRUPTS_PER_BANK;
  u32int bitMask = 1 << (interruptNumber % INTCPS_INTERRUPTS_PER_BANK);

  intcRegWriteBE(REG_INTCPS_MIR_CLEARn + bankNumber*0x20, bitMask);
}

void maskInterruptBE(u32int interruptNumber)
{
  if (interruptNumber >= INTCPS_NR_OF_INTERRUPTS)
  {
    DIE_NOW(NULL, "INTC_BE: mask interrupt number out of range.");
  }

  u32int bankNumber = interruptNumber / INTCPS_INTERRUPTS_PER_BANK;
  u32int bitMask = 1 << (interruptNumber % INTCPS_INTERRUPTS_PER_BANK);

  intcRegWriteBE(REG_INTCPS_MIR_SETn + bankNumber*0x20, bitMask);
}


u32int getIrqNumberBE()
{
  return intcRegReadBE(REG_INTCPS_SIR_IRQ) & INTCPS_SIR_IRQ_ACTIVEIRQ;
}

u32int getFiqNumberBE()
{
  return intcRegReadBE(REG_INTCPS_SIR_FIQ) & INTCPS_SIR_FIQ_ACTIVEFIQ;
}


void acknowledgeIrqBE()
{
  intcRegWriteBE(REG_INTCPS_CONTROL, INTCPS_CONTROL_NEWIRQAGR);
}

void acknowledgeFiqBE()
{
  intcRegWriteBE(REG_INTCPS_CONTROL, INTCPS_CONTROL_NEWFIQAGR);
}


void setInterruptMapping(u32int interruptNumber, u32int fiq)
{
  u32int reg = intcRegReadBE(REG_INTCPS_ILRm + interruptNumber * 0x4);
  reg = (fiq) ? (reg | INTCPS_FIQNIRQ_FIQ) : (reg & (~INTCPS_FIQNIRQ_FIQ));
  intcRegWriteBE(REG_INTCPS_ILRm + interruptNumber * 0x4,  reg);
}


void intcDumpRegistersBE()
{
  u32int indexN = 0;

  printf("INTC_BE: Revision %#.8x" EOL, intcRegReadBE(REG_INTCPS_REVISION));
  printf("INTC_BE: sysconfig reg %#.8x" EOL, intcRegReadBE(REG_INTCPS_SYSCONFIG));
  printf("INTC_BE: sysStatus reg %#.8x" EOL, intcRegReadBE(REG_INTCPS_SYSSTATUS));
  printf("INTC_BE: current active irq reg %#.8x" EOL, intcRegReadBE(REG_INTCPS_SIR_IRQ));
  printf("INTC_BE: current active fiq reg %#.8x" EOL, intcRegReadBE(REG_INTCPS_SIR_FIQ));
  printf("INTC_BE: control reg %#.8x" EOL, intcRegReadBE(REG_INTCPS_CONTROL));
  printf("INTC_BE: protection reg %#.8x" EOL, intcRegReadBE(REG_INTCPS_PROTECTION));
  printf("INTC_BE: idle reg %#.8x" EOL, intcRegReadBE(REG_INTCPS_IDLE));
  printf("INTC_BE: current active irq priority %#.8x" EOL, intcRegReadBE(REG_INTCPS_IRQ_PRIORITY));
  printf("INTC_BE: current active fiq priority %#.8x" EOL, intcRegReadBE(REG_INTCPS_FIQ_PRIORITY));
  printf("INTC_BE: priority threshold %#.8x" EOL, intcRegReadBE(REG_INTCPS_THRESHOLD));
  printf("INTC_BE: interrupt status before masking:" EOL);

  for (indexN = 0; indexN < INTCPS_NR_OF_BANKS; indexN++)
  {
    printf("%#.8x ", intcRegReadBE(REG_INTCPS_ITRn + 0x20*indexN));
  }
  printf(EOL);

  printf("INTC_BE: interrupt mask:" EOL);
  for (indexN = 0; indexN < INTCPS_NR_OF_BANKS; indexN++)
  {
    printf("%#.8x ", intcRegReadBE(REG_INTCPS_MIRn + 0x20*indexN));
  }
  printf(EOL);

  printf("INTC_BE: pending IRQ:" EOL);
  for (indexN = 0; indexN < INTCPS_NR_OF_BANKS; indexN++)
  {
    printf("%#.8x ", intcRegReadBE(REG_INTCPS_PENDING_IRQn + 0x20*indexN));
  }
  printf(EOL);

  printf("INTC_BE: pending FIQ:" EOL);
  for (indexN = 0; indexN < INTCPS_NR_OF_BANKS; indexN++)
  {
    printf("%#.8x ", intcRegReadBE(REG_INTCPS_PENDING_FIQn + 0x20*indexN));
  }
  printf(EOL);

  printf("INTC_BE: interrupt steering/priority dump:" EOL);
  for (indexN = 0; indexN < INTCPS_NR_OF_INTERRUPTS/8; indexN++)
  {
    printf("%#.8x ", intcRegReadBE(REG_INTCPS_ILRm + 0x20*indexN));
    printf("%#.8x ", intcRegReadBE(REG_INTCPS_ILRm + 0x4 + 0x20*indexN));
    printf("%#.8x ", intcRegReadBE(REG_INTCPS_ILRm + 0x8 + 0x20*indexN));
    printf("%#.8x ", intcRegReadBE(REG_INTCPS_ILRm + 0xc + 0x20*indexN));
    printf("%#.8x ", intcRegReadBE(REG_INTCPS_ILRm + 0x10 + 0x20*indexN));
    printf("%#.8x ", intcRegReadBE(REG_INTCPS_ILRm + 0x14 + 0x20*indexN));
    printf("%#.8x ", intcRegReadBE(REG_INTCPS_ILRm + 0x18 + 0x20*indexN));
    printf("%#.8x" EOL, intcRegReadBE(REG_INTCPS_ILRm + 0x1c + 0x20*indexN));
  }
}
