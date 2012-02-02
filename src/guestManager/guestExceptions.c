#include "common/debug.h"

#include "cpuArch/armv7.h"
#include "cpuArch/constants.h"

#include "guestManager/guestExceptions.h"

#include "vm/omap35xx/intc.h"


/*
 * FIXME
 *
 * A guest can set an exception handler on an invalid address. The hypervisor will try to scan this
 * block and get a data abort in privileged mode, and then it crashes...
 */


void deliverServiceCall(GCONTXT *context)
{
  DEBUG(GUEST_EXCEPTIONS, "deliverServiceCall: @ PC = %#.8x" EOL, context->R15);
  if (!isGuestInPrivMode(context))
  {
    guestToPrivMode();
  }
  // 2. copy guest CPSR into SPSR_SVC
  context->SPSR_SVC = context->CPSR;
  // 3. put guest CPSR in SVC mode
  context->CPSR = (context->CPSR & ~PSR_MODE) | PSR_SVC_MODE;
  // 4. set LR to PC+4
#ifdef CONFIG_THUMB2
  if (context->CPSR & PSR_T_BIT)// Were we on Thumb?
  {
    context->R14_SVC = context->R15 + T16_INSTRUCTION_SIZE;
  }
  else
  {
#endif
    context->R14_SVC = context->R15 + ARM_INSTRUCTION_SIZE;
#ifdef CONFIG_THUMB2
  }
  /*
   * FIXME Niels: I think this depends on a CP15 value
   */
  // 5. Clear Thumb bit
  context->CPSR &= ~PSR_T_BIT;
#endif
  // 6. set PC to guest svc handler address
  if (context->virtAddrEnabled)
  {
    if (context->guestHighVectorSet)
    {
      context->R15 = EXC_VECT_HIGH_SVC;
    }
    else
    {
      context->R15 = EXC_VECT_LOW_SVC;
    }
  }
  else
  {
#ifdef CONFIG_GUEST_FREERTOS
    context->R15 = context->guestSwiHandler;
#else
    DIE_NOW(context, "deliverInterrupt: SVC to be delivered with guest vmem off.");
#endif
  }
  // update AFI bits for SVC:
  context->CPSR |= PSR_I_BIT;
}

void throwInterrupt(u32int irqNumber)
{
  GCONTXT * context = getGuestContext();

  switch (irqNumber)
  {
    case GPT2_IRQ:
      // gpt2 is dedicated to the guest. if irq is from gpt2
      // set it pending in emulated interrupt controller
      setInterrupt(GPT1_IRQ);
      // are we forwarding the interrupt event?
      if (isIrqPending() && ((context->CPSR & PSR_I_BIT) == 0))
      {
        // guest has enabled interrupts globally.
        // set guest irq pending flag!
//        DEBUG(GUEST_EXCEPTIONS, "throwInterrupt: enable guest interrupts" EOL);
        context->guestIrqPending = TRUE;
      }
      else
      {
//        DEBUG(GUEST_EXCEPTIONS, "throwInterrupt: guest is not ready to handle IRQ: %#.8x" EOL, irqNumber);
      }
      break;
    case UART1_IRQ:
      setInterrupt(UART1_IRQ);
      // are we forwarding the interrupt event?
      if ( isIrqPending() && ((context->CPSR & PSR_I_BIT) == 0) )
      {
        // guest has enabled interrupts globally.
        // set guest irq pending flag!
        context->guestIrqPending = TRUE;
      }
      break;
    case UART2_IRQ:
      setInterrupt(UART2_IRQ);
      // are we forwarding the interrupt event?
      if ( isIrqPending() && ((context->CPSR & PSR_I_BIT) == 0) )
      {
        // guest has enabled interrupts globally.
        // set guest irq pending flag!
        context->guestIrqPending = TRUE;
      }
      break;
    case UART3_IRQ:
      setInterrupt(UART3_IRQ);
      // are we forwarding the interrupt event?
      if ( isIrqPending() && ((context->CPSR & PSR_I_BIT) == 0) )
      {
        // guest has enabled interrupts globally.
        // set guest irq pending flag!
        context->guestIrqPending = TRUE;
      }
      break;
    default:
      DIE_NOW(context, "throwInterrupt: from unknown source.");
  }
}

