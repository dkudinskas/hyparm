#include "common/debug.h"
#include "common/memFunctions.h"

#include "drivers/beagle/beIntc.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/intc.h"

#include "memoryManager/memoryConstants.h" // for BEAGLE_RAM_START/END
#include "memoryManager/pageTable.h" // for getPhysicalAddress()


extern GCONTXT * getGuestContext(void);

struct InterruptController * irqController;

void initIntc(void)
{
  irqController = (struct InterruptController*)mallocBytes(sizeof(struct InterruptController));
  if (irqController == 0)
  {
    DIE_NOW(0, "Failed to allocate INTC.");
  }
  else
  {
    memset((void*)irqController, 0x0, sizeof(struct InterruptController));
#ifdef INTC_DBG
    DEBUG_STRING("Initializing Interrupt controller at 0x");
    DEBUG_INT((u32int)irqController);
    DEBUG_NEWLINE();
#endif
  }
  intcReset();
}

/* top load function */
u32int loadIntc(device * dev, ACCESS_SIZE size, u32int address)
{
  //We care about the real pAddr of the entry, not its vAddr
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

  if (size != WORD)
  {
    // only word access allowed in these modules
    DIE_NOW(0, "Intc: invalid access size.");
  }

  u32int regOffset = phyAddr - INTERRUPT_CONTROLLER;
  u32int val = 0;
  switch (regOffset)
  {
    case REG_INTCPS_REVISION:
      val = INTC_REVISION;
      break;
    case REG_INTCPS_SYSCONFIG:
      val = irqController->intcSysConfig;
      break;
    case REG_INTCPS_SYSSTATUS:
      val = irqController->intcSysStatus;
      // check reset done flag - it auto clears...
      if (irqController->intcSysStatus & INTCPS_SYSSTATUS_SOFTRESET)
      {
        irqController->intcSysStatus = irqController->intcSysStatus & ~INTCPS_SYSSTATUS_SOFTRESET;
      }
      break;
    case REG_INTCPS_MIR_CLEAR0:
      DIE_NOW(0, "INTC: load from W/O register (MIR0_CLEAR)");
      break;
    case REG_INTCPS_MIR_CLEAR1:
      DIE_NOW(0, "INTC: load from W/O register (MIR1_CLEAR)");
      break;
    case REG_INTCPS_MIR_CLEAR2:
      DIE_NOW(0, "INTC: load from W/O register (MIR2_CLEAR)");
      break;
    case REG_INTCPS_MIR_SET0:
      DIE_NOW(0, "INTC: load from W/O register (MIR0_SET)");
      break;
    case REG_INTCPS_MIR_SET1:
      DIE_NOW(0, "INTC: load from W/O register (MIR1_SET)");
      break;
    case REG_INTCPS_MIR_SET2:
      DIE_NOW(0, "INTC: load from W/O register (MIR2_SET)");
      break;
    case REG_INTCPS_ISR_CLEAR0:
      DIE_NOW(0, "INTC: load from W/O register (ISR0_CLEAR)");
      break;
    case REG_INTCPS_ISR_CLEAR1:
      DIE_NOW(0, "INTC: load from W/O register (ISR1_CLEAR)");
      break;
    case REG_INTCPS_ISR_CLEAR2:
      DIE_NOW(0, "INTC: load from W/O register (ISR2_CLEAR)");
      break;
    case REG_INTCPS_PENDING_IRQ0:
      val = irqController->intcPendingIrq0;
#ifdef INTC_DBG
      DEBUG_STRING("INTC: load pending irq0 value ");
      DEBUG_INT(val);
      DEBUG_NEWLINE();
#endif
      break;
    case REG_INTCPS_PENDING_IRQ1:
      val = irqController->intcPendingIrq1;
#ifdef INTC_DBG
      DEBUG_STRING("INTC: load pending irq1 value ");
      DEBUG_INT(val);
      DEBUG_NEWLINE();
#endif
      break;
    case REG_INTCPS_PENDING_IRQ2:
      val = irqController->intcPendingIrq2;
#ifdef INTC_DBG
      DEBUG_STRING("INTC: load pending irq2 value ");
      DEBUG_INT(val);
      DEBUG_NEWLINE();
#endif
      break;
    case REG_INTCPS_SIR_IRQ:
      val = prioritySortIrqs();
      break;
    case REG_INTCPS_CONTROL:
      val = irqController->intcControl & INTCPS_CONTROL_RESERVED;
      break;
    case REG_INTCPS_SIR_FIQ:
    case REG_INTCPS_PROTECTION:
    case REG_INTCPS_IDLE:
    case REG_INTCPS_IRQ_PRIORITY:
    case REG_INTCPS_FIQ_PRIORITY:
    case REG_INTCPS_THRESHOLD:
    case REG_INTCPS_ITR0:
    case REG_INTCPS_ITR1:
    case REG_INTCPS_ITR2:
    case REG_INTCPS_ISR_SET0:
    case REG_INTCPS_ISR_SET1:
    case REG_INTCPS_ISR_SET2:
    case REG_INTCPS_PENDING_FIQ0:
    case REG_INTCPS_PENDING_FIQ1:
    case REG_INTCPS_PENDING_FIQ2:
    case REG_INTCPS_ILR0:
    case REG_INTCPS_ILR1:
    case REG_INTCPS_ILR2:
    case REG_INTCPS_ILR3:
    case REG_INTCPS_ILR4:
    case REG_INTCPS_ILR5:
    case REG_INTCPS_ILR6:
    case REG_INTCPS_ILR7:
    case REG_INTCPS_ILR8:
    case REG_INTCPS_ILR9:
    case REG_INTCPS_ILR10:
    case REG_INTCPS_ILR11:
    case REG_INTCPS_ILR12:
    case REG_INTCPS_ILR13:
    case REG_INTCPS_ILR14:
    case REG_INTCPS_ILR15:
    case REG_INTCPS_ILR16:
    case REG_INTCPS_ILR17:
    case REG_INTCPS_ILR18:
    case REG_INTCPS_ILR19:
    case REG_INTCPS_ILR20:
    case REG_INTCPS_ILR21:
    case REG_INTCPS_ILR22:
    case REG_INTCPS_ILR23:
    case REG_INTCPS_ILR24:
    case REG_INTCPS_ILR25:
    case REG_INTCPS_ILR26:
    case REG_INTCPS_ILR27:
    case REG_INTCPS_ILR28:
    case REG_INTCPS_ILR29:
    case REG_INTCPS_ILR30:
    case REG_INTCPS_ILR31:
    case REG_INTCPS_ILR32:
    case REG_INTCPS_ILR33:
    case REG_INTCPS_ILR34:
    case REG_INTCPS_ILR35:
    case REG_INTCPS_ILR36:
    case REG_INTCPS_ILR37:
    case REG_INTCPS_ILR38:
    case REG_INTCPS_ILR39:
    case REG_INTCPS_ILR40:
    case REG_INTCPS_ILR42:
    case REG_INTCPS_ILR43:
    case REG_INTCPS_ILR44:
    case REG_INTCPS_ILR45:
    case REG_INTCPS_ILR47:
    case REG_INTCPS_ILR48:
    case REG_INTCPS_ILR49:
    case REG_INTCPS_ILR50:
    case REG_INTCPS_ILR51:
    case REG_INTCPS_ILR52:
    case REG_INTCPS_ILR53:
    case REG_INTCPS_ILR54:
    case REG_INTCPS_ILR56:
    case REG_INTCPS_ILR57:
    case REG_INTCPS_ILR58:
    case REG_INTCPS_ILR59:
    case REG_INTCPS_ILR60:
    case REG_INTCPS_ILR61:
    case REG_INTCPS_ILR62:
    case REG_INTCPS_ILR63:
    case REG_INTCPS_ILR64:
    case REG_INTCPS_ILR65:
    case REG_INTCPS_ILR66:
    case REG_INTCPS_ILR67:
    case REG_INTCPS_ILR68:
    case REG_INTCPS_ILR69:
    case REG_INTCPS_ILR70:
    case REG_INTCPS_ILR71:
    case REG_INTCPS_ILR72:
    case REG_INTCPS_ILR73:
    case REG_INTCPS_ILR75:
    case REG_INTCPS_ILR76:
    case REG_INTCPS_ILR77:
    case REG_INTCPS_ILR78:
    case REG_INTCPS_ILR79:
    case REG_INTCPS_ILR80:
    case REG_INTCPS_ILR81:
    case REG_INTCPS_ILR82:
    case REG_INTCPS_ILR83:
    case REG_INTCPS_ILR84:
    case REG_INTCPS_ILR85:
    case REG_INTCPS_ILR86:
    case REG_INTCPS_ILR87:
    case REG_INTCPS_ILR88:
    case REG_INTCPS_ILR89:
    case REG_INTCPS_ILR90:
    case REG_INTCPS_ILR91:
    case REG_INTCPS_ILR92:
    case REG_INTCPS_ILR93:
    case REG_INTCPS_ILR94:
    case REG_INTCPS_ILR95:
      DEBUG_STRING("Intc: Unimplemted regsiter load.");
      DEBUG_STRING("register number ");
      DEBUG_INT(regOffset);
      DEBUG_NEWLINE();
      DIE_NOW(0, "PANIC");
      break;
    default:
      DIE_NOW(0, "Intc: load on invalid register.");
  }
  
#ifdef INTC_DBG
  DEBUG_STRING(dev->deviceName);
  DEBUG_STRING(" load from pAddr: 0x");
  DEBUG_INT(phyAddr);
  DEBUG_STRING(", vAddr: 0x");
  DEBUG_INT(address);
  DEBUG_STRING(" access size ");
  DEBUG_INT((u32int)size);
  DEBUG_STRING(" val = ");
  DEBUG_INT(val);
  DEBUG_NEWLINE();
#endif
  return val;
}


