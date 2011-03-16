#include "exceptionHandlers.h"
#include "serial.h"
#include "scanner.h"
#include "guestContext.h"
#include "addressing.h"
#include "gptimer.h"
#include "mmu.h"
#include "memoryConstants.h"
#include "scheduler.h"
#include "globalMemoryMapper.h"
#include "blockCache.h"
#include "beIntc.h"
#include "beGPTimer.h"
#include "cpu.h"
#include "guestExceptions.h"
#include "intc.h"
#include "be32kTimer.h"
#include "debug.h"

extern GCONTXT * getGuestContext(void);

void softwareInterrupt(u32int code)
{
#ifdef EXC_HDLR_DBG
  serial_putstring("softwareInterrupt(");
  serial_putint(code);
  serial_putstring(")");
  serial_newline();
#endif
  // parse the instruction to find the start address of next block
  GCONTXT * gContext = getGuestContext();
  u32int nextPC = 0;
  u32int (*instrHandler)(GCONTXT * context);

  if (code <= 0xFF)
  {
    serial_putstring("softwareInterrupt: SVC<");
    serial_putint(code);
    serial_putstring("> @ ");
    serial_putint(gContext->R15);
    serial_putstring(" is a guest system call.");
    serial_newline();
    deliverServiceCall();
    nextPC = gContext->R15;
  }
  else
  {
    // get interpreter function pointer and call it
    instrHandler = gContext->hdlFunct;
    nextPC = instrHandler(gContext);
  }

  if (nextPC == 0)
  {
    DIE_NOW(gContext, "softwareInterrupt: Invalid nextPC. Instr to implement?");
  }

  int i = 0;
  for (i = BLOCK_HISOTRY_SIZE-1; i > 0; i--)
  {
    gContext->blockHistory[i] = gContext->blockHistory[i-1];
  }
  gContext->blockHistory[0] = nextPC; 

  gContext->R15 = nextPC;

  // deliver interrupts
  if (gContext->guestIrqPending)
  {
    if ((gContext->CPSR & CPSR_IRQ_DIS) == 0)
    {
      deliverInterrupt();
    }
  }

#ifdef EXC_HDLR_DBG
  serial_putstring("softwareInterrupt: Next PC = 0x");
  serial_putint(nextPC);
  serial_newline();
#endif

  if ((gContext->CPSR & CPSR_MODE) != CPSR_MODE_USR)
  {
    // guest in privileged mode! scan...
    scanBlock(gContext, gContext->R15);
  }
}

void dataAbort()
{
  // make sure interrupts are disabled while we deal with data abort.
  disableInterrupts();
  u32int faultStatus = (getDFSR().fs3_0) | (getDFSR().fs4 << 4);
  switch(faultStatus)
  {
    case perm_section:
    case perm_page:
    {
      // Check if the addr we have faulted on is caused by 
      // a memory protection the hypervisor has enabled
      GCONTXT* gc = getGuestContext();

      // ATM dont expect anything else to permission fault except load/stores
      emulateLoadStoreGeneric(gc, getDFAR());
      if (!gc->guestDataAbtPending)
      {
        // ONLY move to the next instruction, if the guest hasn't aborted...
        gc->R15 = gc->R15 + 4;
      }
      else
      {
        // deliver the abort!
        deliverDataAbort();
        scanBlock(gc, gc->R15);
      }
      break;
    }
    case translation_section:
    case translation_page:
    {
      GCONTXT* gc = getGuestContext();
      DFSR dfsr = getDFSR();
      bool isPrivAccess = (gc->CPSR & CPSR_MODE) == CPSR_MODE_USR ? FALSE : TRUE;
      if ( shouldDataAbort(isPrivAccess, dfsr.WnR, getDFAR()) )
      {
        deliverDataAbort();
        scanBlock(gc, gc->R15);
      }
      break;
    }
    default:
      serial_putstring("Unimplemented user data abort.");
      serial_newline();
      printDataAbort();
      DIE_NOW(0, "Entering infinite loop");
  }
  enableInterrupts();
}

void dataAbortPrivileged()
{
  /* Here if we abort in a priviledged mode, i.e its the Hypervisors fault */
  serial_putstring("dataAbortPrivileged: Hypervisor data abort in priviledged mode.");
  serial_newline();

  printDataAbort();
  u32int faultStatus = (getDFSR().fs3_0) | (getDFSR().fs4 << 4);
  switch(faultStatus)
  {
    case translation_section:
    case translation_page:
    {
      //Mostly likely trying to access a page of physical memory, just map it.
      u32int memAddr = getDFAR();
      if( (memAddr >= BEAGLE_RAM_START) && (memAddr <= BEAGLE_RAM_END) )
      {
        serial_putstring("Fault inside physical RAM range.  hypervisor_page_fault (exceptionHandlers.c)");
        serial_newline();
        DIE_NOW(0, "Entering infinite loop");
      }
      else
      {
        DIE_NOW(0, "Translation fault for area not in RAM! Entering Infinite Loop...");
        /*
        I imagine there will be a few areas that we will need to map for the hypervisor only
        But not right now.
        */
      }
      break;
    }
    default:
      serial_putstring("UNIMPLEMENTED data abort type. (exceptionHandlers.c).");
      serial_newline();
      DIE_NOW(0, "Entering infinite loop");
      break;
  }


  DIE_NOW(0, "At end of hypervisor data abort handler. Stopping");

  serial_putstring("Exiting data abort handler");
  serial_newline();

  //Should be fixed and ready to re-execute the offending isntruction
}

