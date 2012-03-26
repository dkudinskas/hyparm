#include "common/debug.h"
#include "common/stddef.h"

#include "cpuArch/armv7.h"
#include "cpuArch/constants.h"

#include "drivers/beagle/be32kTimer.h"
#include "drivers/beagle/beGPTimer.h"
#include "drivers/beagle/beIntc.h"
#include "drivers/beagle/beUart.h"

#include "exceptions/exceptionHandlers.h"

#include "guestManager/blockCache.h"
#include "guestManager/guestExceptions.h"
#include "guestManager/scheduler.h"

#include "instructionEmu/loadStoreDecode.h"
#include "instructionEmu/loopDetector.h"
#include "instructionEmu/scanner.h"

#include "memoryManager/shadowMap.h"
#include "memoryManager/memoryConstants.h"

#include "vm/omap35xx/gptimer.h"
#include "vm/omap35xx/intc.h"
#include "vm/omap35xx/uart.h"


#ifdef CONFIG_EXCEPTION_HANDLERS_COUNT_DATA_ABORT

u64int dataAbortCounter;

static inline void incrementDataAbortCounter(void);

u64int getDataAbortCounter()
{
  return dataAbortCounter;
}

static inline void incrementDataAbortCounter()
{
  dataAbortCounter++;
}

void resetDataAbortCounter()
{
  dataAbortCounter = 0;
}

#else

#define incrementDataAbortCounter()

#endif /* CONFIG_EXCEPTION_HANDLERS_COUNT_DATA_ABORT */


#ifdef CONFIG_EXCEPTION_HANDLERS_COUNT_IRQ

u64int irqCounter;

static inline void incrementIrqCounter(void);

u64int getIrqCounter()
{
  return irqCounter;
}

static inline void incrementIrqCounter()
{
  irqCounter++;
}

void resetIrqCounter()
{
  irqCounter = 0;
}

#else

#define incrementIrqCounter()

#endif /* CONFIG_EXCEPTION_HANDLERS_COUNT_IRQ */


#ifdef CONFIG_EXCEPTION_HANDLERS_COUNT_SVC

u64int svcCounter;

static inline void incrementSvcCounter(void);

u64int getSvcCounter()
{
  return svcCounter;
}

static inline void incrementSvcCounter()
{
  svcCounter++;
}

void resetSvcCounter()
{
  svcCounter = 0;
}

#else

#define incrementSvcCounter()

#endif /* CONFIG_EXCEPTION_HANDLERS_COUNT_SVC */


#ifdef CONFIG_LOOP_DETECTOR

/*
 * We do not care about initializing this variable, because the loop detector is reset during boot.
 * If the initial value would evaluate to TRUE, an extra reset happens at start.
 */
bool mustResetLoopDetector;

static inline void delayResetLoopDetector(void)
{
  mustResetLoopDetector = TRUE;
}

static inline void resetLoopDetectorIfNeeded(GCONTXT *context)
{
  if (mustResetLoopDetector)
  {
    resetLoopDetector(context);
    mustResetLoopDetector = FALSE;
  }
}

#else

#define delayResetLoopDetector()
#define resetLoopDetectorIfNeeded(context)

#endif /* CONFIG_LOOP_DETECTOR */