/* top store function */
void storeIntc(device * dev, ACCESS_SIZE size, u32int address, u32int value)
{
  //We care about the real pAddr of the entry, not its vAddr
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

#ifdef INTC_DBG
  DEBUG_STRING(dev->deviceName);
  DEBUG_STRING(" store to pAddr: 0x");
  DEBUG_INT(phyAddr);
  DEBUG_STRING(", vAddr: 0x");
  DEBUG_INT(address);
  DEBUG_STRING(" access size ");
  DEBUG_INT((u32int)size);
  DEBUG_STRING(" val ");
  DEBUG_INT(value);
  DEBUG_NEWLINE();
#endif

  if (size != WORD)
  {
    // only word access allowed in these modules
    DIE_NOW(0, "Intc: invalid access size.");
  }

  u32int regOffset = phyAddr - INTERRUPT_CONTROLLER;
  switch (regOffset)
  {
    case REG_INTCPS_REVISION:
      DIE_NOW(0, "Intc storing to read only register: version");
      break;
    case REG_INTCPS_SYSCONFIG:
      if (value & INTCPS_SYSCONFIG_SOFTRESET)
      {
#ifdef INTC_DBG
        DEBUG_STRING("INTC: soft reset.");
        DEBUG_NEWLINE();
#endif
        intcReset();
        value = value & ~INTCPS_SYSCONFIG_SOFTRESET;
      }
      // never set reset bit, all else except bit 0 is reserved
      irqController->intcSysConfig = value & INTCPS_SYSCONFIG_AUTOIDLE;
      break;
    case REG_INTCPS_SYSSTATUS:
      DIE_NOW(0, "Intc storing to read only register: system status");
      break;
    case REG_INTCPS_SIR_IRQ:
      DIE_NOW(0, "Intc storing to read only register: active irq");
      break;
    case REG_INTCPS_SIR_FIQ:
      DIE_NOW(0, "Intc storing to read only register: active fiq");
      break;
    case REG_INTCPS_MIR_CLEAR0:
    {
      u32int i;
      for (i = 0; i < 32; i++)
      {
        if (value & (1 << i))
        {
#ifdef INTC_DBG
          DEBUG_STRING("INTC: clearing mask from interrupt number ");
          DEBUG_INT_NOZEROS(i);
          DEBUG_NEWLINE();
#endif
        }
      }
    }
      irqController->intcMir0 &= ~value;
      break;
    case REG_INTCPS_MIR_CLEAR1:
    {
      u32int i;
      for (i = 0; i < 32; i++)
      {
        if (value & (1 << i))
        {
          // dedicate linux physical gpt2 for now, to serve as its core (gpt1)
          if (i+32 == GPT1_IRQ)
          {
            // linux unmasking gpt1 interrupt. unmask gpt2 in physical.
            unmaskInterruptBE(GPT2_IRQ);
          }
#ifdef INTC_DBG
          DEBUG_STRING("INTC: clearing mask from interrupt number ");
          DEBUG_INT_NOZEROS(i+32);
          DEBUG_NEWLINE();
#endif
        }
      }
    }
      irqController->intcMir1 &= ~value;
      break;
    case REG_INTCPS_MIR_CLEAR2:
    {
      u32int i;
      for (i = 0; i < 32; i++)
      {
        if (value & (1 << i))
        {
#ifdef INTC_DBG
          DEBUG_STRING("INTC: clearing mask from interrupt number ");
          DEBUG_INT_NOZEROS(i+64);
          DEBUG_NEWLINE();
#endif
        }
      }
    }
      irqController->intcMir2 &= ~value;
      break;
    case REG_INTCPS_MIR_SET0:
      irqController->intcMir0 |= value;
      irqController->intcPendingIrq0 = irqController->intcItr0 & ~irqController->intcMir0;
      break;
    case REG_INTCPS_MIR_SET1:
      irqController->intcMir1 |= value;
      irqController->intcPendingIrq1 = irqController->intcItr1 & ~irqController->intcMir1;
      break;
    case REG_INTCPS_MIR_SET2:
      irqController->intcMir2 |= value;
      irqController->intcPendingIrq2 = irqController->intcItr2 & ~irqController->intcMir2;
      break;
    case REG_INTCPS_CONTROL:
      irqController->intcControl = value & INTCPS_CONTROL_RESERVED;
      break;
    case REG_INTCPS_PROTECTION:
    case REG_INTCPS_IDLE:
    case REG_INTCPS_IRQ_PRIORITY:
    case REG_INTCPS_FIQ_PRIORITY:
    case REG_INTCPS_THRESHOLD:
    case REG_INTCPS_ITR0:
    case REG_INTCPS_ITR1:
    case REG_INTCPS_ITR2:
    case REG_INTCPS_MIR0:
    case REG_INTCPS_MIR1:
    case REG_INTCPS_MIR2:
    case REG_INTCPS_ISR_SET0:
    case REG_INTCPS_ISR_SET1:
    case REG_INTCPS_ISR_SET2:
    case REG_INTCPS_ISR_CLEAR0:
    case REG_INTCPS_ISR_CLEAR1:
    case REG_INTCPS_ISR_CLEAR2:
    case REG_INTCPS_PENDING_IRQ0:
    case REG_INTCPS_PENDING_IRQ1:
    case REG_INTCPS_PENDING_IRQ2:
    case REG_INTCPS_PENDING_FIQ0:
    case REG_INTCPS_PENDING_FIQ1:
    case REG_INTCPS_PENDING_FIQ2:
    case REG_INTCPS_ILR0:
    case REG_INTCPS_ILR1:
    case REG_INTCPS_ILR2:
    case REG_INTCPS_ILR3:
    case REG_INTCPS_ILR4:
    case REG_INTCPS_ILR5:
    case REG_INTCPS_ILR6:
    case REG_INTCPS_ILR7:
    case REG_INTCPS_ILR8:
    case REG_INTCPS_ILR9:
    case REG_INTCPS_ILR10:
    case REG_INTCPS_ILR11:
    case REG_INTCPS_ILR12:
    case REG_INTCPS_ILR13:
    case REG_INTCPS_ILR14:
    case REG_INTCPS_ILR15:
    case REG_INTCPS_ILR16:
    case REG_INTCPS_ILR17:
    case REG_INTCPS_ILR18:
    case REG_INTCPS_ILR19:
    case REG_INTCPS_ILR20:
    case REG_INTCPS_ILR21:
    case REG_INTCPS_ILR22:
    case REG_INTCPS_ILR23:
    case REG_INTCPS_ILR24:
    case REG_INTCPS_ILR25:
    case REG_INTCPS_ILR26:
    case REG_INTCPS_ILR27:
    case REG_INTCPS_ILR28:
    case REG_INTCPS_ILR29:
    case REG_INTCPS_ILR30:
    case REG_INTCPS_ILR31:
    case REG_INTCPS_ILR32:
    case REG_INTCPS_ILR33:
    case REG_INTCPS_ILR34:
    case REG_INTCPS_ILR35:
    case REG_INTCPS_ILR36:
    case REG_INTCPS_ILR37:
    case REG_INTCPS_ILR38:
    case REG_INTCPS_ILR39:
    case REG_INTCPS_ILR40:
    case REG_INTCPS_ILR42:
    case REG_INTCPS_ILR43:
    case REG_INTCPS_ILR44:
    case REG_INTCPS_ILR45:
    case REG_INTCPS_ILR47:
    case REG_INTCPS_ILR48:
    case REG_INTCPS_ILR49:
    case REG_INTCPS_ILR50:
    case REG_INTCPS_ILR51:
    case REG_INTCPS_ILR52:
    case REG_INTCPS_ILR53:
    case REG_INTCPS_ILR54:
    case REG_INTCPS_ILR56:
    case REG_INTCPS_ILR57:
    case REG_INTCPS_ILR58:
    case REG_INTCPS_ILR59:
    case REG_INTCPS_ILR60:
    case REG_INTCPS_ILR61:
    case REG_INTCPS_ILR62:
    case REG_INTCPS_ILR63:
    case REG_INTCPS_ILR64:
    case REG_INTCPS_ILR65:
    case REG_INTCPS_ILR66:
    case REG_INTCPS_ILR67:
    case REG_INTCPS_ILR68:
    case REG_INTCPS_ILR69:
    case REG_INTCPS_ILR70:
    case REG_INTCPS_ILR71:
    case REG_INTCPS_ILR72:
    case REG_INTCPS_ILR73:
    case REG_INTCPS_ILR75:
    case REG_INTCPS_ILR76:
    case REG_INTCPS_ILR77:
    case REG_INTCPS_ILR78:
    case REG_INTCPS_ILR79:
    case REG_INTCPS_ILR80:
    case REG_INTCPS_ILR81:
    case REG_INTCPS_ILR82:
    case REG_INTCPS_ILR83:
    case REG_INTCPS_ILR84:
    case REG_INTCPS_ILR85:
    case REG_INTCPS_ILR86:
    case REG_INTCPS_ILR87:
    case REG_INTCPS_ILR88:
    case REG_INTCPS_ILR89:
    case REG_INTCPS_ILR90:
    case REG_INTCPS_ILR91:
    case REG_INTCPS_ILR92:
    case REG_INTCPS_ILR93:
    case REG_INTCPS_ILR94:
    case REG_INTCPS_ILR95:
      DEBUG_STRING("Intc: Unimplemted regsiter store.");
      DEBUG_STRING("register number ");
      DEBUG_INT(regOffset);
      DEBUG_NEWLINE();
      DIE_NOW(0, "PANIC");
      break;
    default:
      DIE_NOW(0, "Intc: store on invalid register.");
  }
}