void deliverInterrupt(GCONTXT *context)
{
  if (!isGuestInPrivMode(context))
  {
    DIE_NOW(context, "guest irq in guest user mode.\n");
  }
  // 1. reset irq pending flag.
  context->guestIrqPending = FALSE;
  // 2. copy guest CPSR into SPSR_IRQ
  context->SPSR_IRQ = context->CPSR;
  // 3. put guest CPSR in IRQ mode
  context->CPSR = (context->CPSR & ~PSR_MODE) | PSR_IRQ_MODE;
#ifdef CONFIG_THUMB2
  /*
   * FIXME Niels: I think this depends on a CP15 value
   */
  // 4. clear Thumb bit
  context->CPSR &= ~PSR_T_BIT; // FIX ME. This needs to be hardcoded somewhere
#endif
  // 5. set LR to PC+4
  context->R14_IRQ = context->R15 + 4;
  // 6. set PC to guest irq handler address
  if (context->virtAddrEnabled)
  {
    if (context->guestHighVectorSet)
    {
      context->R15 = EXC_VECT_HIGH_IRQ;
    }
    else
    {
      context->R15 = EXC_VECT_LOW_IRQ;
    }
  }
  else
  {
#ifdef CONFIG_GUEST_FREERTOS
    context->R15 = context->guestIrqHandler;
#else
    DIE_NOW(context, "deliverInterrupt: IRQ to be delivered with guest vmem off.");
#endif
  }
  // update AFI bits for IRQ:
  context->CPSR |= PSR_A_BIT | PSR_I_BIT;
}

void deliverDataAbort(GCONTXT *context)
{
  if (!isGuestInPrivMode(context))
  {
    guestToPrivMode();
  }
  // 1. reset abt pending flag
  context->guestDataAbtPending = FALSE;
  // 2. copy CPSR into SPSR_ABT
  context->SPSR_ABT = context->CPSR;
  // 3. put guest CPSR in ABT mode
  context->CPSR = (context->CPSR & ~PSR_MODE) | PSR_ABT_MODE;
  // 4. set LR to PC+8
  context->R14_ABT = context->R15 + 8;
  // 5. set PC to guest irq handler address
  if (context->virtAddrEnabled)
  {
    if (context->guestHighVectorSet)
    {
      context->R15 = EXC_VECT_HIGH_DABT;
    }
    else
    {
      context->R15 = EXC_VECT_LOW_DABT;
    }
  }
  else
  {
#ifdef CONFIG_GUEST_FREERTOS
    context->R15 = context->guestDataAbortHandler;
#else
    DIE_NOW(context, "deliverInterrupt: Data abort to be delivered with guest vmem off.");
#endif
  }
  // update AFI bits for IRQ:
  context->CPSR |= PSR_A_BIT | PSR_I_BIT;
}


void throwDataAbort(GCONTXT *context, u32int address, u32int faultType, bool isWrite, u32int domain)
{
  if (context->R15 == 0x000d3d2c)
  {
    DIE_NOW(context, "stop");
  }
  // set CP15 Data Fault Status Register
  u32int dfsr = (faultType & 0xF) | ((faultType & 0x10) << 6);
  dfsr |= domain << 4;
  if (isWrite)
  {
    dfsr |= 0x800; // write-not-read bit
  }
  DEBUG(GUEST_EXCEPTIONS, "throwDataAbort: address %#.8x: faultType %#x, isWrite %x, dom %#x, @ PC"
      " %#.8x, dfsr %#.8x" EOL, address, faultType, isWrite, domain, context->R15, dfsr);
  setCregVal(5, 0, 0, 0, context->coprocRegBank, dfsr);
  // set CP15 Data Fault Address Register to 'address'
  setCregVal(6, 0, 0, 0, context->coprocRegBank, address);
  // set guest abort pending flag, return
  context->guestDataAbtPending = TRUE;
}


void deliverPrefetchAbort(GCONTXT *context)
{
  if (!isGuestInPrivMode(context))
  {
    guestToPrivMode();
  }
  // 1. reset abt pending flag
  context->guestPrefetchAbtPending = FALSE;
  // 2. copy CPSR into SPSR_ABT
  context->SPSR_ABT = context->CPSR;
  // 3. put guest CPSR in ABT mode
  context->CPSR = (context->CPSR & ~PSR_MODE) | PSR_ABT_MODE;
  // 4. set LR to PC+8
  context->R14_ABT = context->R15 + 4;
  // 5. set PC to guest irq handler address
  if (context->virtAddrEnabled)
  {
    if (context->guestHighVectorSet)
    {
      context->R15 = EXC_VECT_HIGH_IABT;
    }
    else
    {
      context->R15 = EXC_VECT_LOW_IABT;
    }
  }
  else
  {
    DIE_NOW(context, "deliverInterrupt: Prefetch abort to be delivered with guest vmem off.");
  }
  // update AFI bits for IRQ:
  context->CPSR |= PSR_A_BIT | PSR_I_BIT;
}


void throwPrefetchAbort(GCONTXT *context, u32int address, u32int faultType)
{
  // set CP15 Data Fault Status Register
  u32int ifsr = (faultType & 0xF) | ((faultType & 0x10) << 10);

  DEBUG(GUEST_EXCEPTIONS, "throwPrefetchAbort: address %#.8x, faultType %#x, @ PC %#.8x, IFSR "
      "%#.8x" EOL, address, faultType, context->R15, ifsr);

  setCregVal(5, 0, 0, 1, context->coprocRegBank, ifsr);
  // set CP15 Data Fault Address Register to 'address'
  setCregVal(6, 0, 0, 2, context->coprocRegBank, address);
  // set guest abort pending flag, return
  context->guestPrefetchAbtPending = TRUE;
}
