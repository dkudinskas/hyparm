#include "common/debug.h"

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


#ifdef CONFIG_GUEST_FREERTOS
extern bool rtos;
#endif


GCONTXT *softwareInterrupt(GCONTXT *context, u32int code)
{
  DEBUG(EXCEPTION_HANDLERS, "softwareInterrupt(%x)\n", code);

  // parse the instruction to find the start address of next block
  u32int nextPC = 0;
#ifdef CONFIG_THUMB2
  bool gSVC = FALSE;
#endif
  u32int (*instrHandler)(GCONTXT * context);

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
    nextPC = instrHandler(context);
  }

  if (nextPC == 0)
  {
    DIE_NOW(context, "softwareInterrupt: Invalid nextPC. Instr to implement?");
  }

  int i = 0;
  for (i = BLOCK_HISOTRY_SIZE-1; i > 0; i--)
  {
    context->blockHistory[i] = context->blockHistory[i-1];
  }
  context->blockHistory[0] = nextPC;

  context->R15 = nextPC;

  // deliver interrupts
  /* Maybe a timer interrupt is pending on real INTC but
   * hasn't been acked yet
   */
  if (context->guestIrqPending)
  {
    if ((context->CPSR & CPSR_IRQ_DIS) == 0)
    {
      deliverInterrupt(context);
    }
  }

  DEBUG(EXCEPTION_HANDLERS, "softwareInterrupt: Next PC = 0x%x\n", nextPC);

  if ((context->CPSR & CPSR_MODE) != CPSR_MODE_USR)
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
   * FIXME: what is the following comment about???
   * Markos: Stop the timer so we can resume from where we stopped
   *
   * Make sure interrupts are disabled while we deal with data abort.
   */
  disableInterrupts();
  /* Encodings: Page 1289 & 1355 */
  u32int faultStatus = (getDFSR().fs3_0) | (getDFSR().fs4 << 4);
  switch (faultStatus)
  {
    case dfsPermissionSection:
    case dfsPermissionPage:
    {
      // Check if the addr we have faulted on is caused by
      // a memory protection the hypervisor has enabled
      DFSR dfsr = getDFSR();
      bool isPrivAccess = (context->CPSR & CPSR_MODE) == CPSR_MODE_USR ? FALSE : TRUE;
      if (context->virtAddrEnabled)
      {
#ifndef CONFIG_BLOCK_COPY
        if ( shouldDataAbort(isPrivAccess, dfsr.WnR, getDFAR()))
        {
          deliverDataAbort(context);
          setScanBlockCallSource(SCANNER_CALL_SOURCE_DABT_GVA_PERMISSION);
          scanBlock(context, context->R15);
          break;
        }
#endif
      }

      // interpret the load/store
      emulateLoadStoreGeneric(context, getDFAR());

      // load/store might still have failed if it was LDRT/STRT
      if (!context->guestDataAbtPending)
      {
        // ONLY move to the next instruction, if the guest hasn't aborted...
#ifdef CONFIG_THUMB2
        if(context->CPSR & PSR_T_BIT)
        {
          context->R15 = context->R15 + 2;
        }
        else
#endif
        {
          context->R15 = context->R15 + 4;
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
      DFSR dfsr = getDFSR();
      /*
       * Markos: I think this means that the guest was trying to write within its
       * allowed memory area in user mode
       */
      bool isPrivAccess = (context->CPSR & CPSR_MODE) == CPSR_MODE_USR ? FALSE : TRUE;
      if (shouldDataAbort(isPrivAccess, dfsr.WnR, getDFAR()))
      {
        deliverDataAbort(context);
        setScanBlockCallSource(SCANNER_CALL_SOURCE_DABT_TRANSLATION);
        scanBlock(context, context->R15);
      }
      break;
    }
    case dfsSyncExternalAbt:
    {
      printDataAbort();
      DIE_NOW(0, "dataAbort: synchronous external abort hit!");
    }
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
      printf("Unimplemented user data abort.\n");
      printDataAbort();
      DIE_NOW(0, "Entering infinite loop\n");
  }
  enableInterrupts();
  return context;
}

void dataAbortPrivileged(u32int pc)
{
  /* Here if we abort in a priviledged mode, i.e its the Hypervisors fault */
  printf("dataAbortPrivileged: Hypervisor dabt in priv mode @ pc %08x\n", pc);

  printDataAbort();
  u32int faultStatus = (getDFSR().fs3_0) | (getDFSR().fs4 << 4);
  switch(faultStatus)
  {
    case dfsTranslationSection:
    case dfsTranslationPage:
    {
      //Mostly likely trying to access a page of physical memory, just map it.
      u32int memAddr = getDFAR();
      if( (memAddr >= BEAGLE_RAM_START) && (memAddr <= BEAGLE_RAM_END) )
      {
        DIE_NOW(0, "Translation fault inside physical RAM range\n");
      }
      else
      {
        DIE_NOW(0, "Translation fault for area not in RAM!\n");
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
      printf("dataAbortPrivileged: UNIMPLEMENTED data abort type.\n");
      printDataAbort();
      DIE_NOW(0, "Entering infinite loop\n");
      break;
  }

  DIE_NOW(0, "At end of hypervisor data abort handler. Stopping\n");
}

GCONTXT *undefined(GCONTXT *context)
{
  DIE_NOW(0, "undefined: undefined handler, Implement me!\n");
}

void undefinedPrivileged(void)
{
  DIE_NOW(0, "undefinedPrivileged: Undefined handler, privileged mode. Implement me!\n");
}

GCONTXT *prefetchAbort(GCONTXT *context)
{
  /*
   * FIXME: what is the following comment about???
   * Markos: Stop the time so we can resume from where we stopped
   *
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
      /*
       * FIXME: with CONFIG_GUEST_FREERTOS, all above cases still fall through and will end up in the new code. Is this correct?
       */
#ifdef CONFIG_GUEST_FREERTOS
      if (shouldPrefetchAbort(ifar))
      {
        deliverPrefetchAbort(context);
        setScanBlockCallSource(SCANNER_CALL_SOURCE_PABT_FREERTOS);
        scanBlock(context, context->R15);
      }
      break;
#endif
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
      DIE_NOW(0, "Unimplemented privileged prefetch abort.");
   }
}

GCONTXT *monitorMode(GCONTXT *context)
{
  /*
   * TODO
   * Does the omap 3 implement monitor/secure mode?
   * Niels: yes it does!
   */
  DIE_NOW(0, "monitorMode: monitor/secure mode handler, Implement me!");
}

void monitorModePrivileged(void)
{
  /*
   * TODO
   * Does the omap 3 implement monitor/secure mode?
   * Niels: yes it does!
   */
  DIE_NOW(0, "monitorMode: monitor/secure mode handler, privileged mode. Implement me!");
}

GCONTXT *irq(GCONTXT *context)
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
      throwInterrupt(activeIrqNumber);
      gptBEClearOverflowInterrupt(2);
#ifdef CONFIG_GUEST_FREERTOS
      /*
       * FIXME: Niels: I think this is a dirty hack.
       * Shouldn't we figure out which interrupt to clear and then clear the right one?
       */
      if (rtos)
      {
        storeToGPTimer(2,GPT_REG_TCRR,0x0);
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
      printf("Received IRQ = %x\n", activeIrqNumber);
      DIE_NOW(0, "irq: unimplemented IRQ number.\n");
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
      gptBEClearOverflowInterrupt(2);
#ifdef CONFIG_GUEST_FREERTOS
      /*
       * FIXME: Niels: I think this is a dirty hack.
       * Shouldn't we figure out which interrupt to clear and then clear the right one?
       */
      if (rtos)
      {
        storeToGPTimer(2,GPT_REG_TCRR,0x0);
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
      printf("Received IRQ = %x\n", activeIrqNumber);
      DIE_NOW(0, "irqPrivileged: unimplemented IRQ number.");
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
  DIE_NOW(0, "fiq: FIQ handler unimplemented!");
}
