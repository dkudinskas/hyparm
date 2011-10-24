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
#include "guestManager/scheduler.h"
#include "guestManager/guestExceptions.h"

#include "vm/omap35xx/gptimer.h"
#include "vm/omap35xx/intc.h"
#include "vm/omap35xx/uart.h"

#include "instructionEmu/scanner.h"

#include "memoryManager/addressing.h"
#include "memoryManager/globalMemoryMapper.h"
#include "memoryManager/mmu.h"
#include "memoryManager/memoryConstants.h"


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


GCONTXT *softwareInterrupt(GCONTXT *context, u32int code)
{
  incrementSvcCounter();

  DEBUG(EXCEPTION_HANDLERS, "softwareInterrupt(%x)" EOL, code);

  // parse the instruction to find the start address of next block
  u32int nextPC = 0;
#ifdef CONFIG_THUMB2
  bool gSVC = FALSE;
#endif
  instructionHandler instrHandler;

#ifdef CONFIG_THUMB2
  /* Make sure that any SVC that is not part of the scanner
   * will be delivered to the guest
   */
  if (context->CPSR & PSR_T_BIT) // Thumb
  {
    if (code == 0) // svc in Thumb is between 0x01 and 0xFF
    {
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

  // Do we need to forward it to the guest?
  if (gSVC)
  {
    deliverServiceCall(context);
#else
  if (code <= 0xFF)
  {
    DEBUG(EXCEPTION_HANDLERS, "softwareInterrupt %#.2x @ %#.8x is a guest system call" EOL, code, context->R15);
    deliverServiceCall(context);
#endif
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

  // deliver interrupts
  /* Maybe a timer interrupt is pending on real INTC but
   * hasn't been acked yet
   */
  if (context->guestIrqPending)
  {
    if ((context->CPSR & PSR_I_BIT) == 0)
    {
      deliverInterrupt(context);
    }
  }

  DEBUG(EXCEPTION_HANDLERS, "softwareInterrupt: Next PC = %#.8x" EOL, nextPC);

  if ((context->CPSR & PSR_MODE) != PSR_USR_MODE)
  {
    // guest in privileged mode! scan...
    setScanBlockCallSource(SCANNER_CALL_SOURCE_SVC);
    scanBlock(context, context->R15);
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
  u32int dfar = getDFAR();
  DFSR dfsr = getDFSR();
  DEBUG_MMC(EXCEPTION_HANDLERS_TRACE_DABT, "dataAbort: DFAR=%#.8x DFSR=%#.8x" EOL, dfar, *(u32int *)&dfsr);
  /* Encodings: Page 1289 & 1355 */
  u32int faultStatus = (dfsr.fs3_0) | (dfsr.fs4 << 4);
  switch (faultStatus)
  {
    case dfsPermissionSection:
    case dfsPermissionPage:
    {
      // Check if the addr we have faulted on is caused by
      // a memory protection the hypervisor has enabled
      bool isPrivAccess = (context->CPSR & PSR_MODE) != PSR_USR_MODE;
      if (context->virtAddrEnabled)
      {
        if (shouldDataAbort(isPrivAccess, dfsr.WnR, dfar))
        {
          deliverDataAbort(context);
          setScanBlockCallSource(SCANNER_CALL_SOURCE_DABT_GVA_PERMISSION);
          scanBlock(context, context->R15);
          break;
        }
      }

      // interpret the load/store
      emulateLoadStoreGeneric(context, dfar);

      // load/store might still have failed if it was LDRT/STRT
      if (!context->guestDataAbtPending)
      {
        // ONLY move to the next instruction, if the guest hasn't aborted...
#ifdef CONFIG_THUMB2
        if (context->CPSR & PSR_T_BIT)
        {
          context->R15 = context->R15 + T16_INSTRUCTION_SIZE;
        }
        else
#endif
        {
          context->R15 = context->R15 + ARM_INSTRUCTION_SIZE;
        }
      }
      else
      {
        // deliver the abort!
        deliverDataAbort(context);
        setScanBlockCallSource(SCANNER_CALL_SOURCE_DABT_PERMISSION);
        scanBlock(context, context->R15);
      }
      break;
    }
    case dfsTranslationSection:
    case dfsTranslationPage:
    {
      /*
       * Markos: I think this means that the guest was trying to write within its
       * allowed memory area in user mode
       */
      bool isPrivAccess = (context->CPSR & PSR_MODE) != PSR_USR_MODE;
      if (shouldDataAbort(isPrivAccess, dfsr.WnR, dfar))
      {
        deliverDataAbort(context);
        setScanBlockCallSource(SCANNER_CALL_SOURCE_DABT_TRANSLATION);
        scanBlock(context, context->R15);
      }
      break;
    }
    case dfsSyncExternalAbt:
      printDataAbort();
      DIE_NOW(context, "synchronous external abort hit!");
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

  /* Here if we abort in a priviledged mode, i.e its the Hypervisors fault */
  printf("dataAbortPrivileged: Hypervisor dabt in priv mode @ pc %#.8x" EOL, pc);

  printDataAbort();
  u32int faultStatus = (dfsr.fs3_0) | (dfsr.fs4 << 4);
  switch(faultStatus)
  {
    case dfsTranslationSection:
    case dfsTranslationPage:
    {
      //Mostly likely trying to access a page of physical memory, just map it.
      if((dfar >= BEAGLE_RAM_START) && (dfar <= BEAGLE_RAM_END))
      {
        DIE_NOW(NULL, "Translation fault inside physical RAM range");
      }
      else
      {
        DIE_NOW(NULL, "Translation fault for area not in RAM!");
      }
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
      printf("dataAbortPrivileged: UNIMPLEMENTED data abort type.");
      printDataAbort();
      DIE_NOW(NULL, "Entering infinite loop");
      break;
  }

  DIE_NOW(NULL, "At end of hypervisor data abort handler. Stopping");
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
    case ifsTranslationFaultPage:
      if (shouldPrefetchAbort(ifar))
      {
        deliverPrefetchAbort(context);
        setScanBlockCallSource(SCANNER_CALL_SOURCE_PABT_TRANSLATION);
        scanBlock(context, context->R15);
      }
      break;
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
    case ifsTranslationFaultSection:
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
  disableInterrupts();
  IFSR ifsr = getIFSR();
  u32int faultStatus = (ifsr.fs3_0) | (ifsr.fs4 << 4);
  switch(faultStatus){
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
