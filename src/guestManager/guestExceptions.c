#include "guestExceptions.h"
#include "guestContext.h"
#include "intc.h"
#include "cpu.h"
#include "debug.h"

extern GCONTXT * getGuestContext(void);


void tickEvent(u32int irqNumber)
{
  GCONTXT * context = getGuestContext();

  if (irqNumber == GPT2_IRQ)
  {
    // adjust guest interrupt controller state
    setInterrupt(GPT1_IRQ);
    // are we forwarding the interrupt event?
    if ((context->CPSR & CPSR_IRQ_DIS) == 0)
    {
      // guest has enabled interrupts globally.
      // set guest irq pending flag!
      context->guestIrqPending = TRUE;
    }
  }
  else
  {
    DIE_NOW(0, "GuestInterrupts: tickEvent from unknown source timer.");
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
  // 4. disable further guest interrupts
  context->CPSR |= CPSR_IRQ_DIS;
  // 5. set LR to PC+4
  context->R14_IRQ = context->R15 + 4;
  // 6. set PC to guest irq handler address
  if (context->virtAddrEnabled)
  {
    if (context->guestHighVectorSet)
    {
      context->R15 = 0xffff0018;
    }
    else
    {
      context->R15 = 0x00000018;
    }
  }
  else
  {
    DIE_NOW(0, "deliverInterrupt: IRQ to be delivered with guest vmem off.");
  }
  // only thing left is to mask subsequent interrupts
  context->CPSR |= CPSR_IRQ_DIS;
  context->CPSR |= CPSR_FIQ_DIS;
}

void deliverAbort()
{
  GCONTXT * context = getGuestContext();
  // 1. reset abt pending flag
  context->guestAbtPending = FALSE;
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
      context->R15 = 0xffff0010;
    }
    else
    {
      context->R15 = 0x00000010;
    }
  }
  else
  {
    DIE_NOW(0, "deliverInterrupt: IRQ to be delivered with guest vmem off.");
  }
  // only thing left is to mask subsequent interrupts
  context->CPSR |= CPSR_IRQ_DIS;
  context->CPSR |= CPSR_FIQ_DIS;
}

void throwAbort(u32int address, u32int faultType, bool isWrite, u32int domain)
{
  GCONTXT* context = getGuestContext();
#ifdef GUEST_EXCEPTIONS_DBG
  serial_putstring("throwAbort(");
  serial_putint(address);
  serial_putstring(", faultType ");
  serial_putint_nozeros(faultType);
  serial_putstring(", isWrite ");
  serial_putint(isWrite);
  serial_putstring(", dom ");
  serial_putint(isWrite);
  serial_putstring(" @pc=");
  serial_putint(context->R15);
  serial_putstring(")");
  serial_newline();
#endif
  // set CP15 Data Fault Status Register
  u32int dfsr = (faultType & 0xF) | ((faultType & 0x10) << 6);
  dfsr |= domain << 4;
  if (isWrite)
  {
    dfsr |= 0x800; // write-not-read bit
  }
  setCregVal(5, 0, 0, 0, context->coprocRegBank, dfsr);
  // set CP15 Data Fault Address Register to 'address'
  setCregVal(6, 0, 0, 0, context->coprocRegBank, address);
  // set guest abort pending flag, return
  context->guestAbtPending = TRUE;
}
