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

extern GCONTXT * getGuestContext(void);

void do_software_interrupt(u32int code)
{
#ifdef EXC_HDLR_DBG
  serial_putstring("exceptionHandlers: software interrupt ");
  serial_putint(code);
  serial_newline();
#endif
  /* parse the instruction to find the start address of next block */
  /* scan next block! */
  GCONTXT * gContext = getGuestContext();
  u32int nextPC = 0;
  u32int (*instrHandler)(GCONTXT * context);

  /* get evaluate function addr */
  instrHandler = gContext->hdlFunct;
  /* evaluate replaced instruction and get next pc */
  nextPC = instrHandler(gContext);

  if (nextPC == 0)
  {
    serial_putstring("exceptionHandlers: Invalid nextPC. Instr to implement?");
    serial_newline();
    serial_putstring("exceptionHandlers: Dumping gc");
    serial_newline();
    dumpGuestContext(gContext);
    serial_ERROR("exceptionHandlers: In infinite loop");
  }
  gContext->R15 = nextPC;
#ifdef EXC_HDLR_DBG
  serial_putstring("exceptionHandlers: Next PC = 0x");
  serial_putint(nextPC);
  serial_newline();
#endif
  scanBlock(gContext, nextPC);
}

void do_data_abort()
{
  switch(getDFSR().fs3_0)
  {
    case perm_section:
    case perm_page:
    {
      /* Check if the addr we have faulted on is caused by a memory protection the hypervisor has enabled */
      GCONTXT* gc = getGuestContext();

      // ATM dont expect anything else to permission fault except load/stores
      emulateLoadStoreGeneric(gc, getDFAR());
      gc->R15 = gc->R15 + 4;
      break;
    }
    case translation_section:
      printDataAbort();
      dumpGuestContext(getGuestContext());
      serial_ERROR("Translation section data abort.");
      break;
    case translation_page:
      printDataAbort();
      dumpGuestContext(getGuestContext());
      serial_ERROR("Translation page data abort.");
      break;
    default:
      serial_putstring("Unimplemented user data abort.");
      serial_newline();
      printDataAbort();
      dumpGuestContext(getGuestContext());
      serial_ERROR("Entering infinite loop");
  }
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
      ;
      //Mostly likely trying to access a page of physical memory, just map it.
      u32int memAddr = getDFAR();
      if( (memAddr >= BEAGLE_RAM_START) && (memAddr <= BEAGLE_RAM_END) )
      {
        serial_putstring("Fault inside physical RAM range.  hypervisor_page_fault (exceptionHandlers.c)");
        serial_newline();
        GCONTXT* gc = getGuestContext();
        dumpGuestContext(gc);

        if(gc->virtAddrEnabled)
        {
          serial_putstring("Dumping SHADOW PAGE TABLE");
          serial_newline();
          dumpPageTable(gc->PT_shadow);
          serial_putstring("Dumping GUEST OS PAGE TABLE");
          serial_newline();
          dumpPageTable(gc->PT_os);
        }
        serial_ERROR("Entering infinite loop");
      }
      else
      {
        serial_ERROR("Translation fault for area not in RAM! Entering Infinite Loop...");
        /*
        I imagine there will be a few areas that we will need to map for the hypervisor only
        But not right now.
        */
      }
      break;
    default:
      serial_putstring("UNIMPLEMENTED data abort type. (exceptionHandlers.c).");
      serial_newline();


      GCONTXT* gContext = getGuestContext();
      dumpGuestContext(gContext);

      serial_ERROR("Entering infinite loop");
      break;
  }


  serial_ERROR("At end of hypervisor data abort handler. Stopping");

  serial_putstring("Exiting data abort handler");
  serial_newline();

  //Should be fixed and ready to re-execute the offending isntruction
}

void do_undefined(void)
{
  serial_putstring("exceptionHandlers: undefined handler, Implement me!");
  serial_newline();

  /* show us the context */
  GCONTXT* gContext = getGuestContext();
  dumpGuestContext(gContext);

  serial_ERROR("Entering infinite loop.");
}

void do_undefined_hypervisor(void)
{
  serial_putstring("exceptionHandlers: Undefined handler (Privileged/Hypervisor), Implement me!");
  serial_newline();

  /* Show us the context */
  GCONTXT* gContext = getGuestContext();
  dumpGuestContext(gContext);

  serial_ERROR("Entering infinite loop.");
}

void do_prefetch_abort(void)
{
  serial_putstring("exceptionHandlers: prefetch abort handler, Implement me!");
  serial_newline();

  printPrefetchAbort();

  /* show us the context */
  GCONTXT* gContext = getGuestContext();
  dumpGuestContext(gContext);

  serial_ERROR("Entering Infinite Loop.");
  //Never returns
}

void do_prefetch_abort_hypervisor(void)
{
  serial_putstring("Hypervisor Prefetch Abort");
  serial_newline();

  printPrefetchAbort();

  serial_ERROR("Entering Infinite Loop.");
  //Never returns
}

void do_monitor_mode(void)
{
  serial_putstring("exceptionHandlers: monitor/secure mode handler, Implement me!");
  serial_newline();

  /* show us the context */
  GCONTXT* gContext = getGuestContext();
  dumpGuestContext(gContext);

  /* Does the omap 3 implement monitor/secure mode? */

  serial_ERROR("Entering Infinite Loop.");
  //Never returns
}

void do_monitor_mode_hypervisor(void)
{
  serial_putstring("exceptionHandlers: monitor/secure mode handler(privileged/Hypervisor), Implement me!");
  serial_newline();

  /* show us the context */
  GCONTXT* gContext = getGuestContext();
  dumpGuestContext(gContext);

  /* Does the omap 3 implement monitor/secure mode? */

  serial_ERROR("Entering Infinite Loop.");
  //Never returns
}

void do_irq()
{
  // Get the number of the highest priority active IRQ/FIQ
  u32int activeIrqNumber = getIrqNumberBE();

  if (activeIrqNumber == GPT1_IRQ)
  {
//    scheduleGuest();
    gptBEClearOverflowInterrupt(1);
    acknowledgeIrqBE();
    serial_putstring("?");
  }
  else
  {
    serial_putstring("Received IRQ=");
    serial_putint(activeIrqNumber);
    serial_ERROR(" Implement me!");
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

  if (activeIrqNumber == GPT1_IRQ)
  {
    serial_putchar('!');
//    scheduleGuest();
    gptBEClearOverflowInterrupt(1);
    acknowledgeIrqBE();
  }
  else
  {
    serial_putstring("Received IRQ=");
    serial_putint(activeIrqNumber);
    serial_ERROR(" Implement me!");
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
  serial_ERROR("Received FIQ! Implement me.");
}

