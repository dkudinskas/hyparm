#include "common/debug.h"

#include "cpuArch/armv7.h"
#include "cpuArch/constants.h"

#include "guestManager/guestExceptions.h"

#include "vm/omap35xx/cp15coproc.h"
#include "vm/omap35xx/intc.h"


static inline u32int getExceptionHandlerAddress(GCONTXT *context, u32int offset);


/*
 * FIXME
 *
 * A guest can set an exception handler on an invalid address. The hypervisor will try to scan this
 * block and get a data abort in privileged mode, and then it crashes...
 */

void deliverServiceCall(GCONTXT *context)
{
  DEBUG(GUEST_EXCEPTIONS, "deliverServiceCall: @ PC = %#.8x" EOL, context->R15);
  // copy guest CPSR into SPSR_SVC
  context->SPSR_SVC.value = context->CPSR.value;
  // put guest CPSR in SVC mode
  guestChangeMode(context, SVC_MODE);
  // set LR to PC+instruction_size
#ifdef CONFIG_THUMB2
  context->R14_SVC += (context->CPSR.bits.T) ? context->R15 + T16_INSTRUCTION_SIZE 
                                             : context->R15 + ARM_INSTRUCTION_SIZE;
  // Clear or set Thumb bit according to SCTLR.TE
  context->CPSR.bits.T = (context->coprocRegBank[CP15_SCTRL].value & SCTLR_TE) ? 1 : 0;
#endif
  context->R14_SVC = context->R15 + ARM_INSTRUCTION_SIZE;
  // set PC to guest svc handler address
  context->R15 = getExceptionHandlerAddress(context, EXC_VECT_LOW_SVC);
  // update AFI bits for SVC:
  context->CPSR.bits.I = 1;
}


