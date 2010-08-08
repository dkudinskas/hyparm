#include "serial.h"
#include "intc.h"
#include "memFunctions.h"
#include "memoryConstants.h" // for BEAGLE_RAM_START/END
#include "pageTable.h" // for getPhysicalAddress()
#include "guestContext.h"

extern GCONTXT * getGuestContext(void);

struct InterruptController * irqController;

void initIntc(void)
{
  irqController = (struct InterruptController*)mallocBytes(sizeof(struct InterruptController));
  if (irqController == 0)
  {
    serial_ERROR("Failed to allocate INTC.");
  }
  else
  {
    memset((void*)irqController, 0x0, sizeof(struct InterruptController));
#ifdef INTC_DBG
    serial_putstring("Initializing INTC at 0x");
    serial_putint((u32int)irqController);
    serial_newline();
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
    serial_ERROR("Intc: invalid access size.");
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
    case REG_INTCPS_SIR_IRQ:
    case REG_INTCPS_SIR_FIQ:
    case REG_INTCPS_CONTROL:
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
    case REG_INTCPS_MIR_CLEAR0:
    case REG_INTCPS_MIR_CLEAR1:
    case REG_INTCPS_MIR_CLEAR2:
    case REG_INTCPS_MIR_SET0:
    case REG_INTCPS_MIR_SET1:
    case REG_INTCPS_MIR_SET2:
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
      serial_putstring("Intc: Unimplemted regsiter load.");
      serial_putstring("register number ");
      serial_putint(regOffset);
      serial_newline();
      serial_ERROR("PANIC");
      break;
    default:
      serial_ERROR("Intc: load on invalid register.");
  }
  
#ifdef INTC_DBG
  serial_putstring(dev->deviceName);
  serial_putstring(" load from pAddr: 0x");
  serial_putint(phyAddr);
  serial_putstring(", vAddr: 0x");
  serial_putint(address);
  serial_putstring(" access size ");
  serial_putint((u32int)size);
  serial_putstring(" val = ");
  serial_putint(val);
  serial_newline();
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
  serial_putstring(dev->deviceName);
  serial_putstring(" store to pAddr: 0x");
  serial_putint(phyAddr);
  serial_putstring(", vAddr: 0x");
  serial_putint(address);
  serial_putstring(" access size ");
  serial_putint((u32int)size);
  serial_putstring(" val ");
  serial_putint(value);
  serial_newline();
#endif

  if (size != WORD)
  {
    // only word access allowed in these modules
    serial_ERROR("Intc: invalid access size.");
  }

  u32int regOffset = phyAddr - INTERRUPT_CONTROLLER;
  u32int val = 0;
  switch (regOffset)
  {
    case REG_INTCPS_REVISION:
      serial_ERROR("Intc storing to read only register: version");
      break;
    case REG_INTCPS_SYSCONFIG:
      if (val & INTCPS_SYSCONFIG_SOFTRESET)
      {
        intcReset();
        val = val & ~INTCPS_SYSCONFIG_SOFTRESET;
      }
      // never set reset bit, all else except bit 0 is reserved
      irqController->intcSysConfig = val & INTCPS_SYSCONFIG_AUTOIDLE;
      break;
    case REG_INTCPS_SYSSTATUS:
      serial_ERROR("Intc storing to read only register: system status");
      break;
    case REG_INTCPS_SIR_IRQ:
    case REG_INTCPS_SIR_FIQ:
    case REG_INTCPS_CONTROL:
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
    case REG_INTCPS_MIR_CLEAR0:
    case REG_INTCPS_MIR_CLEAR1:
    case REG_INTCPS_MIR_CLEAR2:
    case REG_INTCPS_MIR_SET0:
    case REG_INTCPS_MIR_SET1:
    case REG_INTCPS_MIR_SET2:
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
      serial_putstring("Intc: Unimplemted regsiter store.");
      serial_putstring("register number ");
      serial_putint(regOffset);
      serial_newline();
      serial_ERROR("PANIC");
      break;
    default:
      serial_ERROR("Intc: store on invalid register.");
  }
}