void intcReset()
{
  // reset all register values to defaults
  irqController->intcSysConfig   = 0x00000000;
  irqController->intcSysStatus   = 0x00000000;
  irqController->intcSirIrq      = 0xFFFFFF80;
  irqController->intcSirFiq      = 0xFFFFFF80;
  irqController->intcControl     = 0x00000000;
  irqController->intcProtection  = 0x00000000;
  irqController->intcIdle        = 0x00000000;
  irqController->intcIrqPriority = 0xFFFFFFC0;
  irqController->intcFiqPriority = 0xFFFFFFC0;
  irqController->intcThreshold   = 0x000000FF;
  irqController->intcItr0 = 0x0;
  irqController->intcItr1 = 0x0;
  irqController->intcItr2 = 0x0;
  irqController->intcMir0 = 0xFFFFFFFF;
  irqController->intcMir1 = 0xFFFFFFFF;
  irqController->intcMir2 = 0xFFFFFFFF;
  irqController->intcMirClear0 = 0x0;
  irqController->intcMirClear1 = 0x0;
  irqController->intcMirClear2 = 0x0;
  irqController->intcMirSet0 = 0x0;
  irqController->intcMirSet1 = 0x0;
  irqController->intcMirSet2 = 0x0;
  irqController->intcIsrSet0 = 0x0;
  irqController->intcIsrSet1 = 0x0;
  irqController->intcIsrSet2 = 0x0;
  irqController->intcIsrClear0 = 0x0;
  irqController->intcIsrClear1 = 0x0;
  irqController->intcIsrClear2 = 0x0;
  irqController->intcPendingIrq0 = 0x0;
  irqController->intcPendingIrq1 = 0x0;
  irqController->intcPendingIrq2 = 0x0;
  irqController->intcPendingFiq0 = 0x0;
  irqController->intcPendingFiq1 = 0x0;
  irqController->intcPendingFiq2 = 0x0;
  u32int i = 0;
  for (i = 0; i < 96; i++)
  {
    irqController->intcIlr[i] = 0;
  }
  // reset done flag
  irqController->intcSysStatus = irqController->intcSysStatus | INTCPS_SYSSTATUS_SOFTRESET;
}

