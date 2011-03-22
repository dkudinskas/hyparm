#include "common/debug.h"

#include "cpuArch/cpu.h"

#include "guestManager/guestExceptions.h"
#include "guestManager/guestContext.h"

#include "hardware/intc.h"


extern GCONTXT * getGuestContext(void);

void deliverServiceCall(void)
{
  GCONTXT * context = getGuestContext();
  // 2. copy guest CPSR into SPSR_SVC
  context->SPSR_SVC = context->CPSR;
  // 3. put guest CPSR in SVC mode
  context->CPSR = (context->CPSR & ~CPSR_MODE) | CPSR_MODE_SVC;
  // 5. set LR to PC+4
  context->R14_SVC = context->R15 + 4;
  // 6. set PC to guest svc handler address
  if (context->virtAddrEnabled)
  {
    if (context->guestHighVectorSet)
    {
      context->R15 = 0xffff0008;
    }
    else
    {
      context->R15 = 0x00000008;
    }
  }
  else
  {
    DIE_NOW(0, "deliverInterrupt: SVC to be delivered with guest vmem off.");
  }
  // only thing left is to mask subsequent interrupts
  context->CPSR |= CPSR_IRQ_DIS;
  context->CPSR |= CPSR_FIQ_DIS;
}

void throwInterrupt(u32int irqNumber)
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
  // 4. set LR to PC+4
  context->R14_IRQ = context->R15 + 4;
  // 5. set PC to guest irq handler address
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
  // 6. mask subsequent guest interrupts
  context->CPSR |= CPSR_IRQ_DIS;
  context->CPSR |= CPSR_FIQ_DIS;
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
      context->R15 = 0xffff0010;
    }
    else
    {
      context->R15 = 0x00000010;
    }
  }
  else
  {
    DIE_NOW(0, "deliverInterrupt: Data abort to be delivered with guest vmem off.");
  }
  // 6. mask subsequent guest interrupts
  context->CPSR |= CPSR_IRQ_DIS;
  context->CPSR |= CPSR_FIQ_DIS;
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
  serial_putstring("throwDataAbort(");
  serial_putint(address);
  serial_putstring(", faultType ");
  serial_putint_nozeros(faultType);
  serial_putstring(", isWrite ");
  serial_putint(isWrite);
  serial_putstring(", dom ");
  serial_putint(isWrite);
  serial_putstring(" @pc=");
  serial_putint(context->R15);
  serial_putstring(" dfsr=");
  serial_putint(dfsr);
  serial_putstring(")");
  serial_newline();
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
      context->R15 = 0xffff000c;
    }
    else
    {
      context->R15 = 0x0000000c;
    }
  }
  else
  {
    DIE_NOW(0, "deliverInterrupt: Prefetch abort to be delivered with guest vmem off.");
  }
  // only thing left is to mask subsequent interrupts
  context->CPSR |= CPSR_IRQ_DIS;
  context->CPSR |= CPSR_FIQ_DIS;
}

void throwPrefetchAbort(u32int address, u32int faultType)
{
  GCONTXT* context = getGuestContext();
  // set CP15 Data Fault Status Register
  u32int ifsr = (faultType & 0xF) | ((faultType & 0x10) << 10);

#ifdef GUEST_EXCEPTIONS_DBG
  serial_putstring("throwPrefetchAbort(");
  serial_putint(address);
  serial_putstring(", faultType ");
  serial_putint(faultType);
  serial_putstring(" @pc=");
  serial_putint(context->R15);
  serial_putstring(" ifsr=");
  serial_putint(ifsr);
  serial_putstring(")");
  serial_newline();
#endif
  setCregVal(5, 0, 0, 1, context->coprocRegBank, ifsr);
  // set CP15 Data Fault Address Register to 'address'
  setCregVal(6, 0, 0, 2, context->coprocRegBank, address);
  // set guest abort pending flag, return
  context->guestPrefetchAbtPending = TRUE;
}