GCONTXT *softwareInterrupt(GCONTXT *context, u32int code)
{
  incrementSvcCounter();

  DEBUG(EXCEPTION_HANDLERS, "softwareInterrupt(%x)" EOL, code);

  // parse the instruction to find the start address of next block
  u32int nextPC = 0;
  bool gSVC = FALSE;
  instructionHandler instrHandler;

#ifdef CONFIG_THUMB2
  /* Make sure that any SVC that is not part of the scanner
   * will be delivered to the guest */
  if (context->CPSR & PSR_T_BIT) // Thumb
  {
    if (code == 0) // svc in Thumb is between 0x01 and 0xFF
    {
      DEBUG(EXCEPTION_HANDLERS, "softwareInterrupt %#.2x @ %#.8x is a guest system call" EOL, code, context->R15);
      gSVC = TRUE;
    }
  }
  else
  {
    if (code <= 0xFF)
    {
      DEBUG(EXCEPTION_HANDLERS, "softwareInterrupt %#.2x @ %#.8x is a guest system call" EOL, code, context->R15);
      gSVC = TRUE;
    }
  }
#else
  if (code <= 0xFF)
  {
    DEBUG(EXCEPTION_HANDLERS, "softwareInterrupt %#.2x @ %#.8x is a guest system call" EOL, code, context->R15);
    gSVC = TRUE;
  }
#endif

  u32int cpsrOld = context->CPSR;
  // Do we need to forward it to the guest?
  if (gSVC)
  {
    deliverServiceCall(context);
    nextPC = context->R15;
  }
  else
  {
    // get interpreter function pointer and call it
    instrHandler = context->hdlFunct;
    nextPC = instrHandler(context, context->endOfBlockInstr);
  }

  if (nextPC == 0)
  {
    DIE_NOW(context, "softwareInterrupt: Invalid nextPC. Instr to implement?");
  }

  traceBlock(context, nextPC);
  context->R15 = nextPC;

  /* Maybe a timer interrupt is pending on real INTC but hasn't been acked yet */
  if (context->guestIrqPending)
  {
    if ((context->CPSR & PSR_I_BIT) == 0)
    {
      deliverInterrupt(context);
    }
  }

  if (((cpsrOld & PSR_MODE) != PSR_USR_MODE) &&
      ((context->CPSR & PSR_MODE) == PSR_USR_MODE))
  {
    // guest was in privileged mode, after interpreting switched to user mode.
    // return from exception, CPS or MSR. act accordingly
    guestToUserMode();
  }
  if (((cpsrOld & PSR_MODE) == PSR_USR_MODE) &&
      ((context->CPSR & PSR_MODE) != PSR_USR_MODE))
  {
    // guest was in user mode. we hit a guest SVC. switch guest to privileged mode
    // return from exception, CPS or MSR. act accordingly
    guestToPrivMode();
  }

  DEBUG(EXCEPTION_HANDLERS, "softwareInterrupt: Next PC = %#.8x" EOL, nextPC);

  if ((context->CPSR & PSR_MODE) != PSR_USR_MODE)
  {
    /*
     * guest in privileged mode! scan...
     * We need to reset the loop detector here because the block trace contains 'return' blocks of
     * guest SVCs!
     */
    resetLoopDetectorIfNeeded(context);
    runLoopDetector(context);
    setScanBlockCallSource(SCANNER_CALL_SOURCE_SVC);
    scanBlock(context, context->R15);
  }
  else
  {
    printf("SVC: returning to user mode, pc %08x\n", context->R15);
    delayResetLoopDetector();
  }

  return context;
}


GCONTXT *dataAbort(GCONTXT *context)
{
  /*
   * Make sure interrupts are disabled while we deal with data abort.
   */
  disableInterrupts();
  incrementDataAbortCounter();
  /* Encodings: Page 1289 & 1355 */
  u32int dfar = getDFAR();
  DFSR dfsr = getDFSR();
  DEBUG_MMC(EXCEPTION_HANDLERS_TRACE_DABT, "dataAbort: DFAR=%#.8x DFSR=%#.8x" EOL, dfar, *(u32int *)&dfsr);
  u32int faultStatus = (dfsr.fs3_0) | (dfsr.fs4 << 4);
  switch (faultStatus)
  {
    case dfsPermissionSection:
    case dfsPermissionPage:
    {
      dabtPermissionFault(context, dfsr, dfar);
      break;
    }
    case dfsTranslationSection:
    case dfsTranslationPage:
    {
      dabtTranslationFault(context, dfsr, dfar);
      break;
    }
    case dfsSyncExternalAbt:
    case dfsAlignmentFault:
    case dfsDebugEvent:
    case dfsAccessFlagSection:
    case dfsIcacheMaintenance:
    case dfsAccessFlagPage:
    case dfsDomainSection:
    case dfsDomainPage:
    case dfsTranslationTableWalkLvl1SyncExtAbt:
    case dfsTranslationTableWalkLvl2SyncExtAbt:
    case dfsImpDepLockdown:
    case dfsAsyncExternalAbt:
    case dfsMemAccessAsyncParityErr:
    case dfsMemAccessAsyncParityERr2:
    case dfsImpDepCoprocessorAbort:
    case dfsTranslationTableWalkLvl1SyncParityErr:
    case dfsTranslationTableWalkLvl2SyncParityErr:
    default:
      printf("unimplemented user data abort %#.8x" EOL, faultStatus);
      printDataAbort();
      DIE_NOW(context, "unimplemented user data abort");
  }
  enableInterrupts();
  return context;
}