void maskInterrupt(u32int interruptNumber)
{
  u32int bitMask = 0;
  u32int bankNumber = 0;

  if ((interruptNumber < 0) || (interruptNumber >= INTCPS_NR_OF_INTERRUPTS))
  {
    DIE_NOW(0, "INTC: mask interrupt number out of range.");
  }
  
  bankNumber = interruptNumber / INTCPS_INTERRUPTS_PER_BANK;
  bitMask = 1 << (interruptNumber % INTCPS_INTERRUPTS_PER_BANK);

  switch (bankNumber)
  {
    case 0:
      irqController->intcMir0 |= bitMask;
      break;  
    case 1:
      irqController->intcMir1 |= bitMask;
      break;  
    case 2:
      irqController->intcMir2 |= bitMask;
      break;  
    default:
      DIE_NOW(0, "INTC: mask interrupt from invalid interrupt bank");
  }

}

void unmaskInterrupt(u32int interruptNumber)
{
  u32int bitMask = 0;
  u32int bankNumber = 0;

  if ((interruptNumber < 0) || (interruptNumber >= INTCPS_NR_OF_INTERRUPTS))
  {
    DIE_NOW(0, "INTC: unmask interrupt number out of range.");
  }
  bankNumber = interruptNumber / INTCPS_INTERRUPTS_PER_BANK;
  bitMask = 1 << (interruptNumber % INTCPS_INTERRUPTS_PER_BANK);
  switch (bankNumber)
  {
    case 0:
      irqController->intcMir0 &= ~bitMask;
      break;  
    case 1:
      irqController->intcMir1 &= ~bitMask;
      break;  
    case 2:
      irqController->intcMir2 &= ~bitMask;
      break;  
    default:
      DIE_NOW(0, "INTC: unmask interrupt from invalid interrupt bank");
  }
}

