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

void do_software_interrupt(u32int code)
{
#ifdef EXC_HDLR_DBG
  serial_putstring("exceptionHandlers: software interrupt ");
  serial_putint(code);
  serial_newline();
#endif
  // parse the instruction to find the start address of next block
  GCONTXT * gContext = getGuestContext();
  u32int nextPC = 0;
  u32int (*instrHandler)(GCONTXT * context);

  // get interpreter function pointer and call it
  instrHandler = gContext->hdlFunct;
  nextPC = instrHandler(gContext);

  if (nextPC == 0)
  {
    serial_putstring("exceptionHandlers: Invalid nextPC. Instr to implement?");
    serial_newline();
    serial_putstring("exceptionHandlers: Dumping gc");
    serial_newline();
    dumpGuestContext(gContext);
    DIE_NOW(0, "exceptionHandlers: In infinite loop");
  }

  int i = 0;
  for (i = BLOCK_HISOTRY_SIZE-1; i > 0; i--)
  {
    gContext->blockHistory[i] = gContext->blockHistory[i-1];
  }
  gContext->blockHistory[0] = nextPC; 

  gContext->R15 = nextPC;

  // deliver interrupts
  if (gContext->guestAbtPending)
  {
    dumpGuestContext(gContext);
    DIE_NOW(0, "Exception handlers: guest abort in SWI handler! implement.");
  }
  else if (gContext->guestIrqPending)
  {
    if ((gContext->CPSR & CPSR_IRQ_DIS) == 0)
    {
      deliverInterrupt();
    }
  }

#ifdef EXC_HDLR_DBG
  serial_putstring("exceptionHandlers: Next PC = 0x");
  serial_putint(nextPC);
  serial_newline();
#endif

  scanBlock(gContext, gContext->R15);
}

void do_data_abort()
{
  // make sure interrupts are disabled while we deal with data abort.
  disableInterrupts();
  switch(getDFSR().fs3_0)
  {
    case perm_section:
    case perm_page:
    {
      // Check if the addr we have faulted on is caused by 
      // a memory protection the hypervisor has enabled
      GCONTXT* gc = getGuestContext();

      // ATM dont expect anything else to permission fault except load/stores
      emulateLoadStoreGeneric(gc, getDFAR());
      if (!gc->guestAbtPending)
      {
        // ONLY move to the next instruction, if the guest hasn't aborted... 
        gc->R15 = gc->R15 + 4;
      }
      else
      {
        // deliver the abort!
        deliverAbort();
        scanBlock(gc, gc->R15);
      }
      break;
    }
    case translation_section:
    {
      GCONTXT* gc = getGuestContext();
      u32int dfar = getDFAR();
      DFSR dfsr = getDFSR();
      if (forwardTranslationAbort(dfar))
      {
        throwAbort(dfar, translation_section, dfsr.WnR, dfsr.domain);
        deliverAbort();
        scanBlock(gc, gc->R15);
      }
      break;
    }
    case translation_page:
      printDataAbort();
      DIE_NOW(0, "Translation page data abort.");
      break;
    default:
      serial_putstring("Unimplemented user data abort.");
      serial_newline();
      printDataAbort();
      DIE_NOW(0, "Entering infinite loop");
  }
  enableInterrupts();
}

void do_data_abort_hypervisor()
{
  /* Here if we abort in a priviledged mode, i.e its the Hypervisors fault */
  serial_putstring("exceptionHandlers: Hypervisor data abort in priviledged mode.");
  serial_newline();

  printDataAbort();

  switch(getDFSR().fs3_0)
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

void do_undefined(void)
{
  serial_putstring("exceptionHandlers: undefined handler, Implement me!");
  serial_newline();

  DIE_NOW(getGuestContext(), "Entering infinite loop.");
}

void do_undefined_hypervisor(void)
{
  serial_putstring("exceptionHandlers: Undefined handler (Privileged/Hypervisor), Implement me!");
  serial_newline();

  DIE_NOW(getGuestContext(), "Entering infinite loop.");
}

void do_prefetch_abort(void)
{
  serial_putstring("exceptionHandlers: prefetch abort handler, Implement me!");
  serial_newline();

  printPrefetchAbort();

  DIE_NOW(getGuestContext(), "Entering Infinite Loop.");
  //Never returns
}

void do_prefetch_abort_hypervisor(void)
{
  serial_putstring("Hypervisor Prefetch Abort");
  serial_newline();

  printPrefetchAbort();

  DIE_NOW(getGuestContext(), "Entering Infinite Loop.");
  //Never returns
}

void do_monitor_mode(void)
{
  serial_putstring("exceptionHandlers: monitor/secure mode handler, Implement me!");
  serial_newline();

  /* Does the omap 3 implement monitor/secure mode? */

  DIE_NOW(getGuestContext(), "Entering Infinite Loop.");
  //Never returns
}

void do_monitor_mode_hypervisor(void)
{
  serial_putstring("exceptionHandlers: monitor/secure mode handler(privileged/Hypervisor), Implement me!");
  serial_newline();

  /* Does the omap 3 implement monitor/secure mode? */

  DIE_NOW(getGuestContext(), "Entering Infinite Loop.");
  //Never returns
}

void do_irq()
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
        tickEvent(activeIrqNumber);
      }
      gptBEClearOverflowInterrupt(2);
      acknowledgeIrqBE();
      break;
    }
    default:
      serial_putstring("Received IRQ=");
      serial_putint(activeIrqNumber);
      DIE_NOW(0, " Implement me!");
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


void do_irq_hypervisor()
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
        tickEvent(activeIrqNumber);
      }
      gptBEClearOverflowInterrupt(2);
      acknowledgeIrqBE();
      break;
    default:
      serial_putstring("Received IRQ=");
      serial_putint(activeIrqNumber);
      DIE_NOW(0, " Implement me!");
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


void do_fiq(void)
{
  DIE_NOW(getGuestContext(), "Received FIQ! Implement me.");
}