void dataAbortPrivileged(u32int pc)
{
  incrementDataAbortCounter();

  u32int dfar = getDFAR();
  DFSR dfsr = getDFSR();
  DEBUG_MMC(EXCEPTION_HANDLERS_TRACE_DABT, "dataAbortPrivileged: PC=%#.8x DFAR=%#.8x DFSR=%#.8x"
        EOL, pc, dfar, *(u32int *)&dfsr);

  u32int faultStatus = (dfsr.fs3_0) | (dfsr.fs4 << 4);
  switch(faultStatus)
  {
    case dfsTranslationSection:
    case dfsTranslationPage:
    {
      dabtTranslationFault(getGuestContext(), dfsr, dfar);
      break;
    }
    case dfsAlignmentFault:
    case dfsDebugEvent:
    case dfsAccessFlagSection:
    case dfsIcacheMaintenance:
    case dfsAccessFlagPage:
    case dfsSyncExternalAbt:
    case dfsDomainSection:
    case dfsDomainPage:
    case dfsTranslationTableWalkLvl1SyncExtAbt:
    case dfsPermissionSection:
    case dfsTranslationTableWalkLvl2SyncExtAbt:
    case dfsPermissionPage:
    case dfsImpDepLockdown:
    case dfsAsyncExternalAbt:
    case dfsMemAccessAsyncParityErr:
    case dfsMemAccessAsyncParityERr2:
    case dfsImpDepCoprocessorAbort:
    case dfsTranslationTableWalkLvl1SyncParityErr:
    case dfsTranslationTableWalkLvl2SyncParityErr:
    default:
      printf("dataAbortPrivileged: UNIMPLEMENTED data abort type.\n");
      printDataAbort();
      DIE_NOW(0, "Entering infinite loop\n");
      break;
  }
}

GCONTXT *undefined(GCONTXT *context)
{
  DIE_NOW(context, "undefined: undefined handler, Implement me!");
}

void undefinedPrivileged(void)
{
  DIE_NOW(NULL, "undefinedPrivileged: Undefined handler, privileged mode. Implement me!");
}

GCONTXT *prefetchAbort(GCONTXT *context)
{
  /*
   * Make sure interrupts are disabled while we deal with prefetch abort.
   */
  disableInterrupts();

  IFSR ifsr = getIFSR();
  u32int ifar = getIFAR();
  u32int faultStatus = (ifsr.fs3_0) | (ifsr.fs4 << 4);

  switch(faultStatus)
  {
    case ifsTranslationFaultSection:
    case ifsTranslationFaultPage:
    {
      iabtTranslationFault(context, ifsr, ifar);
      break;
    }
    case ifsTranslationTableWalk2ndLvllSynchExtAbt:
#ifdef CONFIG_GUEST_FREERTOS
      if (shouldPrefetchAbort(ifar))
      {
        deliverPrefetchAbort(context);
        setScanBlockCallSource(SCANNER_CALL_SOURCE_PABT_FREERTOS);
        scanBlock(context, context->R15);
      }
      break;
#endif
    case ifsDebugEvent:
    case ifsAccessFlagFaultSection:
    case ifsAccessFlagFaultPage:
    case ifsSynchronousExternalAbort:
    case ifsDomainFaultSection:
    case ifsDomainFaultPage:
    case ifsTranslationTableTalk1stLvlSynchExtAbt:
    case ifsPermissionFaultSection:
    case ifsPermissionFaultPage:
    case ifsImpDepLockdown:
    case ifsMemoryAccessSynchParityError:
    case ifsImpDepCoprocessorAbort:
    case ifsTranslationTableWalk1stLvlSynchParityError:
    case ifsTranslationTableWalk2ndLvlSynchParityError:
    default:
      printPrefetchAbort();
      DIE_NOW(context, "Unimplemented guest prefetch abort.");
  }
  enableInterrupts();
  return context;
}

void prefetchAbortPrivileged(void)
{
  DIE_NOW(0, "prefetchAbortPrivileged: unimplemented");
  disableInterrupts();
  IFSR ifsr = getIFSR();
  u32int faultStatus = (ifsr.fs3_0) | (ifsr.fs4 << 4);
  switch(faultStatus)
  {
    case ifsTranslationFaultPage:
    case ifsDebugEvent:
    case ifsAccessFlagFaultSection:
    case ifsTranslationFaultSection:
    case ifsAccessFlagFaultPage:
    case ifsSynchronousExternalAbort:
    case ifsDomainFaultSection:
    case ifsDomainFaultPage:
    case ifsTranslationTableTalk1stLvlSynchExtAbt:
    case ifsPermissionFaultSection:
    case ifsTranslationTableWalk2ndLvllSynchExtAbt:
    case ifsPermissionFaultPage:
    case ifsImpDepLockdown:
    case ifsMemoryAccessSynchParityError:
    case ifsImpDepCoprocessorAbort:
    case ifsTranslationTableWalk1stLvlSynchParityError:
    case ifsTranslationTableWalk2ndLvlSynchParityError:
    default:
      printPrefetchAbort();
      DIE_NOW(NULL, "Unimplemented privileged prefetch abort.");
   }
}

