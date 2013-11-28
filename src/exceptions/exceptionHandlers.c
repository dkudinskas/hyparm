#include "common/debug.h"
#include "common/stddef.h"
#include "common/linker.h"
#ifdef CONFIG_PROFILER
#include "common/profiler.h"
#endif

#include "cpuArch/armv7.h"
#include "cpuArch/constants.h"

#include "drivers/beagle/be32kTimer.h"
#include "drivers/beagle/beGPTimer.h"
#include "drivers/beagle/beIntc.h"
#include "drivers/beagle/beUart.h"

#include "exceptions/exceptionHandlers.h"

#include "guestManager/basicBlockStore.h"
#include "guestManager/guestExceptions.h"
#include "guestManager/scheduler.h"

#include "instructionEmu/blockLinker.h"
#include "instructionEmu/loadStoreDecode.h"
#include "instructionEmu/loopDetector.h"
#include "instructionEmu/scanner.h"
#include "instructionEmu/translator/translator.h"

#include "memoryManager/shadowMap.h"
#include "memoryManager/memoryConstants.h"
#include "memoryManager/memoryProtection.h"

#include "perf/contextSwitchCounters.h"

#include "vm/omap35xx/gptimer.h"
#include "vm/omap35xx/intc.h"
#include "vm/omap35xx/uart.h"

#ifdef CONFIG_HW_PASSTHROUGH
  static bool IrqBitModified;
  static u8int oldIBit;
#endif

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
  // disable possible further interrupts for now
  disableInterrupts();
  bool link = TRUE;
  u32int nextPC = 0;
  bool gSVC = FALSE;
  BasicBlock* block = NULL;

  // asm context switch saved translated host pc into guest context R15
  // it will get overwritten by following code, but needed for block linking
  u32int lastTranslatedPC = context->R15;

#ifdef CONFIG_HW_PASSTHROUGH
  if (IrqBitModified)
  {
    IrqBitModified = FALSE;
    context->CPSR.bits.I = oldIBit;
  }
#endif

#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
  context->svcCount++;
#endif

  DEBUG(EXCEPTION_HANDLERS, "softwareInterrupt(%x)" EOL, code);

#ifdef CONFIG_THUMB2
  /* Make sure that any SVC that is not part of the scanner
   * will be delivered to the guest */
  if (context->CPSR.bits.T) // Thumb
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
  if (code < 0x100)
  {
    DEBUG(EXCEPTION_HANDLERS, "softwareInterrupt %#.2x @ %#.8x is a guest system call" EOL, code, context->R15);
    gSVC = TRUE;
  }
#endif

  // Do we need to forward it to the guest?
  if (gSVC)
  {
#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
    context->svcGuest++;
#endif
    deliverServiceCall(context);
    nextPC = context->R15;
    link = FALSE;
  }
  else
  {
    u32int blockStoreIndex = code - 0x100;
    block = getBasicBlockStoreEntry(context->translationStore, blockStoreIndex);
    context->R15 = (u32int)(block->guestEnd);

    registerSvc(context, block->handler);

    // interpret the instruction to find the start address of next block
    nextPC = block->handler(context, *block->guestEnd);
  }

  traceBlock(context, nextPC);
  context->R15 = nextPC;

  /* Maybe an interrupt is pending but hasn't been delivered? */
  if (context->guestIrqPending)
  {
    if (context->CPSR.bits.I == 0)
    {
      deliverInterrupt(context);
      link = FALSE;
    }
  }
  else if (context->guestDataAbtPending)
  {
    deliverDataAbort(context);
    link = FALSE;
  }

  DEBUG(EXCEPTION_HANDLERS, "softwareInterrupt: Next PC = %#.8x" EOL, nextPC);

  if (context->CPSR.bits.mode != USR_MODE)
  {
    /* guest in privileged mode! scan...
     * We need to reset the loop detector here because the block trace
     * contains 'return' blocks of guest SVCs! */
    resetLoopDetectorIfNeeded(context);
    runLoopDetector(context);
    if (context->guestDataAbtPending)
    {
      setScanBlockCallSource(SCANNER_CALL_SOURCE_DABT_GVA_PERMISSION);
    }
    else
    {
      setScanBlockCallSource(SCANNER_CALL_SOURCE_SVC);
    }

    if (link && isBranch(*block->guestEnd))
    {
      linkBlock(context, context->R15, lastTranslatedPC, block);
    }

    scanBlock(context, context->R15);
  }
  else
  {
    // going to user mode.
    delayResetLoopDetector();
  }
  return context;
}