void undefined(void)
{
  DIE_NOW(0, "undefined: undefined handler, Implement me!");
}

void undefinedPrivileged(void)
{
  DIE_NOW(0, "undefinedPrivileged: Undefined handler, privileged mode. Implement me!");
}

void prefetchAbort(void)
{
  // make sure interrupts are disabled while we deal with prefetch abort.
  disableInterrupts();

  IFSR ifsr = getIFSR();
  u32int ifar = getIFAR();
  u32int faultStatus = (ifsr.fs3_0) | (ifsr.fs4 << 4);
  GCONTXT* gc = getGuestContext();

  switch(faultStatus)
  {
    case translationFaultPage:
    {
      if ( shouldPrefetchAbort(ifar) )
      {
        deliverPrefetchAbort();
        scanBlock(gc, gc->R15);
      }
      break;
    }
    case debugEvent:
    case accessFlagFaultSection:
    case translationFaultSection:
    case accessFlagFaultPage:
    case synchronousExternalAbort:
    case domainFaultSection:
    case domainFaultPage:
    case translationTableTalk1stLvlSynchExtAbt:
    case permissionFaultSection:
    case translationTableWalk2ndLvllSynchExtAbt:
    case permissionFaultPage:
    case impDepLockdown:
    case memoryAccessSynchParityError:
    case impDepCoprocessorAbort:
    case translationTableWalk1stLvlSynchParityError:
    case translationTableWalk2ndLvlSynchParityError:
    default:
      printPrefetchAbort();
      DIE_NOW(gc, "Unimplemented guest prefetch abort.");
  }
  enableInterrupts();
}

void prefetchAbortPrivileged(void)
{
  printPrefetchAbort();
  DIE_NOW(0, "prefetchAbortPrivileged: unimplemented");
  //Never returns
}

void monitorMode(void)
{
  /* Does the omap 3 implement monitor/secure mode? */
  DIE_NOW(0, "monitorMode: monitor/secure mode handler, Implement me!");
}

void monitorModePrivileged(void)
{
  /* Does the omap 3 implement monitor/secure mode? */
  DIE_NOW(0, "monitorMode: monitor/secure mode handler, privlieged mode. Implement me!");
}

void irq()
{
  // Get the number of the highest priority active IRQ/FIQ
  u32int activeIrqNumber = getIrqNumberBE();
  switch(activeIrqNumber)
  {
    case GPT1_IRQ:
      scheduleGuest();
      gptBEClearOverflowInterrupt(1);
      acknowledgeIrqBE();
      break;
    case GPT2_IRQ:
    {
      if(!isGuestIrqMasked(activeIrqNumber))
      {
        throwInterrupt(activeIrqNumber);
      }
      gptBEClearOverflowInterrupt(2);
      acknowledgeIrqBE();
      break;
    }
    default:
      serial_putstring("Received IRQ=");
      serial_putint(activeIrqNumber);
      DIE_NOW(0, "irq: unimplemented IRQ number.");
  }

  /* Because the writes are posted on an Interconnect bus, to be sure
   * that the preceding writes are done before enabling IRQs/FIQs,
   * a Data Synchronization Barrier is used. This operation ensure that
   * the IRQ/FIQ line is de-asserted before IRQ/FIQ enabling. */
  asm volatile("MOV R0, #0\n\t"
               "MCR P15, #0, R0, C7, C10, #4"
               : : : "memory");

  return;
}


void irqPrivileged()
{
  // Get the number of the highest priority active IRQ/FIQ
  u32int activeIrqNumber = getIrqNumberBE();
  switch(activeIrqNumber)
  {
    case GPT1_IRQ:
      gptBEClearOverflowInterrupt(1);
      acknowledgeIrqBE();
      break;
    case GPT2_IRQ:
      if(!isGuestIrqMasked(activeIrqNumber))
      {
        throwInterrupt(activeIrqNumber);
      }
      gptBEClearOverflowInterrupt(2);
      acknowledgeIrqBE();
      break;
    default:
      serial_putstring("Received IRQ=");
      serial_putint(activeIrqNumber);
      DIE_NOW(0, "irqPrivileged: unimplemented IRQ number.");
  }

  /* Because the writes are posted on an Interconnect bus, to be sure
   * that the preceding writes are done before enabling IRQs/FIQs,
   * a Data Synchronization Barrier is used. This operation ensure that
   * the IRQ/FIQ line is de-asserted before IRQ/FIQ enabling. */
  asm volatile("MOV R0, #0\n\t"
               "MCR P15, #0, R0, C7, C10, #4"
               : : : "memory");
  return;
}


void fiq(void)
{
  DIE_NOW(getGuestContext(), "fiq: FIQ handler unimplemented!");
}