GCONTXT *monitorMode(GCONTXT *context)
{
  /*
   * TODO
   * Does the omap 3 implement monitor/secure mode?
   * Niels: yes it does!
   */
  DIE_NOW(context, "monitorMode: monitor/secure mode handler, Implement me!");
}

void monitorModePrivileged(void)
{
  /*
   * TODO
   * Does the omap 3 implement monitor/secure mode?
   * Niels: yes it does!
   */
  DIE_NOW(NULL, "monitorMode: monitor/secure mode handler, privileged mode. Implement me!");
}

GCONTXT *irq(GCONTXT *context)
{
  incrementIrqCounter();

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
      throwInterrupt(activeIrqNumber);
      /*
       * FIXME: figure out which interrupt to clear and then clear the right one?
       */
      gptBEClearOverflowInterrupt(2);
#ifdef CONFIG_GUEST_FREERTOS
      if (context->os == GUEST_OS_FREERTOS)
      {
        gptBEResetCounter(2);
        gptBEClearMatchInterrupt(2);
      }
#endif
      acknowledgeIrqBE();
      break;
    case UART3_IRQ:
    {
      /*
       * FIXME: Niels: are we sure we're supposed to read characters unconditionally?
       */
      // read character from UART
      u8int c = serialGetcAsync();
      acknowledgeIrqBE();
      // forward character to emulated UART
      uartPutRxByte(c, 3);
      break;
    }
    default:
      printf("Received IRQ = %x" EOL, activeIrqNumber);
      DIE_NOW(context, "irq: unimplemented IRQ number");
  }

  /* Because the writes are posted on an Interconnect bus, to be sure
   * that the preceding writes are done before enabling IRQs/FIQs,
   * a Data Synchronization Barrier is used. This operation ensure that
   * the IRQ/FIQ line is de-asserted before IRQ/FIQ enabling. */
  asm volatile("MOV R0, #0\n\t"
               "MCR P15, #0, R0, C7, C10, #4"
               : : : "memory");
  return context;
}


void irqPrivileged()
{
  incrementIrqCounter();

  // Get the number of the highest priority active IRQ/FIQ
  u32int activeIrqNumber = getIrqNumberBE();
  switch(activeIrqNumber)
  {
    case GPT1_IRQ:
      gptBEClearOverflowInterrupt(1);
      acknowledgeIrqBE();
      break;
    case GPT2_IRQ:
      throwInterrupt(activeIrqNumber);
      /*
       * FIXME: figure out which interrupt to clear and then clear the right one?
       */
      gptBEClearOverflowInterrupt(2);
#ifdef CONFIG_GUEST_FREERTOS
      if (getGuestContext()->os == GUEST_OS_FREERTOS)
      {
        gptBEResetCounter(2);
        gptBEClearMatchInterrupt(2);
      }
#endif
      acknowledgeIrqBE();
      break;
    case UART3_IRQ:
    {
      /*
       * FIXME: Niels: are we sure we're supposed to read characters unconditionally?
       */
      // read character from UART
      u8int c = serialGetcAsync();
      acknowledgeIrqBE();
      // forward character to emulated UART
      uartPutRxByte(c, 3);
      break;
    }
    default:
      printf("Received IRQ = %#x" EOL, activeIrqNumber);
      DIE_NOW(NULL, "irqPrivileged: unimplemented IRQ number.");
  }

  /* Because the writes are posted on an Interconnect bus, to be sure
   * that the preceding writes are done before enabling IRQs/FIQs,
   * a Data Synchronization Barrier is used. This operation ensure that
   * the IRQ/FIQ line is de-asserted before IRQ/FIQ enabling. */
  asm volatile("MOV R0, #0\n\t"
               "MCR P15, #0, R0, C7, C10, #4"
               : : : "memory");
}


void fiq(void)
{
  DIE_NOW(NULL, "fiq: FIQ handler unimplemented!");
}