bool isGuestIrqMasked(u32int interruptNumber)
{
  u32int bitMask = 0;
  u32int bankNumber = 0;

  if ((interruptNumber < 0) || (interruptNumber >= INTCPS_NR_OF_INTERRUPTS))
  {
    DIE_NOW(0, "INTC: isMasked interrupt number out of range.");
  }
  bankNumber = interruptNumber / INTCPS_INTERRUPTS_PER_BANK;
  bitMask = 1 << (interruptNumber % INTCPS_INTERRUPTS_PER_BANK);
  switch (bankNumber)
  {
    case 0:
      return ((irqController->intcMir0 & bitMask) == 1);
      break;  
    case 1:
      return ((irqController->intcMir1 & bitMask) == 1);
      break;  
    case 2:
      return ((irqController->intcMir2 & bitMask) == 1);
      break;  
    default:
      DIE_NOW(0, "INTC: unmask interrupt from invalid interrupt bank");
  }
  // keep compiler quiet
  return TRUE;
}

u32int getIrqNumber(void)
{
  DIE_NOW(0, "INTC: getIrqNumber - implement priority sorting of queued IRQS");
  return (irqController->intcSirIrq & INTCPS_SIR_IRQ_ACTIVEIRQ);
}

void setInterrupt(u32int irqNum)
{
  // 1. set raw interrupt signal before masking
  u32int bitMask = 0;
  u32int bankNumber = 0;

  if ((irqNum < 0) || (irqNum >= INTCPS_NR_OF_INTERRUPTS))
  {
    DIE_NOW(0, "INTC: setInterrupt interrupt number out of range.");
  }
  bankNumber = irqNum / INTCPS_INTERRUPTS_PER_BANK;
  bitMask = 1 << (irqNum % INTCPS_INTERRUPTS_PER_BANK);
  switch (bankNumber)
  {
    case 0:
      irqController->intcItr0 |= bitMask;
      break;
    case 1:
      irqController->intcMir1 |= bitMask;
      break;
    case 2:
      irqController->intcMir2 |= bitMask;
      break;
    default:
      DIE_NOW(0, "INTC: setInterrupt in invalid interrupt bank");
  }

  // 2. check mask, set signal after masking if needed.
  if(!isGuestIrqMasked(irqNum))
  {
    // unmasked! set flag...
    switch (bankNumber)
    {
      case 0:
        irqController->intcPendingIrq0 |= bitMask;
        break;
      case 1:
        irqController->intcPendingIrq1 |= bitMask;
        break;
      case 2:
        irqController->intcPendingIrq2 |= bitMask;
        break;
      default:
        DIE_NOW(0, "INTC: setInterrupt in invalid interrupt bank");
    }
  }
  // 3. leave priority sorting for now. it will be done when IRQ number gets read.
}