void intcReset()
{
#ifdef INTC_DBG
  serial_putstring("INTC: soft reset.");
  serial_newline();
#endif
  // reset all register values to defaults
  irqController->intcSysConfig   = 0x00000000;
  irqController->intcSysStatus   = 0x00000000;
  irqController->intcSirIrq      = 0xFFFFFFE0;
  irqController->intcSirFiq      = 0xFFFFFFE0;
  irqController->intcControl     = 0x00000000;;
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
  irqController->intcIlr0 = 0x0;
  irqController->intcIlr1 = 0x0;
  irqController->intcIlr2 = 0x0;
  irqController->intcIlr3 = 0x0;
  irqController->intcIlr4 = 0x0;
  irqController->intcIlr5 = 0x0;
  irqController->intcIlr6 = 0x0;
  irqController->intcIlr7 = 0x0;
  irqController->intcIlr8 = 0x0;
  irqController->intcIlr9 = 0x0;
  irqController->intcIlr10 = 0x0;
  irqController->intcIlr11 = 0x0;
  irqController->intcIlr12 = 0x0;
  irqController->intcIlr13 = 0x0;
  irqController->intcIlr14 = 0x0;
  irqController->intcIlr15 = 0x0;
  irqController->intcIlr16 = 0x0;
  irqController->intcIlr17 = 0x0;
  irqController->intcIlr18 = 0x0;
  irqController->intcIlr19 = 0x0;
  irqController->intcIlr20 = 0x0;
  irqController->intcIlr21 = 0x0;
  irqController->intcIlr22 = 0x0;
  irqController->intcIlr23 = 0x0;
  irqController->intcIlr24 = 0x0;
  irqController->intcIlr25 = 0x0;
  irqController->intcIlr26 = 0x0;
  irqController->intcIlr27 = 0x0;
  irqController->intcIlr28 = 0x0;
  irqController->intcIlr29 = 0x0;
  irqController->intcIlr30 = 0x0;
  irqController->intcIlr31 = 0x0;
  irqController->intcIlr32 = 0x0;
  irqController->intcIlr33 = 0x0;
  irqController->intcIlr34 = 0x0;
  irqController->intcIlr35 = 0x0;
  irqController->intcIlr36 = 0x0;
  irqController->intcIlr37 = 0x0;
  irqController->intcIlr38 = 0x0;
  irqController->intcIlr39 = 0x0;
  irqController->intcIlr40 = 0x0;
  irqController->intcIlr41 = 0x0;
  irqController->intcIlr42 = 0x0;
  irqController->intcIlr43 = 0x0;
  irqController->intcIlr44 = 0x0;
  irqController->intcIlr45 = 0x0;
  irqController->intcIlr46 = 0x0;
  irqController->intcIlr47 = 0x0;
  irqController->intcIlr48 = 0x0;
  irqController->intcIlr49 = 0x0;
  irqController->intcIlr50 = 0x0;
  irqController->intcIlr51 = 0x0;
  irqController->intcIlr52 = 0x0;
  irqController->intcIlr53 = 0x0;
  irqController->intcIlr54 = 0x0;
  irqController->intcIlr55 = 0x0;
  irqController->intcIlr56 = 0x0;
  irqController->intcIlr57 = 0x0;
  irqController->intcIlr58 = 0x0;
  irqController->intcIlr59 = 0x0;
  irqController->intcIlr60 = 0x0;
  irqController->intcIlr61 = 0x0;
  irqController->intcIlr62 = 0x0;
  irqController->intcIlr63 = 0x0;
  irqController->intcIlr64 = 0x0;
  irqController->intcIlr65 = 0x0;
  irqController->intcIlr66 = 0x0;
  irqController->intcIlr67 = 0x0;
  irqController->intcIlr68 = 0x0;
  irqController->intcIlr69 = 0x0;
  irqController->intcIlr70 = 0x0;
  irqController->intcIlr71 = 0x0;
  irqController->intcIlr72 = 0x0;
  irqController->intcIlr73 = 0x0;
  irqController->intcIlr74 = 0x0;
  irqController->intcIlr75 = 0x0;
  irqController->intcIlr76 = 0x0;
  irqController->intcIlr77 = 0x0;
  irqController->intcIlr78 = 0x0;
  irqController->intcIlr79 = 0x0;
  irqController->intcIlr80 = 0x0;
  irqController->intcIlr81 = 0x0;
  irqController->intcIlr82 = 0x0;
  irqController->intcIlr83 = 0x0;
  irqController->intcIlr84 = 0x0;
  irqController->intcIlr85 = 0x0;
  irqController->intcIlr86 = 0x0;
  irqController->intcIlr87 = 0x0;
  irqController->intcIlr88 = 0x0;
  irqController->intcIlr89 = 0x0;
  irqController->intcIlr90 = 0x0;
  irqController->intcIlr91 = 0x0;
  irqController->intcIlr92 = 0x0;
  irqController->intcIlr93 = 0x0;
  irqController->intcIlr94 = 0x0;
  irqController->intcIlr95 = 0x0;
  // reset done flag
  irqController->intcSysStatus = irqController->intcSysStatus | INTCPS_SYSSTATUS_SOFTRESET;
}



// this is backend functionality. dont do it yet.
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
*/