void dabtPermissionFault(GCONTXT * gc, DFSR dfsr, u32int dfar)
{
  // Check if the addr we have faulted on is caused by
  // a memory protection the hypervisor has enabled

//  printf("dabtPermissionFault: dfar %08x, priv %x, write %x\n",
//         dfar, isGuestInPrivMode(gc), dfsr.WnR);
  // ok, more stuff to do here as well! whilst a translation fault can mean we haven't yet shadow mapped
  // an address, permission faults can become spurious, when we have protected a memory region because it
  // mapped a guest page table at the time of shadow-mapping. If the page-table in question has been removed by
  // the guest, the hypervisor may have not removed the memory protection! therefore, we must take the faulting address
  // and validate the permission fault, and possibly remove the memory protection

  // the validation process is complicated by the fact that permission faults may be generated by several
  // different causes: gPT write-protection, hardware access, executed-code protection
  // DFAR is the virtual address that faulted upon access. its a permission fault - thus a page table entry MUST
  // exist. GET it now.

  // try to match this entry address/value pair with the list of all page table entry address/value pairs that
  // the hypervisor saved:
  // this saved list is generated when on demand shadow mapping entries, we check the underlying guets PA being mapped
  // and compare it with existing known page table guest PAs. if its a match, we must write-protect access to this
  // virtual address, and then add an entry to this list of 'remembered' mappings.

  // when validating the permission fault, if we find that the page table that cause the write-protect is no longer
  // there, lets remove the write-protection, and remove the appropriate entry from the linked list!
  if (gc->virtAddrEnabled)
  {
    if (shouldDataAbort(isGuestInPrivMode(gc), dfsr.WnR, dfar))
    {
      deliverDataAbort(gc);
      setScanBlockCallSource(SCANNER_CALL_SOURCE_DABT_GVA_PERMISSION);
      scanBlock(gc, gc->R15);
      return;
    }
  }

  // interpret the load/store
  emulateLoadStoreGeneric(gc, dfar);
  
  // load/store might still have failed if it was LDRT/STRT
  if (!gc->guestDataAbtPending)
  {
    // ONLY move to the next instruction, if the guest hasn't aborted...
#ifdef CONFIG_THUMB2
    if (gc->CPSR & PSR_T_BIT)
    {
      gc->R15 = gc->R15 + T16_INSTRUCTION_SIZE;
    }
    else
#endif
    {
      gc->R15 = gc->R15 + ARM_INSTRUCTION_SIZE;
    }
  }
  else
  {
    // deliver the abort!
    deliverDataAbort(gc);
    setScanBlockCallSource(SCANNER_CALL_SOURCE_DABT_PERMISSION);
    scanBlock(gc, gc->R15);
  }
}


void dabtTranslationFault(GCONTXT * gc, DFSR dfsr, u32int dfar)
{
  /* if we hit this - that means that:
     mem access address corresponding 1st level page table entry FAULT/reserved
     or corresponding 2nd level page table entry FAULT
     see if translation fault should be forwarded to the guest!
     if THAT fails, then really panic.
   */
//  printf("dabtTranslationFault: dfar %08x, priv %x, write %x\n",
//         dfar, isGuestInPrivMode(gc), dfsr.WnR);
  if (!shadowMap(dfar))
  {
    // failed to shadow map!
    if (shouldDataAbort(isGuestInPrivMode(gc), dfsr.WnR, dfar))
    {
      deliverDataAbort(gc);
      setScanBlockCallSource(SCANNER_CALL_SOURCE_DABT_TRANSLATION);
      scanBlock(gc, gc->R15);
      return;
    }
    else
    {
      DIE_NOW(0, "dabtTranslationFault: panic now!\n");
    }
  }
}


void iabtTranslationFault(GCONTXT * gc, IFSR ifsr, u32int ifar)
{
  /* if we hit this - that means that:
     mem access address corresponding 1st level page table entry FAULT/reserved
     or corresponding 2nd level page table entry FAULT
     see if translation fault should be forwarded to the guest!
     if THAT fails, then really panic.
   */
//  printf("iabtTranslationFault: ifar %08x, priv %x\n", ifar, isGuestInPrivMode(gc));
  if (!shadowMap(ifar))
  {
    // failed to shadow map!
    if (shouldPrefetchAbort(isGuestInPrivMode(gc), ifar))
    {
      deliverPrefetchAbort(gc);
      setScanBlockCallSource(SCANNER_CALL_SOURCE_PABT_TRANSLATION);
      scanBlock(gc, gc->R15);
      return;
    }
    else
    {
      DIE_NOW(0, "iabtTranslationFault: panic now!\n");
    }
  }
}