void clearInterrupt(u32int irqNum)
{
  // 1. clear raw interrupt signal before masking
  u32int bitMask = 0;
  u32int bankNumber = 0;

  if ((irqNum < 0) || (irqNum >= INTCPS_NR_OF_INTERRUPTS))
  {
    DIE_NOW(0, "INTC: setInterrupt interrupt number out of range.");
  }
  bankNumber = irqNum / INTCPS_INTERRUPTS_PER_BANK;
  bitMask = 1 << (irqNum % INTCPS_INTERRUPTS_PER_BANK);
  bitMask = ~bitMask;
  switch (bankNumber)
  {
    case 0:
      irqController->intcItr0 &= bitMask;
      break;
    case 1:
      irqController->intcMir1 &= bitMask;
      break;
    case 2:
      irqController->intcMir2 &= bitMask;
      break;
    default:
      DIE_NOW(0, "INTC: setInterrupt in invalid interrupt bank");
  }

  // 2. clear irq-after-masking reg just in case as well
  switch (bankNumber)
  {
    case 0:
      irqController->intcPendingIrq0 &= bitMask;
      break;
    case 1:
      irqController->intcPendingIrq1 &= bitMask;
      break;
    case 2:
      irqController->intcPendingIrq2 &= bitMask;
      break;
    default:
      DIE_NOW(0, "INTC: setInterrupt in invalid interrupt bank");
  }
}


