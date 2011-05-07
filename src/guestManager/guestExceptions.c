#include "common/debug.h"

#include "cpuArch/cpu.h"

#include "guestManager/guestExceptions.h"
#include "guestManager/guestContext.h"

#include "vm/omap35xx/intc.h"
#include "drivers/beagle/beGPTimer.h"

extern GCONTXT * getGuestContext(void);

void deliverServiceCall(void)
{
  GCONTXT * context = getGuestContext();
#ifdef GUEST_EXCEPTIONS_DBG
  printf("deliverServiceCall: @pc %08x\n", context->R15);
#endif
  // 2. copy guest CPSR into SPSR_SVC
  context->SPSR_SVC = context->CPSR;
  // 3. put guest CPSR in SVC mode
  context->CPSR = (context->CPSR & ~CPSR_MODE) | CPSR_MODE_SVC;
  // 4. set LR to PC+4
  if(context->CPSR & 0x20)// Were we on Thumb?
  {
  	context->R14_SVC = context->R15 + 2;
  }
  else
  {
  	context->R14_SVC = context->R15 + 4;
  }
  // 5. Clear Thumb bit
  context->CPSR &= ~0x20;
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
      context->R15 = context->guestSwiHandler;
	//DIE_NOW(0, "deliverInterrupt: SVC to be delivered with guest vmem off.");
  }
  // update AFI bits for SVC:
  context->CPSR |= CPSR_IRQ_DIS;
}

void throwInterrupt(u32int irqNumber)
{
  GCONTXT * context = getGuestContext();
  //printf("Guest IRQ: %x\n",irqNumber);
  switch (irqNumber)
  {
    case GPT2_IRQ:
      // gpt2 is dedicated to the guest. if irq is from gpt2
      // set it pending in emulated interrupt controller
      setInterrupt(GPT1_IRQ);
      // are we forwarding the interrupt event?
      if ( ( isIrqPending() ) && ((context->CPSR & CPSR_IRQ_DIS) == 0) )
      {
        // guest has enabled interrupts globally.
        // set guest irq pending flag!
#ifdef	GUEST_EXCEPTIONS_DBG
		printf("Enable guest Interrupts\n");
#endif
      	context->guestIrqPending = TRUE;
      }
	  //else
	  	//printf("Guest is not ready to handle IRQ: %x\n",context->R15);
      break;
    case UART1_IRQ:
      setInterrupt(UART1_IRQ);
      // are we forwarding the interrupt event?
      if ( isIrqPending() && ((context->CPSR & CPSR_IRQ_DIS) == 0) )
      {
        // guest has enabled interrupts globally.
        // set guest irq pending flag!
        context->guestIrqPending = TRUE;
      }
      break;
    case UART2_IRQ:
      setInterrupt(UART2_IRQ);
      // are we forwarding the interrupt event?
      if ( isIrqPending() && ((context->CPSR & CPSR_IRQ_DIS) == 0) )
      {
        // guest has enabled interrupts globally.
        // set guest irq pending flag!
        context->guestIrqPending = TRUE;
		maskInterrupt(GPT1_IRQ);
      }
      break;
    case UART3_IRQ:
      setInterrupt(UART3_IRQ);
      // are we forwarding the interrupt event?
      if ( isIrqPending() && ((context->CPSR & CPSR_IRQ_DIS) == 0) )
      {
        // guest has enabled interrupts globally.
        // set guest irq pending flag!
        context->guestIrqPending = TRUE;
      }
      break;
    default:
      DIE_NOW(0, "throwInterrupt: from unknown source.");
  }
}

void deliverInterrupt(void)
{
  GCONTXT * context = getGuestContext();
  // 1. reset irq pending flag.
  context->guestIrqPending = FALSE;
  // 2. copy guest CPSR into SPSR_IRQ
  context->SPSR_IRQ = context->CPSR;
  // 3. put guest CPSR in IRQ mode
  context->CPSR = (context->CPSR & ~CPSR_MODE) | CPSR_MODE_IRQ;
  // 4. clear Thumb bit
  context->CPSR &= ~0x20; // FIX ME. This needs to be hardcoded somewhere
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
      context->R15 = context->guestIrqHandler;
  }
  // update AFI bits for IRQ:
  context->CPSR |= CPSR_IRQ_DIS;
  context->CPSR |= CPSR_ASYNC_ABT_DIS;
}

void deliverDataAbort()
{
  GCONTXT * context = getGuestContext();
  // 1. reset abt pending flag
  context->guestDataAbtPending = FALSE;
  // 2. copy CPSR into SPSR_ABT
  context->SPSR_ABT = context->CPSR;
  // 3. put guest CPSR in ABT mode
  context->CPSR = (context->CPSR & ~CPSR_MODE) | CPSR_MODE_ABT;
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
  	context->R15 = context->guestDataAbortHandler;
  }
  // update AFI bits for IRQ:
  context->CPSR |= CPSR_IRQ_DIS;
  context->CPSR |= CPSR_ASYNC_ABT_DIS;
}

void throwDataAbort(u32int address, u32int faultType, bool isWrite, u32int domain)
{
  GCONTXT* context = getGuestContext();

  // set CP15 Data Fault Status Register
  u32int dfsr = (faultType & 0xF) | ((faultType & 0x10) << 6);
  dfsr |= domain << 4;
  if (isWrite)
  {
    dfsr |= 0x800; // write-not-read bit
  }
#ifdef GUEST_EXCEPTIONS_DBG
  printf("throwDataAbort(%08x): faultType %x, isWrite %x, dom %x, @pc %08x, dfsr %08x\n", 
         address, faultType, isWrite, domain, context->R15, dfsr);
#endif
  setCregVal(5, 0, 0, 0, context->coprocRegBank, dfsr);
  // set CP15 Data Fault Address Register to 'address'
  setCregVal(6, 0, 0, 0, context->coprocRegBank, address);
  // set guest abort pending flag, return
  context->guestDataAbtPending = TRUE;
}

void deliverPrefetchAbort(void)
{
  GCONTXT * context = getGuestContext();
  // 1. reset abt pending flag
  context->guestPrefetchAbtPending = FALSE;
  // 2. copy CPSR into SPSR_ABT
  context->SPSR_ABT = context->CPSR;
  // 3. put guest CPSR in ABT mode
  context->CPSR = (context->CPSR & ~CPSR_MODE) | CPSR_MODE_ABT;
  // 4. set LR to PC+8
  context->R14_ABT = context->R15 + 8;
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
    DIE_NOW(0, "deliverInterrupt: Prefetch abort to be delivered with guest vmem off.");
  }
  // update AFI bits for IRQ:
  context->CPSR |= CPSR_IRQ_DIS;
  context->CPSR |= CPSR_ASYNC_ABT_DIS;
}

void throwPrefetchAbort(u32int address, u32int faultType)
{
  GCONTXT* context = getGuestContext();
  // set CP15 Data Fault Status Register
  u32int ifsr = (faultType & 0xF) | ((faultType & 0x10) << 10);

#ifdef GUEST_EXCEPTIONS_DBG
  printf("throwPrefetchAbort(%08x): faultType %x, @pc %08x, ifsr %08x\n",
          address, faultType, context->R15, ifsr);
#endif
  setCregVal(5, 0, 0, 1, context->coprocRegBank, ifsr);
  // set CP15 Data Fault Address Register to 'address'
  setCregVal(6, 0, 0, 2, context->coprocRegBank, address);
  // set guest abort pending flag, return
  context->guestPrefetchAbtPending = TRUE;
}
