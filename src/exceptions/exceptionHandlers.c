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
    deliverInterrupt();
  }

  dumpLinuxFunctionInfo(gContext->R15);
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
      /* Check if the addr we have faulted on is caused by a memory protection the hypervisor has enabled */
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
      printDataAbort();
      dumpGuestContext(getGuestContext());
      DIE_NOW(0, "Translation section data abort.");
      break;
    case translation_page:
      printDataAbort();
      dumpGuestContext(getGuestContext());
      DIE_NOW(0, "Translation page data abort.");
      break;
    default:
      serial_putstring("Unimplemented user data abort.");
      serial_newline();
      printDataAbort();
      dumpGuestContext(getGuestContext());
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
    default:
      serial_putstring("UNIMPLEMENTED data abort type. (exceptionHandlers.c).");
      serial_newline();


      GCONTXT* gContext = getGuestContext();
      dumpGuestContext(gContext);

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

  /* show us the context */
  GCONTXT* gContext = getGuestContext();
  dumpGuestContext(gContext);

  DIE_NOW(0, "Entering infinite loop.");
}

void do_undefined_hypervisor(void)
{
  serial_putstring("exceptionHandlers: Undefined handler (Privileged/Hypervisor), Implement me!");
  serial_newline();

  /* Show us the context */
  GCONTXT* gContext = getGuestContext();
  dumpGuestContext(gContext);

  DIE_NOW(0, "Entering infinite loop.");
}

void do_prefetch_abort(void)
{
  serial_putstring("exceptionHandlers: prefetch abort handler, Implement me!");
  serial_newline();

  printPrefetchAbort();

  /* show us the context */
  GCONTXT* gContext = getGuestContext();
  dumpGuestContext(gContext);

  DIE_NOW(0, "Entering Infinite Loop.");
  //Never returns
}

void do_prefetch_abort_hypervisor(void)
{
  serial_putstring("Hypervisor Prefetch Abort");
  serial_newline();

  printPrefetchAbort();
  dumpGuestContext(getGuestContext());

  DIE_NOW(0, "Entering Infinite Loop.");
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

  DIE_NOW(0, "Entering Infinite Loop.");
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

  DIE_NOW(0, "Entering Infinite Loop.");
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
  DIE_NOW(0, "Received FIQ! Implement me.");
}

void dumpLinuxFunctionInfo(u32int nextPC)
{
  switch(nextPC)
  {
    case 0xc00086e0:
      serial_putstring("LINUX: start_kernel");
      serial_newline();
      break;
    case 0xc0008324:
      serial_putstring("LINUX: smp_setup_processor_id");
      serial_newline();
      break;
    case 0xc001324c:
      serial_putstring("LINUX: tick_init");
      serial_newline();
      break;
    case 0xc000e540:
      serial_putstring("LINUX: setup_arch");
      serial_newline();
      break;
    case 0xc0011b28:
      serial_putstring("LINUX: sched_init");
      serial_newline();
      break;
    case 0xc006d004:
      serial_putstring("LINUX: __build_all_zonelists");
      serial_newline();
      break;
    case 0xc001437c:
      serial_putstring("LINUX: page_alloc_init");
      serial_newline();
      break;
    case 0xc005141c:
      serial_putstring("LINUX: parse_args");
      serial_newline();
      break;
    case 0xc0008824:
      serial_putstring("LINUX: bug. interrupts where enabled *very very* early.");
      serial_newline();
      break;
    case 0xc0012924:
      serial_putstring("LINUX: sort_main_extable");
      serial_newline();
      break;
    case 0xc000ebe8:
      serial_putstring("LINUX: trap_init");
      serial_newline();
      break;
    case 0xc0012910:
      serial_putstring("LINUX: rcu_init");
      serial_newline();
      break;
    case 0xc000e120:
      serial_putstring("LINUX: init_IRQ");
      serial_newline();
      break;
    case 0xc0012868:
      serial_putstring("LINUX: pidhash_init");
      serial_newline();
      break;
    case 0xc001256c:
      serial_putstring("LINUX: init_timers");
      serial_newline();
      break;
    case 0xc0012d74:
      serial_putstring("LINUX: hrtimers_init");
      serial_newline();
      break;
    case 0xc0012240:
      serial_putstring("LINUX: softirq_init");
      serial_newline();
      break;
    case 0xc0012f28:
      serial_putstring("LINUX: timekeeping_init");
      serial_newline();
      break;
    case 0xc000eb84:
      serial_putstring("LINUX: time_init");
      serial_newline();
      break;
    case 0xc0058320:
      serial_putstring("LINUX: sched_clock_init");
      serial_newline();
      break;
    case 0xc0008868:
      serial_putstring("LINUX: enable interrupts!");
      serial_newline();
      break;
    case 0xc000886c:
      serial_putstring("LINUX: jump to console_init.");
      serial_newline();
      break;
    case 0xc00177a4:
      serial_putstring("LINUX: console_init");
      serial_newline();
      break;
    case 0xc016ee94:
      serial_putstring("LINUX: PANIC!!!");
      serial_newline();
      break;
    case 0xc0014c80:
      serial_putstring("LINUX: vmalloc_init");
      serial_newline();
      break;
    case 0xc0015494:
      serial_putstring("LINUX: vfs_caches_init_early");
      serial_newline();
      break;
    case 0xc000f1f0:
      serial_putstring("LINUX: mem_init");
      serial_newline();
      break;
    case 0xc0014ee4:
      serial_putstring("LINUX: kmem_cache_init");
      serial_newline();
      break;
    case 0xc00172ec:
      serial_putstring("LINUX: idr_init_cache");
      serial_newline();
      break;
    case 0xc001ad78:
      serial_putstring("LINUX: calibrate_delay");
      serial_newline();
      break;
    case 0xc0008378:
      serial_putstring("LINUX: kernel_init");
      serial_newline();
      break;
    case 0xc0011aa0:
      serial_putstring("LINUX: sched_init_smp");
      serial_newline();
      break;
    case 0xc001275c:
      serial_putstring("LINUX: init_workqueues");
      serial_newline();
      break;
    case 0xc0012700:
      serial_putstring("LINUX: usermodehelper_init");
      serial_newline();
      break;
    case 0xc0018cc4:
      serial_putstring("LINUX: driver_init");
      serial_newline();
      break;
    case 0xc0066744:
      serial_putstring("LINUX: init_irq_proc");
      serial_newline();
      break;
    case 0xc001f240:
      serial_putstring("LINUX: do_one_initcall");
      serial_newline();
      break;
    case 0xc004fd34:
      serial_putstring("LINUX: flush_scheduled_work");
      serial_newline();
      break;
    case 0xc008a214:
      serial_putstring("LINUX: sys_access");
      serial_newline();
      break;
    case 0xc0008f94:
      serial_putstring("LINUX: prepare_namespace");
      serial_newline();
      break;
    case 0xc001f3dc:
      serial_putstring("LINUX: init_post");
      serial_newline();
      break;
  }
}