// Function to look through all pending irqs and select highest priority one
// return: interrupt number
u32int prioritySortIrqs()
{
  if (!isIrqPending())
  {
    // no interrupts pending.
    return 0;
  }
  else
  {
    u32int currentHighestPriority = 0;
    u32int currentIrqNumber = 0;
    if (irqController->intcPendingIrq0 != 0)
    {
      u32int mask = 0;
      u32int i = 0;
      for (i = 0; i < 32; i++)
      {
        mask = 1 << i;
        if ((irqController->intcPendingIrq0 & mask) != 0)
        {
          u32int priority = irqController->intcIlr[i];
#ifdef INTC_DBG
          DEBUG_STRING("INTC: irq nr ");
          DEBUG_INT(i);
          DEBUG_STRING(" is pending with priority ");
          DEBUG_INT(priority);
          DEBUG_NEWLINE();
#endif
          if (priority >= currentHighestPriority)
          {
            currentHighestPriority = priority;
            currentIrqNumber = i;
          }
        }
      }
    }

    if (irqController->intcPendingIrq1 != 0)
    {
      u32int mask = 0; 
      u32int i = 0;
      for (i = 0; i < 32; i++)
      {
        mask = 1 << i;
        if ((irqController->intcPendingIrq1 & mask) != 0)
        {
          u32int priority = irqController->intcIlr[i+32];
#ifdef INTC_DBG
          DEBUG_STRING("INTC: irq nr ");
          DEBUG_INT(i+32);
          DEBUG_STRING(" is pending with priority ");
          DEBUG_INT(priority);
          DEBUG_NEWLINE();
#endif
          if (priority >= currentHighestPriority)
          {
            currentHighestPriority = priority;
            currentIrqNumber = i+32;
          }
        }
      }
    }

    if (irqController->intcPendingIrq2 != 0)
    {
      u32int mask = 0; 
      u32int i = 0;
      for (i = 0; i < 32; i++)
      {
        mask = 1 << i;
        if ((irqController->intcPendingIrq2 & mask) != 0)
        {
          u32int priority = irqController->intcIlr[i+64];
#ifdef INTC_DBG
          DEBUG_STRING("INTC: irq nr ");
          DEBUG_INT(i+64);
          DEBUG_STRING(" is pending with priority ");
          DEBUG_INT(priority);
          DEBUG_NEWLINE();
#endif
          if (priority >= currentHighestPriority)
          {
            currentHighestPriority = priority;
            currentIrqNumber = i+64;
          }
        }
      }
    }

    return currentIrqNumber;
  }
}