GCONTXT *dataAbort(GCONTXT *context)
{
  /* Make sure interrupts are disabled while we deal with data abort. */
  disableInterrupts();
#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
  context->dabtCount++;
  context->dabtUser++;
#endif
  /* Encodings: Page 1289 & 1355 */
  u32int dfar = getDFAR();
  DFSR dfsr = getDFSR();
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
    {
      printf("unimplemented user data abort %#.8x" EOL, faultStatus);
      printDataAbort();
      DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
    }
  }
  return context;
}


void dataAbortPrivileged(u32int pc, u32int sp, u32int spsr)
{
  /* Make sure interrupts are disabled while we deal with data abort. */
  disableInterrupts();
#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
  GCONTXT* context = getActiveGuestContext();
  context->dabtCount++;
  context->dabtPriv++;
#endif
  u32int dfar = getDFAR();
  DFSR dfsr = getDFSR();

  u32int faultStatus = (dfsr.fs3_0) | (dfsr.fs4 << 4);
  switch(faultStatus)
  {
    case dfsTranslationSection:
    case dfsTranslationPage:
    {
      dabtTranslationFault(getActiveGuestContext(), dfsr, dfar);
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
    {
      printf("dataAbortPrivileged pc %08x addr %08x" EOL, pc, dfar);
      printDataAbort();
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
      break;
    }
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
  /* Make sure interrupts are disabled while we deal with fetch abort. */
  disableInterrupts();
#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
  context->pabtCount++;
  context->pabtUser++;
#endif
  // Make sure interrupts are disabled while we deal with prefetch abort.
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
    {
      if (shouldPrefetchAbort(context, isGuestInPrivMode(context), ifar))
      {
        deliverPrefetchAbort(context);
        setScanBlockCallSource(SCANNER_CALL_SOURCE_PABT_FREERTOS);
        scanBlock(context, context->R15);
      }
      break;
    }
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
    {
      printPrefetchAbort();
      DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
    }
  }
  return context;
}


void prefetchAbortPrivileged(u32int pc, u32int sp, u32int spsr)
{
  printf("prefetchAbortPrivileged pc %08x sp %08x spsr %08x\n", pc, sp, spsr);

  // disable possible further interrupts
  disableInterrupts();
#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
  getActiveGuestContext()->pabtCount++;
  getActiveGuestContext()->pabtPriv++;
#endif

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
    {
      printPrefetchAbort();
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
  }
}


GCONTXT *monitorMode(GCONTXT *context)
{
  /*
   * TODO
   * Does the omap 3 implement monitor/secure mode?
   * Niels: yes it does!
   */
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}


void monitorModePrivileged(void)
{
  /*
   * TODO
   * Does the omap 3 implement monitor/secure mode?
   * Niels: yes it does!
   */
  DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
}


GCONTXT *irq(GCONTXT *context)
{
  disableInterrupts();
#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
  getActiveGuestContext()->irqCount++;
  getActiveGuestContext()->irqUser++;
#endif

#ifdef CONFIG_HW_PASSTHROUGH
  if (isGuestInPrivMode(context))
  {
    BasicBlock* block = getBasicBlockStoreEntry(context->translationStore, context->lastEntryBlockIndex);
    // now we must unlink the current block if it is in a group block.
    // to make sure the guest isn't waiting for our deferred interrupt forever
    if (block->type == GB_TYPE_ARM)
    {
      u32int index = findBlockIndexNumber(context, context->R15);
      BasicBlock* groupblock = getBasicBlockStoreEntry(context->translationStore, index);
      unlinkBlock(groupblock, index);
    }
    // we defer until next hypercall
    context->guestIrqPending = TRUE;
    IrqBitModified = TRUE;
    oldIBit = context->CPSR.bits.I;
    context->CPSR.bits.I = 1;;
  }
  else
  {
    // if guest is in user mode, and device passthrough is enabled
    // just deliver the interrupt to the guest
    // carry on guest from IRQ vector entry
    deliverInterrupt(context);
    setScanBlockCallSource(SCANNER_CALL_SOURCE_INTERRUPT);
    scanBlock(context, context->R15);
  }
#else
  // Get the number of the highest priority active IRQ
  u32int activeIrqNumber = getIrqNumberBE();
  switch(activeIrqNumber)
  {
    case GPT1_IRQ:
    {
      throwInterrupt(context, activeIrqNumber);
      BasicBlock* block = getBasicBlockStoreEntry(context->translationStore, context->lastEntryBlockIndex);
      // now we must unlink the current block if it is in a group block.
      // to make sure the guest isn't waiting for our deferred interrupt forever
      if (block->type == GB_TYPE_ARM)
      {
        u32int index = findBlockIndexNumber(context, context->R15);
        BasicBlock* groupblock = getBasicBlockStoreEntry(context->translationStore, index);
        unlinkBlock(groupblock, index);
      }
 
      // FIXME: figure out which interrupt to clear and then clear the right one?
      gptBEClearOverflowInterrupt(1);
#ifdef CONFIG_GUEST_FREERTOS
      if (context->os == GUEST_OS_FREERTOS)
      {
        gptBEResetCounter(1);
        gptBEClearMatchInterrupt(1);
      }
#endif
      acknowledgeIrqBE();
      break;
    }
    case UART3_IRQ:
    {
      // read character from UART
      if (serialCheckInput())
      {
        u8int c = serialGetcAsync();
        acknowledgeIrqBE();
        // forward character to emulated UART
        uartPutRxByte(context, c, 3);
      }
      else
      {
        // what interrupt??
        acknowledgeIrqBE();
      }
      break;
    }
    default:
    {
      printf("Received IRQ = %x" EOL, activeIrqNumber);
      DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
    }
  }
 
  /* Because the writes are posted on an Interconnect bus, to be sure
   * that the preceding writes are done before enabling IRQss,
   * a Data Synchronization Barrier is used. This operation ensure that
   * the IRQ line is de-asserted before IRQ enabling. */
  __asm__ __volatile__("MOV R0, #0\n\t"
               "MCR p15, #0, R0, c7, c10, #4"
               : : : "memory");
#endif

  return context;
}


void irqPrivileged()
{
  // disable possible further interrupts
  disableInterrupts();
#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
  getActiveGuestContext()->irqCount++;
  getActiveGuestContext()->irqPriv++;
#endif

#ifdef CONFIG_HW_PASSTHROUGH
  DIE_NOW(0, "irqPrivileged should not be here with HW passthrough.");
#else
  GCONTXT *const context = getActiveGuestContext();
  // Get the number of the highest priority active IRQ
  u32int activeIrqNumber = getIrqNumberBE();
  switch(activeIrqNumber)
  {
    case GPT1_IRQ:
    {
      throwInterrupt(context, activeIrqNumber);
      /*
       * FIXME: figure out which interrupt to clear and then clear the right one?
       */
      gptBEClearOverflowInterrupt(1);
#ifdef CONFIG_GUEST_FREERTOS
      if (context->os == GUEST_OS_FREERTOS)
      {
        gptBEResetCounter(1);
        gptBEClearMatchInterrupt(1);
      }
#endif
      acknowledgeIrqBE();
      break;
    }
    case UART3_IRQ:
    {
      // read character from UART
      if (serialCheckInput())
      {
        u8int c = serialGetcAsync();
        acknowledgeIrqBE();
        // forward character to emulated UART
        uartPutRxByte(context, c, 3);
      }
      else
      {
        // what interrupt??
        acknowledgeIrqBE();
      }
      break;
    }
    default:
    {
      printf("Received IRQ = %#x" EOL, activeIrqNumber);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
  }

  /* Because the writes are posted on an Interconnect bus, to be sure
   * that the preceding writes are done before enabling IRQss,
   * a Data Synchronization Barrier is used. This operation ensure that
   * the IRQ line is de-asserted before IRQ enabling. */
  __asm__ __volatile__("MOV R0, #0\n\t"
               "MCR p15, #0, R0, c7, c10, #4"
               : : : "memory");
#endif
}


void fiq(u32int addr)
{
#ifdef CONFIG_PROFILER
  GCONTXT *const context = getActiveGuestContext();

  u32int activeFiqNumber = getFiqNumberBE();
  switch (activeFiqNumber)
  {
    case GPT3_IRQ:
    {
      profilerRecord(addr);
      gptBEClearOverflowInterrupt(3);
      break;
    }
    default:
      DIE_NOW(NULL, "unimplemented FIQ handler.");
  }

  acknowledgeFiqBE();

  // write barrier
  asm volatile("MOV R0, #0\n\t"
               "MCR P15, #0, R0, C7, C10, #4"
               : : : "memory");
#else
   DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
 }


void dabtPermissionFault(GCONTXT *gc, DFSR dfsr, u32int dfar)
{
  // Check if the addr we have faulted on is caused by
  // a memory protection the hypervisor has enabled
  if (gc->virtAddrEnabled)
  {
    if (shouldDataAbort(gc, isGuestInPrivMode(gc), dfsr.WnR, dfar))
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
    if (gc->CPSR.bits.T)
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


void dabtTranslationFault(GCONTXT *context, DFSR dfsr, u32int dfar)
{
  /* if we hit this - that means that:
     mem access address corresponding 1st level page table entry FAULT/reserved
     or corresponding 2nd level page table entry FAULT
     see if translation fault should be forwarded to the guest!
     if THAT fails, then really panic.
   */
  if (!shadowMap(context, dfar))
  {
    // failed to shadow map!
    if (shouldDataAbort(context, isGuestInPrivMode(context), dfsr.WnR, dfar))
    {
      if (isGuestInPrivMode(context))
      {
        // we need to map host PC to guest PC
        context->R15 = hostpcToGuestpc(context);
        DEBUG(EXCEPTION_HANDLERS, "dabtTranslationFault: mapping PC got %08x\n", context->R15);
      }
      /*
       * here we must find the correct guest PC to store in R14_ABT
       * - if guest was running in user mode
       *   then code was executed in place, and context has the correct gPC
       * - if guest was privileged mode, guest code was executed from code store
       *   then we must find correct guest PC
       */
      deliverDataAbort(context);
      setScanBlockCallSource(SCANNER_CALL_SOURCE_DABT_TRANSLATION);
      scanBlock(context, context->R15);
      return;
    }
    else
    {
      DIE_NOW(NULL, "dabtTranslationFault: panic now!\n");
    }
  }
}


void iabtTranslationFault(GCONTXT *context, IFSR ifsr, u32int ifar)
{
  /* if we hit this - that means that:
     mem access address corresponding 1st level page table entry FAULT/reserved
     or corresponding 2nd level page table entry FAULT
     see if translation fault should be forwarded to the guest!
     if THAT fails, then really panic.
   */
  if (!shadowMap(context, ifar))
  {
    // failed to shadow map!
    if (shouldPrefetchAbort(context, isGuestInPrivMode(context), ifar))
    {
      if (isGuestInPrivMode(context))
      {
        // we need to map host PC to guest PC
        context->R15 = hostpcToGuestpc(context);
        DEBUG(EXCEPTION_HANDLERS, "dabtTranslationFault: mapping PC got %08x\n", context->R15);
      }
      deliverPrefetchAbort(context);
      setScanBlockCallSource(SCANNER_CALL_SOURCE_PABT_TRANSLATION);
      scanBlock(context, context->R15);
      return;
    }
    else
    {
      DIE_NOW(context, "iabtTranslationFault: panic now!\n");
    }
  }
}


