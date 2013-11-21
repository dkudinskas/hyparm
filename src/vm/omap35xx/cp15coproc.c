#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "memoryManager/addressing.h"
#include "memoryManager/mmu.h"

#include "vm/omap35xx/cp15coproc.h"


#define VBAR_ALIGN_MASK  ~0x1F


static void initialiseRegister(CREG *crb, Coprocessor15Register reg, u32int value) __cold__;


CREG *createCRB()
{
  CREG *crb = (CREG *)calloc(MAX_CRB_SIZE, sizeof(CREG));
  if (crb == NULL)
  {
    return NULL;
  }

  DEBUG(INTERPRETER_ANY_COPROC, "Initializing coprocessor reg bank @ address %p" EOL, crb);

  /* MIDR:
   * main ID register: CPU idenification including implementor code
   * read only. init to 411FC083
   * 41xxxxxx - implementer ID
   * xx1xxxxx - variant
   * xxxFxxxx - architecture
   * xxxxC08x - primary part number
   * xxxxxxx3 - revision */
  initialiseRegister(crb, CP15_MIDR, 0x411FC083);

  /* CTR:
   * cache type register: information on the architecture of caches
   * read only. init to 80048004 */
  initialiseRegister(crb, CP15_CTR, 0x80048004);

  /* IDPFR0:
   * processor feature register 0
   * read only */
#ifdef CONFIG_THUMB2
  // For the future... if ever.
  initialiseRegister(crb, CP15_IDPFR0, IDPFR0_ARM_ISA_SUPPORT | IDPFR0_THUMB2_ISA_SUPPORT);
#else
  initialiseRegister(crb, CP15_IDPFR0, IDPFR0_ARM_ISA_SUPPORT);
#endif

  /* MMFR0:
   * memory model feature register 0
   * read only, init to 31100003 */
  initialiseRegister(crb, CP15_MMFR0, 0x31100003);

  /* MMFR1:
   * memory model feature register 1
   * read only, init to 20000000 */
  initialiseRegister(crb, CP15_MMFR1, 0x20000000);

  /* CCSIDR:
   * cache size id register: provide information about the architecture of caches
   * CSSELR selects default level 0 data cache information: 0xE007E01A
   * WT, WB, RA able, no WA. 256 sets, associativity 4, words per line 16 */
  initialiseRegister(crb, CP15_CCSIDR, 0xE007E01A);

  /* CLIDR:
   * cache level ID register: beagle board 0x0A000023
   * lvl1 separate I&D caches, lvl2 unified cache */
  initialiseRegister(crb, CP15_CLIDR, 0x0A000023);

  /* CSSELR:
   * cache size select register: selects the current CCSIDR
   * initialize to lvl0, data cache (0x00000000) */
  initialiseRegister(crb, CP15_CSSELR, 0);

  /* SCTLR:
   * system control register, init to C5187A */
  initialiseRegister(crb, CP15_SCTRL, 0x00C5187A);

  /* ACTLR:
   * auxiliary control register, init to 0x00000002 (L2 cache enabled) */
  initialiseRegister(crb, CP15_ACTLR, 2);

  /* TTBR0:
   * translation table base register 0. initialise to 0. */
  initialiseRegister(crb, CP15_TTBR0, 0);

  /* TTBR1:
   * translation table base register 1. initialise to 0. */
  initialiseRegister(crb, CP15_TTBR1, 0);

  /* TTBCR:
   * translation table base control register: determines which TTBR to use */
  initialiseRegister(crb, CP15_TTBCR, 0);

  /* DACR:
   * domain access control register: initialise to dom0 ALL, dom1 ALL */
  initialiseRegister(crb, CP15_DACR, 0x0000000f);

  /* DFSR:
   * data fault status register: encodes information about the last dAbort
   * initialize to 0 */
  initialiseRegister(crb, CP15_DFSR, 0);

  /* IFSR:
   * instruction fault status register: encodes information about the last
   * prefetch abort. Initialize to 0 */
  initialiseRegister(crb, CP15_IFSR, 0);

  /* DFAR:
   * data fault address register: target address of last mem access that
   * caused a data abort. initialize to 0 */
  initialiseRegister(crb, CP15_DFAR, 0);

  /* IFAR:
   * instruction fault address register: target PC of the last instruction fetch
   * that caused a prefetch abort. Initialize to 0 */
  initialiseRegister(crb, CP15_IFAR, 0);

  /* ICIALLU:
   * invalidate all instruction caches to PoC, write-only */
  initialiseRegister(crb, CP15_ICIALLU, 0);

  /* ICIMVAU:
   * invalidate instruction caches by MVA to PoU, write-only */
  initialiseRegister(crb, CP15_ICIMVAU, 0);

  /* CP15ISB:
   * Instruction Synchronization Barrier operation */
  initialiseRegister(crb, CP15_ISB, 0);

  /* BPIALL:
   * invalidate entire branch predictor array, write-only */
  initialiseRegister(crb, CP15_BPIALL, 0);

  /* DCCMVAC:
   * clean data cache line by MVA to PoC, write-only */
  initialiseRegister(crb, CP15_DCCMVAC, 0);

  /* CP15_DCIMVAC:
   * invalidate data cache line (using MVA) */
  initialiseRegister(crb, CP15_DCIMVAC, 0);

  /* DCCSW:
   * clean data cache line by set/way tSo PoC, write-only */
  initialiseRegister(crb, CP15_DCCSW, 0);

  /* CP15DSB:
   * Data Synchronization Barrier operation */
  initialiseRegister(crb, CP15_DSB, 0);

  /* CP15DMB
   * Data Memory Barrier operation */
  initialiseRegister(crb, CP15_DMB, 0);

  /* DCCMVAU:
   * clean data cache line by MVA to PoU, write only */
  initialiseRegister(crb, CP15_DCCMVAU, 0);

  /* DCCIMVAC: clean and invalidate dCache by MVA to PoC, write-only */
  initialiseRegister(crb, CP15_DCCIMVAC, 0);

  /* DCCISW:
   * clean and invalidate data cache line by set/way, write-only */
  initialiseRegister(crb, CP15_DCCISW, 0);

  /* ITLBIALL: invalide instruction TLB (all), write-only */
  initialiseRegister(crb, CP15_ITLBIALL, 0);

  /* ITLBIMVA: invalide instruction TLB by MVA, write-only */
  initialiseRegister(crb, CP15_ITLBIMVA, 0);

  /* ITLBIASID: invalide instruction TLB by ASID match, write-only */
  initialiseRegister(crb, CP15_ITLBIASID, 0);

  /* DTLBIALL: invalide data TLB (all), write-only */
  initialiseRegister(crb, CP15_DTLBIALL, 0);

  /* DTLBIMVA: invalide data TLB by MVA, write-only */
  initialiseRegister(crb, CP15_DTLBIMVA, 0);

  /* DTLBIASID: invalide data TLB by ASID match, write-only */
  initialiseRegister(crb, CP15_DTLBIASID, 0);

  /* TLBIALL: invalide unified TLB, write-only */
  initialiseRegister(crb, CP15_TLBIALL, 0);

  /* TLBIMVA: invalidate unified TLB by MVA, write-only */
  initialiseRegister(crb, CP15_TLBIMVA, 0);

  /* PRRR:
   * primary region remap register: does lots of weird things with
   * C, B, and TEX attributes of memory regions, init to 0x98AA4 */
  initialiseRegister(crb, CP15_PRRR, 0x00098AA4);

  /* NMRR:
   * normal memory remap register: additional mapping controls for memory regions
   * that are mapped as Normal memory by their entry in the PRRR
   * initialize to 44E048E0 */
  initialiseRegister(crb, CP15_NMRR, 0x44E048E0);

  /* VBAR:
   * vector base address register */
  initialiseRegister(crb, CP15_VBAR, 0);

  /* FCSEIDR:
   * fast context switch extension process ID register
   * initialize to 0 */
  initialiseRegister(crb, CP15_FCSEIDR, 0);

  /* CONTEXTID:
   * context ID register
   * initialize to 0 */
  initialiseRegister(crb, CP15_CONTEXTID, 0);

  /* TPIDRURW:
   * software thread ID register, user mode read-write
   * initialize to 0 */
  initialiseRegister(crb, CP15_TPIDRURW, 0);

  /* TPIDRURO:
   * software thread ID register, user mode read-only
   * initialize to 0 */
  initialiseRegister(crb, CP15_TPIDRURO, 0);

  /* TPIDRPRW:
   * software thread ID register, privileged read/write only
   * initialize to 0 */
  initialiseRegister(crb, CP15_TPIDRPRW, 0);

  return crb;
}