bool isIrqPending()
{
  return ( (irqController->intcPendingIrq0 != 0) ||
           (irqController->intcPendingIrq1 != 0) ||
           (irqController->intcPendingIrq2 != 0) );
}

bool isFiqPending()
{
  return ( (irqController->intcPendingFiq0 != 0) ||
           (irqController->intcPendingFiq1 != 0) ||
           (irqController->intcPendingFiq2 != 0) );
}

void intcDumpRegisters(void)
{
  DEBUG_STRING("INTC: Revision ");
  DEBUG_INT(INTC_REVISION);
  DEBUG_NEWLINE();

  DEBUG_STRING("INTC: sysconfig reg ");
  DEBUG_INT(irqController->intcSysConfig);
  DEBUG_NEWLINE();

  DEBUG_STRING("INTC: sysStatus reg ");
  DEBUG_INT(irqController->intcSysStatus);
  DEBUG_NEWLINE();

  DEBUG_STRING("INTC: current active irq reg ");
  DEBUG_INT(irqController->intcSirIrq);
  DEBUG_NEWLINE();

  DEBUG_STRING("INTC: current active fiq reg ");
  DEBUG_INT(irqController->intcSirFiq);
  DEBUG_NEWLINE();

  DEBUG_STRING("INTC: control reg ");
  DEBUG_INT(irqController->intcControl);
  DEBUG_NEWLINE();

  DEBUG_STRING("INTC: protection reg ");
  DEBUG_INT(irqController->intcProtection);
  DEBUG_NEWLINE();

  DEBUG_STRING("INTC: idle reg ");
  DEBUG_INT(irqController->intcIdle);
  DEBUG_NEWLINE();

  DEBUG_STRING("INTC: current active irq priority ");
  DEBUG_INT(irqController->intcIrqPriority);
  DEBUG_NEWLINE();

  DEBUG_STRING("INTC: current active fiq priority ");
  DEBUG_INT(irqController->intcFiqPriority);
  DEBUG_NEWLINE();

  DEBUG_STRING("INTC: priority threshold ");
  DEBUG_INT(irqController->intcThreshold);
  DEBUG_NEWLINE();

  DEBUG_STRING("INTC: interrupt status before masking:");
  DEBUG_NEWLINE();
  DEBUG_INT(irqController->intcItr0);
  DEBUG_INT(irqController->intcItr1);
  DEBUG_INT(irqController->intcItr2);
  DEBUG_NEWLINE();

  DEBUG_STRING("INTC: interrupt mask:");
  DEBUG_NEWLINE();
  DEBUG_INT(irqController->intcMir0);
  DEBUG_INT(irqController->intcMir1);
  DEBUG_INT(irqController->intcMir2);
  DEBUG_NEWLINE();

  DEBUG_STRING("INTC: pending IRQ:");
  DEBUG_NEWLINE();
  DEBUG_INT(irqController->intcPendingIrq0);
  DEBUG_INT(irqController->intcPendingIrq1);
  DEBUG_INT(irqController->intcPendingIrq2);
  DEBUG_NEWLINE();

  DEBUG_STRING("INTC: pending FIQ:");
  DEBUG_NEWLINE();
  DEBUG_INT(irqController->intcPendingFiq0);
  DEBUG_INT(irqController->intcPendingFiq1);
  DEBUG_INT(irqController->intcPendingFiq2);
  DEBUG_NEWLINE();
}