void throwInterrupt(GCONTXT *context, u32int irqNumber)
{
#ifdef CONFIG_HW_PASSTHROUGH
  DEBUG(GUEST_EXCEPTIONS, "throwInterrupt: %x\n", irqNumber);
  DIE_NOW(context, "how was this called? shouldn't happen in hw passthrough\n");
#else
  switch (irqNumber)
  {
    case GPT1_IRQ:
    {
      // gpt1 is dedicated to the guest.
      setInterrupt(context, GPT1_IRQ);
      // are we forwarding the interrupt event?
      if (isIrqPending(context->vm.irqController) && (context->CPSR.bits.I == 0))
      {
        // guest has enabled interrupts globally.
        // set guest irq pending flag!
        context->guestIrqPending = TRUE;
      }
      else
      {
        DEBUG(GUEST_EXCEPTIONS, "throwInterrupt: guest is not ready to handle IRQ: %#.8x" EOL, irqNumber);
      }
      break;
    }
    case UART1_IRQ:
    {
      setInterrupt(context, UART1_IRQ);
      // are we forwarding the interrupt event?
      if ( isIrqPending(context->vm.irqController) && (context->CPSR.bits.I == 0))
      {
        // guest has enabled interrupts globally.
        // set guest irq pending flag!
        context->guestIrqPending = TRUE;
      }
      break;
    }
    case UART2_IRQ:
    {
      setInterrupt(context, UART2_IRQ);
      // are we forwarding the interrupt event?
      if ( isIrqPending(context->vm.irqController) && (context->CPSR.bits.I == 0))
      {
        // guest has enabled interrupts globally.
        // set guest irq pending flag!
        context->guestIrqPending = TRUE;
      }
      break;
    }
    case UART3_IRQ:
    {
      setInterrupt(context, UART3_IRQ);
      // are we forwarding the interrupt event?
      if ( isIrqPending(context->vm.irqController) && (context->CPSR.bits.I == 0))
      {
        // guest has enabled interrupts globally.
        // set guest irq pending flag!
        context->guestIrqPending = TRUE;
      }
      break;
    }
    case I2C1_IRQ:
    {
      setInterrupt(context, I2C1_IRQ);
      // are we forwarding the interrupt event?
      if ( isIrqPending(context->vm.irqController) && (context->CPSR.bits.I == 0))
      {
        // guest has enabled interrupts globally.
        // set guest irq pending flag!
        context->guestIrqPending = TRUE;
      }
      break;
    }
    case I2C2_IRQ:
    {
      setInterrupt(context, I2C2_IRQ);
      // are we forwarding the interrupt event?
      if ( isIrqPending(context->vm.irqController) && ((context->CPSR.bits.I == 0))
      {
        // guest has enabled interrupts globally.
        // set guest irq pending flag!
        context->guestIrqPending = TRUE;
      }
      break;
    }
    case I2C3_IRQ:
    {
      setInterrupt(context, I2C3_IRQ);
      // are we forwarding the interrupt event?
      if ( isIrqPending(context->vm.irqController) && ((context->CPSR.bits.I == 0))
      {
        // guest has enabled interrupts globally.
        // set guest irq pending flag!
        context->guestIrqPending = TRUE;
      }
      break;
    }
    case MMC1_IRQ:
    {
      setInterrupt(context, MMC1_IRQ);
      // are we forwarding the interrupt event?
      if ( isIrqPending(context->vm.irqController) && ((context->CPSR.bits.I == 0))
      {
        // guest has enabled interrupts globally.
        // set guest irq pending flag!
        context->guestIrqPending = TRUE;
      }
      break;
    }
    case MMC2_IRQ:
    {
      setInterrupt(context, MMC2_IRQ);
      // are we forwarding the interrupt event?
      if ( isIrqPending(context->vm.irqController) && ((context->CPSR.bits.I == 0))
      {
        // guest has enabled interrupts globally.
        // set guest irq pending flag!
        context->guestIrqPending = TRUE;
      }
      break;
    }
    case MMC3_IRQ:
    {
      setInterrupt(context, MMC3_IRQ);
      // are we forwarding the interrupt event?
      if ( isIrqPending(context->vm.irqController) && ((context->CPSR.bits.I == 0))
      {
        // guest has enabled interrupts globally.
        // set guest irq pending flag!
        context->guestIrqPending = TRUE;
      }
      break;
    }
    case SDMA_IRQ_0:
    {
      setInterrupt(context, SDMA_IRQ_0);
      // are we forwarding the interrupt event?
      if ( isIrqPending(context->vm.irqController) && ((context->CPSR.bits.I == 0))
      {
        // guest has enabled interrupts globally.
        // set guest irq pending flag!
        context->guestIrqPending = TRUE;
      }
      break;
    }
    case SDMA_IRQ_1:
    {
      setInterrupt(context, SDMA_IRQ_1);
      // are we forwarding the interrupt event?
      if ( isIrqPending(context->vm.irqController) && ((context->CPSR.bits.I == 0))
      {
        // guest has enabled interrupts globally.
        // set guest irq pending flag!
        context->guestIrqPending = TRUE;
      }
      break;
    }
    case SDMA_IRQ_2:
    {
      setInterrupt(context, SDMA_IRQ_2);
      // are we forwarding the interrupt event?
      if ( isIrqPending(context->vm.irqController) && ((context->CPSR.bits.I == 0))
      {
        // guest has enabled interrupts globally.
        // set guest irq pending flag!
        context->guestIrqPending = TRUE;
      }
      break;
    }
    case SDMA_IRQ_3:
    {
      setInterrupt(context, SDMA_IRQ_3);
      // are we forwarding the interrupt event?
      if ( isIrqPending(context->vm.irqController) && ((context->CPSR.bits.I == 0))
      {
        // guest has enabled interrupts globally.
        // set guest irq pending flag!
        context->guestIrqPending = TRUE;
      }
      break;
    }
    default:
    {
      DIE_NOW(context, "throwInterrupt: from unknown source.");
    }
  }
#endif
}


void deliverInterrupt(GCONTXT *context)
{
  DEBUG(GUEST_EXCEPTIONS, "deliverInterrupt: @ PC = %#.8x" EOL, context->R15);
  // reset irq pending flag.
  context->guestIrqPending = FALSE;
  // copy guest CPSR into SPSR_IRQ
  context->SPSR_IRQ.value = context->CPSR.value;
  // put guest CPSR in IRQ mode
  guestChangeMode(context, IRQ_MODE);
#ifdef CONFIG_THUMB2
  // Clear or set Thumb bit according to SCTLR.TE
  context->CPSR.bits.T = (context->coprocRegBank[CP15_SCTRL].value & SCTLR_TE) ? 1 : 0;
#endif

  // 5. set LR to PC+4
  context->R14_IRQ = context->R15 + LR_OFFSET_IRQ;
  // 6. set PC to guest irq handler address
  context->R15 = getExceptionHandlerAddress(context, EXC_VECT_LOW_IRQ);
  // update AFI bits for IRQ:
  context->CPSR.bits.A = 1;
  context->CPSR.bits.I = 1;
}


void deliverDataAbort(GCONTXT *context)
{
  DEBUG(GUEST_EXCEPTIONS, "deliverDataAbort: @ PC = %#.8x" EOL, context->R15);
  // reset abt pending flag
  context->guestDataAbtPending = FALSE;
  // copy CPSR into SPSR_ABT
  context->SPSR_ABT.value = context->CPSR.value;
  // put guest CPSR in ABT mode
  guestChangeMode(context, ABT_MODE);
#ifdef CONFIG_THUMB2
  // Clear or set Thumb bit according to SCTLR.TE
  context->CPSR.bits.T = (context->coprocRegBank[CP15_SCTRL].value & SCTLR_TE) ? 1 : 0;
#endif
  // set LR to PC+8
  context->R14_ABT = context->R15 + LR_OFFSET_DATA_ABT;
  // set PC to guest irq handler address
  context->R15 = getExceptionHandlerAddress(context, EXC_VECT_LOW_DABT);
  // update AFI bits for IRQ:
  context->CPSR.bits.A = 1;
  context->CPSR.bits.I = 1;
}


void throwDataAbort(GCONTXT *context, u32int address, u32int faultType, bool isWrite, u32int domain)
{
  // set CP15 Data Fault Status Register
  u32int dfsr = (faultType & 0xF) | ((faultType & 0x10) << 6);
  dfsr |= domain << 4;
  if (isWrite)
  {
    dfsr |= 0x800; // write-not-read bit
  }
  DEBUG(GUEST_EXCEPTIONS, "throwDataAbort: address %#.8x: faultType %#x, isWrite %x, dom %#x, @ PC"
      " %#.8x, dfsr %#.8x" EOL, address, faultType, isWrite, domain, context->R15, dfsr);
  context->coprocRegBank[CP15_DFSR].value = dfsr;
  // set CP15 Data Fault Address Register to 'address'
  context->coprocRegBank[CP15_DFAR].value = address;
  // set guest abort pending flag, return
  context->guestDataAbtPending = TRUE;
}


void deliverPrefetchAbort(GCONTXT *context)
{
  DEBUG(GUEST_EXCEPTIONS, "deliverPrefetchAbort: @ PC = %#.8x" EOL, context->R15);
  // reset abt pending flag
  context->guestPrefetchAbtPending = FALSE;
  // copy CPSR into SPSR_ABT
  context->SPSR_ABT.value = context->CPSR.value;
  // put guest CPSR in ABT mode
  guestChangeMode(context, ABT_MODE);
#ifdef CONFIG_THUMB2
  // Clear or set Thumb bit according to SCTLR.TE
  context->CPSR.bits.T = (context->coprocRegBank[CP15_SCTRL].value & SCTLR_TE) ? 1 : 0;
#endif
  // 5. set LR to PC+4
  context->R14_ABT = context->R15 + LR_OFFSET_PREFETCH_ABT;
  // 6. set PC to guest irq handler address
  context->R15 = getExceptionHandlerAddress(context, EXC_VECT_LOW_PABT);
  // update AFI bits for IRQ:
  context->CPSR.bits.A = 1;
  context->CPSR.bits.I = 1;
}


void throwPrefetchAbort(GCONTXT *context, u32int address, u32int faultType)
{
  // set CP15 Data Fault Status Register
  u32int ifsr = (faultType & 0xF) | ((faultType & 0x10) << 10);

  DEBUG(GUEST_EXCEPTIONS, "throwPrefetchAbort: address %#.8x, faultType %#x, @ PC %#.8x, IFSR "
      "%#.8x" EOL, address, faultType, context->R15, ifsr);

  context->coprocRegBank[CP15_IFSR].value = ifsr;
  // set CP15 Data Fault Address Register to 'address'
  context->coprocRegBank[CP15_IFAR].value = address;
  // set guest abort pending flag, return
  context->guestPrefetchAbtPending = TRUE;
}


static inline u32int getExceptionHandlerAddress(GCONTXT *context, u32int offset)
{
  /*
   * If SCTLR.V == 0, the exception base address is controlled by VBAR
   * otherwise, 'Hivecs' are in use and the exception base address is fixed
   * at EXC_VECT_HIGH_OFFS. WARNING: the calculation below is NOT valid for
   * the monitor exception base address!
   */
  if (context->guestHighVectorSet)
  {
    return EXC_VECT_HIGH_OFFS + offset;
  }
  else
  {
    u32int vbar = context->coprocRegBank[CP15_VBAR].value;
    if (likely(vbar == 0))
    {
      // Hack: bypass boot ROM, SRAM, etc.
      return *(u32int *)((u32int)context + offsetof(GCONTXT, guestUndefinedHandler) + offset - 4);
    }
    else
    {
      return vbar + offset;
    }
  }
}