static void initialiseRegister(CREG *crb, Coprocessor15Register reg, u32int value)
{
  crb[reg].value = value;
  crb[reg].valid = TRUE;
}

void setCregVal(GCONTXT *context, u32int registerIndex, u32int value)
{
  CREG *const registerBank = context->coprocRegBank;
  if (unlikely(!registerBank[registerIndex].valid))
  {
    // guest writing to a register that is not valid yet! investigate
    printf("setCregVal: op1=%#x CRn=%#x CRm=%#x op2=%#x value=%#.8x" EOL,
           CRB_INDEX_TO_OP1(registerIndex), CRB_INDEX_TO_CRN(registerIndex),
           CRB_INDEX_TO_CRM(registerIndex), CRB_INDEX_TO_OP2(registerIndex), value);
    DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
  }

  u32int oldVal = registerBank[registerIndex].value;
  registerBank[registerIndex].value = value;

  DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: op1=%#x CRn=%#x CRm=%#x op2=%#x value=%#.8x -> %#.8x" EOL,
        CRB_INDEX_TO_OP1(registerIndex), CRB_INDEX_TO_CRN(registerIndex),
        CRB_INDEX_TO_CRM(registerIndex), CRB_INDEX_TO_OP2(registerIndex), oldVal, value);

  switch (registerIndex)
  {
    case CP15_MIDR:
    case CP15_CTR:
    case CP15_MMFR0:
    case CP15_MMFR1:
    case CP15_CCSIDR:
    case CP15_CLIDR:
    {
      DIE_NOW(NULL, "read-only register");
    }
    case CP15_CSSELR:
    {
      // write to CSSELR: cache size select register.
      // need to update R/O CSSIDR value
      switch (value)
      {
        case 0b0000:
        {
          // lvl1 data cache
          registerBank[CP15_CCSIDR].value = 0xE007E01A;
          break;
        }
        case 0b0001:
        {
          // lvl1 instruction cache
          registerBank[CP15_CCSIDR].value = 0x2007E01A;
          break;
        }
        case 0b0010:
        {
          // lvl2 unified cache
          registerBank[CP15_CCSIDR].value = 0xF03FE03A;
          break;
        }
        default:
        {
          // no such cache exists on the beagleboard!
          printf("setCregVal: CSSELR = %x selects unimplemented cache" EOL, value);
          DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
        }
      } // switch (value) ends
      break;
    }
    case CP15_SCTRL:
    {
      // SCTRL: System control register
      if (((oldVal & SYS_CTRL_MMU_ENABLE) == 0) && ((value & SYS_CTRL_MMU_ENABLE) != 0))
      {
        DEBUG(INTERPRETER_ANY_COPROC, "CP15: SysCtrl - MMU enable." EOL);
        guestEnableMMU(context);
      }
      else if (((oldVal & SYS_CTRL_MMU_ENABLE)!=0) && ((value & SYS_CTRL_MMU_ENABLE)==0))
      {
        DEBUG(INTERPRETER_ANY_COPROC, "CP15: SysCtrl - MMU disable." EOL);
        guestDisableMMU(context);
      }

      //Interupt handler remap
      if (((oldVal & SYS_CTRL_HIGH_VECS) == 0) && ((value & SYS_CTRL_HIGH_VECS) != 0))
      {
        DEBUG(INTERPRETER_ANY_COPROC, "CP15: SysCtrl - high interrupt vector set." EOL);
        context->guestHighVectorSet = TRUE;
      }
      else if (((oldVal & SYS_CTRL_HIGH_VECS) != 0) && ((value & SYS_CTRL_HIGH_VECS) == 0))
      {
        DEBUG(INTERPRETER_ANY_COPROC, "CP15: SysCtrl - low interrupt vector set." EOL);
        context->guestHighVectorSet = FALSE;
      }

      if (((oldVal & SYS_CTRL_ALIGNMENT) == 0) && ((value & SYS_CTRL_ALIGNMENT) != 0))
      {
        DEBUG(INTERPRETER_ANY_COPROC, "CP15: sysctrl align check set." EOL);
        mmuToggleAlignCheck(TRUE);
      }
      else if (((oldVal & SYS_CTRL_ALIGNMENT) != 0) && ((value & SYS_CTRL_ALIGNMENT) == 0))
      {
        DEBUG(INTERPRETER_ANY_COPROC, "CP15: sysctrl align check unset." EOL);
        mmuToggleAlignCheck(FALSE);
      }

      ASSERT(!(value & SYS_CTRL_ACCESS_FLAG), ERROR_NOT_IMPLEMENTED);
      ASSERT(!(value & SYS_CTRL_HW_ACC_FLAG), ERROR_NOT_IMPLEMENTED);
      ASSERT(!(value & SYS_CTRL_VECT_INTERRUPT), ERROR_NOT_IMPLEMENTED);

      if (((oldVal & SYS_CTRL_TEX_REMAP) == 0) && ((value & SYS_CTRL_TEX_REMAP) != 0))
      {
        DEBUG(INTERPRETER_ANY_COPROC, "CP15: Warning: guest enabling TEX remap" EOL);
      }
      else if (((oldVal & SYS_CTRL_TEX_REMAP) != 0) && ((value & SYS_CTRL_TEX_REMAP) == 0))
      {
        DEBUG(INTERPRETER_ANY_COPROC, "CP15: Warning: guest disabling TEX remap" EOL);
      }

      break;
    }
    case CP15_ACTLR:
    {
      ACTLR old = { .value = oldVal };
      ACTLR new = { .value = value };
      ASSERT(old.bits.l1AliasChecksEnable == new.bits.l1AliasChecksEnable, ERROR_NOT_IMPLEMENTED)
      ASSERT(old.bits.l2Enable == new.bits.l2Enable, ERROR_NOT_IMPLEMENTED)
      ASSERT(old.bits.l1ParityDetectEnable == new.bits.l1ParityDetectEnable, ERROR_NOT_IMPLEMENTED)
      ASSERT(old.bits.axiSpeculativeAccessEnable == new.bits.axiSpeculativeAccessEnable, ERROR_NOT_IMPLEMENTED)
      ASSERT(old.bits.l1NEONEnable == new.bits.l1NEONEnable, ERROR_NOT_IMPLEMENTED)
      if (old.bits.invalidateBTBEnable != new.bits.invalidateBTBEnable)
      {
        if (new.bits.invalidateBTBEnable)
        {
          printf("Warning: guest enabling CP15 BTB invalidate operations");
        }
        else
        {
          printf("Warning: guest disabling CP15 BTB invalidate operations");
        }
        old.bits.invalidateBTBEnable = new.bits.invalidateBTBEnable;
        registerBank[CP15_ACTLR].value = old.value;
      }
      ASSERT(old.bits.branchSizeMispredictDisable == new.bits.branchSizeMispredictDisable, ERROR_NOT_IMPLEMENTED)
      ASSERT(old.bits.wfiNOP == new.bits.wfiNOP, ERROR_NOT_IMPLEMENTED)
      ASSERT(old.bits.pldNOP == new.bits.pldNOP, ERROR_NOT_IMPLEMENTED)
      ASSERT(old.bits.forceSingleIssue == new.bits.forceSingleIssue, ERROR_NOT_IMPLEMENTED)
      ASSERT(old.bits.forceLoadStoreSingleIssue == new.bits.forceLoadStoreSingleIssue, ERROR_NOT_IMPLEMENTED)
      ASSERT(old.bits.forceNEONSingleIssue == new.bits.forceNEONSingleIssue, ERROR_NOT_IMPLEMENTED)
      ASSERT(old.bits.forceMainClock == new.bits.forceMainClock, ERROR_NOT_IMPLEMENTED)
      ASSERT(old.bits.forceNEONClock == new.bits.forceNEONClock, ERROR_NOT_IMPLEMENTED)
      ASSERT(old.bits.forceETMClock == new.bits.forceETMClock, ERROR_NOT_IMPLEMENTED)
      ASSERT(old.bits.cp1415PipelineFlush == new.bits.cp1415PipelineFlush, ERROR_NOT_IMPLEMENTED)
      ASSERT(old.bits.cp1415WaitOnIdle == new.bits.cp1415WaitOnIdle, ERROR_NOT_IMPLEMENTED)
      ASSERT(old.bits.cp1415InstructionSerialization == new.bits.cp1415InstructionSerialization, ERROR_NOT_IMPLEMENTED)
      ASSERT(old.bits.clockStopRequestDisable == new.bits.clockStopRequestDisable, ERROR_NOT_IMPLEMENTED)
      ASSERT(old.bits.cacheMaintenancePipeline == new.bits.cacheMaintenancePipeline, ERROR_NOT_IMPLEMENTED)
      ASSERT(old.bits.l1HardwareResetDisable == new.bits.l1HardwareResetDisable, ERROR_NOT_IMPLEMENTED)
      ASSERT(old.bits.l2HardwareResetDisable == new.bits.l2HardwareResetDisable, ERROR_NOT_IMPLEMENTED)
    }
    case CP15_TTBR0:
    {
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: TTBR0 write %x" EOL, value);
      // must calculate: bits 31 to (14-N) give TTBR value. N is [0:2] of TTBCR!
      u32int N = registerBank[CP15_TTBCR].value & 0x7;
      u32int bottomBitNumber = 14 - N;
      u32int mask = ~((1 << bottomBitNumber)-1);
      guestSetPageTableBase(context, value & mask);
      break;
    }
    case CP15_TTBR1:
    {
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: WARN: TTBR1 write %x" EOL, value);
      break;
    }
    case CP15_TTBCR:
    {
      // TTBCR: translation table base control register
      // 0b000 - always use TTBR0. !0 - depends...
      ASSERT((value & 0x7) == 0, ERROR_NOT_IMPLEMENTED);
      break;
    }
    case CP15_DACR:
    {
      if (oldVal != value)
      {
        DEBUG(INTERPRETER_ANY_COPROC, "CP15: DACR change val %x old DACR %x" EOL, value, oldVal);
        DACR old = {.value = oldVal};
        DACR new = {.value = value};
        changeGuestDACR(context, old, new);
      }
      break;
    }
    case CP15_DFSR:
    {
      // DFSR: data fault status register
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: set DFSR to %x\n", value);
      break;
    }
    case CP15_IFSR:
    {
      // IFSR: instruction fault status register
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: set IFSR to %x\n", value);
      break;
    }
    case CP15_DFAR:
    {
      // DFAR: data fault address register
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: set DFAR to %x\n", value);
      break;
    }
    case CP15_IFAR:
    {
      // IFAR: instruction fault address register
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: set IFAR to %x\n", value);
      break;
    }
    case CP15_ICIALLU:
    {
      // ICIALLU: invalidate all instruction caches to PoU
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: invalidate all iCaches to PoU" EOL);
      mmuInvIcacheToPOU();
      break;
    }
    case CP15_ICIMVAU:
    {
      // ICIMVAU: invalidate instruction caches by MVA to PoU
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: invalidate iCaches by MVA to PoU: %x" EOL, value);
      mmuInvIcacheByMVAtoPOU(value);
      break;
    }
    case CP15_ISB:
    {
      // CP15DSB: Instruction Synchronization Barrier operation
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: Instruction Synchronization Barrier operation" EOL);
      mmuInstructionSync();
      break;
    }
    case CP15_BPIALL:
    {
      // BPIALL: invalidate entire branch predictor array
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: invalidate entire branch predictor array (no effect)" EOL);
      break;
    }
    case CP15_DCIMVAC:
    {
      // DCIMVAC: invalidate data cache line (using MVA)
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: invalidate data cache line (using MVA)" EOL);
      break;
    }
    case CP15_DCCMVAC:
    {
      // DCCMVAC: clean data cache line by MVA to PoC
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: clean Dcache line by MVA to PoC: %x" EOL, value);
      mmuCleanDcacheByMVAtoPOC(value);
      break;
    }
    case CP15_DCCSW:
    {
      // DCCSW: clean data cache line by set/way
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: clean Dcache line, set/way: %x" EOL, value);
      mmuCleanInvDCacheBySetWay(value);
      break;
    }
    case CP15_DSB:
    {
      // CP15DSB: Data Synchronization Barrier operation
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: Data Synchronization Barrier operation" EOL);
      mmuDataSyncBarrier();
      break;
    }
    case CP15_DMB:
    {
      // CP15DMB: Data Memory Barrier operation
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: Data Memory Barrier operation" EOL);
      mmuDataMemoryBarrier();
      break;
    }
    case CP15_DCCMVAU:
    {
      // DCCMVAU: clean data cache line by MVA to PoU
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: clean dCache line by MVA to PoU: %x" EOL, value);
      mmuCleanDCacheByMVAtoPOU(value);
      break;
    }
    case CP15_DCCIMVAC:
    {
      // DCCIMVAC: clean and invalidate dCache by MVA to PoC
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: clean and invalidate dCache by MVA to PoC: %x" EOL, value);
      mmuCleanInvDCacheByMVAtoPOC(value);
      break;
    }
    case CP15_DCCISW:
    {
      // DCCISW: clean and invalidate data cache line by set/way
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: clean and invalidate Dcache line, set/way: %x" EOL, value);
      mmuCleanInvDCacheBySetWay(value);
      break;
    }
    case CP15_ITLBIALL:
    {
      // ITLBIALL: invalide instruction TLB (all)
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: invalidate instruction TLB (all)" EOL);
      mmuInvalidateITLB();
      break;
    }
    case CP15_ITLBIMVA:
    {
      // ITLBIMVA: invalide instruction TLB by MVA
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: invalidate instruction TLB by MVA: %x" EOL, value);
      mmuInvalidateITLBbyMVA(value);
      break;
    }
    case CP15_ITLBIASID:
    {
      // ITLBIASID: invalide instruction TLB by ASID match
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: invalidate instruction TLB by ASID match: %x" EOL, value);
      mmuInvalidateITLBbyASID(value);
      break;
    }
    case CP15_DTLBIALL:
    {
      // DTLBIALL: invalide data TLB (all)
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: invalidate data TLB (all)" EOL);
      mmuInvalidateDTLB();
      break;
    }
    case CP15_DTLBIMVA:
    {
      // DTLBIMVA: invalidate dTLB entry by MVA
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: invalidate data TLB by MVA: %x" EOL, value);
      mmuInvalidateDTLBbyMVA(value);
      break;
    }
    case CP15_DTLBIASID:
    {
      // DTLBIASID: invalidate dTLB entry by MVA
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: invalidate data TLB by ASID match: %x" EOL, value);
      mmuInvalidateDTLBbyASID(value);
      break;
    }
    case CP15_TLBIALL:
    {
      // TLBIALL: invalide unified TLB, write-only
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: invalidate unified TLB (all)" EOL);
      mmuInvalidateUTLB();
      break;
    }
    case CP15_TLBIMVA:
    {
      // TLBIMVA: invalidate unified TLB by MVA, write-only
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: invalidate unified TLB by MVA" EOL);
      mmuInvalidateUTLBbyMVA(value);
      break;
    }
    case CP15_PRRR:
    {
      // PRRR: primary region remap register
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: WARN: PRRR value %x" EOL, value);
      __asm__ __volatile__("mcr p15, 0, %0, c10, c2, 0": :"r"(value));
      break;
    }
    case CP15_NMRR:
    {
      // NMRR: normal memory remap register
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: WARN: NMRR value %x" EOL, value);
      __asm__ __volatile__("mcr p15, 0, %0, c10, c2, 1": :"r"(value));
      break;
    }
    case CP15_VBAR:
    {
      // VBAR: vector base address register
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: VBAR=%x" EOL, value);
      break;
    }
    case CP15_FCSEIDR:
    {
      // FCSEIDR: fast context switch extension process ID register
      DIE_NOW(NULL, "setCregVal: writing FCSEIDR - investigate!");
      break;
    }
    case CP15_CONTEXTID:
    {
      // CONTEXTID: context ID register
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: WARN: CONTEXTID value %x" EOL, value);
      guestSetContextID(context, value);
      break;
    }
    case CP15_TPIDRURW:
    {
      // TPIDRURW: software thread ID register, user mode read-write
      if (value)
      {
        DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: WARN: TPIDRURW value %x\n", value);
      }
      break;
    }
    case CP15_TPIDRURO:
    {
      // TPIDRURO: software thread ID register, user mode read-only
      // writes are caught by the hypervisor - they are privileged operations
      // but reads are not! must propagate TPIDRURO value to real CP15.
      DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: WARN: TPIDRURO value %x" EOL, value);
      __asm__ __volatile__("mcr p15, 0, %0, c13, c0, 3": :"r"(value));
      break;
    }
    case CP15_TPIDRPRW:
    {
      // TPIDRPRW: software thread ID register, user mode no access
      if (value)
      {
        DEBUG(INTERPRETER_ANY_COPROC, "setCregVal: WARN: TPIDRPRW value %x\n", value);
      }
      break;
    }
    default:
    {
      printf("setCregVal: op1=%#x CRn=%#x CRm=%#x op2=%#x value=%#.8x not handled" EOL,
             CRB_INDEX_TO_OP1(registerIndex), CRB_INDEX_TO_CRN(registerIndex),
             CRB_INDEX_TO_CRM(registerIndex), CRB_INDEX_TO_OP2(registerIndex), value);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
  }
}

u32int getCregVal(GCONTXT* context, u32int registerIndex)
{
  const CREG* cr = &context->coprocRegBank[registerIndex];

  DEBUG(INTERPRETER_ANY_COPROC, "getCreg: op1=%#x CRn=%#x CRm=%#x op2=%#x value=%#.8x valid=%x" EOL,
        CRB_INDEX_TO_OP1(registerIndex), CRB_INDEX_TO_CRN(registerIndex),
        CRB_INDEX_TO_CRM(registerIndex), CRB_INDEX_TO_OP2(registerIndex),
        cr->value, cr->valid);

  if (cr->valid)
  {
    return cr->value;
  }
  else
  {
    printf("getCregVal: op1=%#x CRn=%#x CRm=%#x op2=%#x value=%#.8x invalid" EOL,
           CRB_INDEX_TO_OP1(registerIndex), CRB_INDEX_TO_CRN(registerIndex),
           CRB_INDEX_TO_CRM(registerIndex), CRB_INDEX_TO_OP2(registerIndex), cr->value);
    DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
  }
}
