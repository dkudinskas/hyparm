#include "common/debug.h"
#include "common/stddef.h"

#include "cpuArch/constants.h"

#include "exceptions/exceptionHandlers.h"

#include "guestManager/blockCache.h"

#ifdef CONFIG_THUMB2
#include "guestManager/guestExceptions.h"
#endif

#include "instructionEmu/decoder.h"
#include "instructionEmu/scanner.h"

#include "memoryManager/mmu.h"
#include "memoryManager/pageTable.h"


#define INSTR_SWI            0xEF000000U
#define INSTR_SWI_THUMB      0x0000DF00U
#define INSTR_NOP_THUMB      0x0000BF00U
#define INSTR_SWI_THUMB_MIX  ((INSTR_SWI_THUMB << 16) | INSTR_NOP_THUMB)


static void scanArmBlock(GCONTXT *context, u32int *start, u32int cacheIndex);

#ifdef CONFIG_THUMB2
static void scanThumbBlock(GCONTXT *context, u16int *start, u32int cacheIndex);
#endif


#ifdef CONFIG_SCANNER_COUNT_BLOCKS
u64int scanBlockCounter;
static inline u64int getScanBlockCounter(void);
static inline void incrementScanBlockCounter(void);
static inline u64int getScanBlockCounter()
{
  return scanBlockCounter;
}
static inline void incrementScanBlockCounter()
{
  scanBlockCounter++;
}
void resetScanBlockCounter()
{
  scanBlockCounter = 0;
}
#else
#define getScanBlockCounter()        (0ULL)
#define incrementScanBlockCounter()
#endif /* CONFIG_SCANNER_COUNT_BLOCKS */


#ifdef CONFIG_SCANNER_EXTRA_CHECKS
static u8int scanBlockCallSource;
static inline u8int getScanBlockCallSource(void);
static inline u8int getScanBlockCallSource()
{
  return scanBlockCallSource;
}
void setScanBlockCallSource(u8int source)
{
  scanBlockCallSource = source;
}
#else
#define getScanBlockCallSource()  ((u8int)(-1))
#endif /* CONFIG_SCANNER_EXTRA_CHECKS */


#ifdef CONFIG_DEBUG_SCANNER_MARK_INTERVAL
#define MARK_MASK  ((1 << CONFIG_DEBUG_SCANNER_MARK_INTERVAL) - 1)
#else
#define MARK_MASK  (0U)
#endif /* CONFIG_DEBUG_SCANNER_MARK_INTERVAL */

void scanBlock(GCONTXT *context, u32int startAddress)
{
  /*
   * WARNING: startAddress is not checked! Data aborts may follow and hide bugs elsewhere.
   */

#ifdef CONFIG_SCANNER_COUNT_BLOCKS
  incrementScanBlockCounter();
#endif

#ifdef CONFIG_SCANNER_EXTRA_CHECKS
  if (getScanBlockCallSource() == SCANNER_CALL_SOURCE_NOT_SET)
  {
    DEBUG(SCANNER, "scanBlock: gc = %p, blkStartAddr = %#.8x" EOL, context, startAddress);
    DIE_NOW(context, "scanBlock() called from unknown source");
  }
  if (startAddress == 0)
  {
    DEBUG(SCANNER, "scanBlock: gc = %p, blkStartAddr = %.8x" EOL, context, startAddress);
    DEBUG(SCANNER, "scanBlock: called from source %u" EOL, (u32int)scanBlockCallSource);
    if (getScanBlockCounter())
    {
      DEBUG(SCANNER, "scanBlock: scanned block count is %#Lx" EOL, getScanBlockCounter());
    }
    DIE_NOW(context, "scanBlock() called with NULL pointer");
  }
#endif /* CONFIG_SCANNER_EXTRA_CHECKS */

#ifdef CONFIG_SCANNER_COUNT_BLOCKS
  if ((getScanBlockCounter() & MARK_MASK) == 1)
  {
    DEBUG(SCANNER_MARK, "scanBlock: #B = %#.16Lx; #DABT = %#.16Lx; #IRQ = %#.16Lx; startAddress = "
        "%#.8x" EOL, getScanBlockCounter(), getDataAbortCounter(), getIrqCounter(), startAddress);
  }
#endif

  u32int cacheIndex = (startAddress >> 2) & (BLOCK_CACHE_SIZE-1);
  bool cached = ((context->blockCache[cacheIndex].type != BCENTRY_TYPE_INVALID)
              && (context->blockCache[cacheIndex].startAddress == startAddress));

  DEBUG(SCANNER_BLOCK_TRACE, "scanBlock: @%.8x, source = %#x, count = %#Lx; %s" EOL, startAddress,
      getScanBlockCallSource(), getScanBlockCounter(), (cached ? "HIT" : "MISS"));

  setScanBlockCallSource(SCANNER_CALL_SOURCE_NOT_SET);

  if (cached)
  {
    BCENTRY *bcEntry = getBlockCacheEntry(context->blockCache, cacheIndex);
    context->hdlFunct = bcEntry->hdlFunct;
    context->endOfBlockInstr = bcEntry->hyperedInstruction;
    return;
  }

#ifdef CONFIG_THUMB2
  if (context->CPSR & PSR_T_BIT)
  {
    scanThumbBlock(context, (void *)startAddress, cacheIndex);
  }
  else
#endif
  {
    scanArmBlock(context, (u32int *)startAddress, cacheIndex);
  }
}

static void scanArmBlock(GCONTXT *context, u32int *start, u32int cacheIndex)
{
  u32int *end;
  armInstruction *instr;
  u32int instruction;
  /*
   * Find the next sensitive instruction
   */
  end = start;

  while ((instr = decodeArmInstruction(*end))->replace == 0x0)
  {
    end++;
  }
  instruction = *end;

  if ((instruction & INSTR_SWI) == INSTR_SWI)
  {
    u32int svcCode = instruction & 0x00FFFFFF;
    if (svcCode > 0xFF)
    {
      /*
       * NIELS: the following code looks fishy to say the least ?!
       */
      // we hit a SWI that we placed ourselves as EOB. retrieve the real EOB...
      u32int svcCacheIndex = (svcCode >> 8) - 1;
      if (svcCacheIndex >= BLOCK_CACHE_SIZE)
      {
        printf("scanArmBlock: instruction %#.8x @ %p", instruction, end);
        DIE_NOW(context, "scanArmBlock: block cache index in SWI out of range.");
      }
      DEBUG(SCANNER_EXTRA, "scanArmBlock: EOB instruction is SWI @ %p code %#x" EOL, end, svcCacheIndex);
      BCENTRY * bcEntry = getBlockCacheEntry(context->blockCache, svcCacheIndex);
      // retrieve end of block instruction and handler function pointer
      context->endOfBlockInstr = bcEntry->hyperedInstruction;
      context->hdlFunct = bcEntry->hdlFunct;
    }
    else //Handle guest SVC
    {
      context->endOfBlockInstr = instruction;
      context->hdlFunct = instr->handler;
    }
  }
  /* If the instruction is not a SWI placed by the hypervisor OR
   * it is a non-SWI instruction, then proceed as normal
   */
  else
  {
    // save end of block instruction and handler function pointer close to us...
    context->endOfBlockInstr = instruction;
    context->hdlFunct = instr->handler;
    // Thumb compatibility
    // replace end of block instruction with hypercall of the appropriate code
    *end = INSTR_SWI | ((cacheIndex + 1) << 8);
    // if guest instruction stream is mapped with caching enabled, must maintain
    // i and d cache coherency
    // iCacheFlushByMVA((u32int)currAddress);
  }

  DEBUG(SCANNER_EXTRA, "scanArmBlock: EOB %#.8x @ %p SVC code %#x hdlrFuncPtr %p" EOL,
      context->endOfBlockInstr, end, ((cacheIndex + 1) << 8), context->hdlFunct);

  addToBlockCache(context->blockCache, cacheIndex, (u32int) start, (u32int)end,
      context->endOfBlockInstr, BCENTRY_TYPE_ARM, context->hdlFunct);

  /* To ensure that subsequent fetches from eobAddress get a hypercall
   * rather than the old cached copy...
   * 1. clean data cache entry by address
   * DCCMVAU, Clean data cache line by MVA to PoU: c7, 0, c11, 1
   * 2. invalidate instruction cache entry by address.
   * ICIMVAU, Invalidate instruction caches by MVA to PoU: c7, 0, c5, 1
   */
  mmuInvIcacheByMVAtoPOU((u32int)end);
  mmuCleanDcacheByMVAtoPOC((u32int)end);
  guestWriteProtect((u32int)start, (u32int)end);
}


#ifdef CONFIG_THUMB2
static void scanThumbBlock(GCONTXT *context, u16int *start, u32int cacheIndex)
{
  u16int *end;
  instructionHandler handler;
  u32int instruction;
  u32int blockType = BCENTRY_TYPE_THUMB;
  u32int endIs16Bit;

  u16int *currtmpAddress = start;   //backup pointer  ?? seems to be start address of last instruction

  end = start;
  while (TRUE)
  {
    instruction = *end;
    currtmpAddress = end;

    switch (instruction & THUMB32)
    {
      case THUMB32_1:
      case THUMB32_2:
      case THUMB32_3:
        instruction = (instruction << 16) | *++end;
        break;
    }

    if ((handler = decodeThumbInstruction(instruction)) != NULL)
    {
      break;
    }

    end++;
  }

  if (((instruction & INSTR_SWI_THUMB_MIX) == INSTR_SWI_THUMB_MIX) || ( ((instruction & 0xFFFF) >= 0xDF00) && ((instruction & 0xFFFF) <= 0xDFFF) ) ) // FIX ME -> This doesn't look right
  {
    u32int svcCode = (instruction & 0xFF); // NOP|SVC -> Keep the last 8 bits
    if (svcCode > 0)
    {
      // we hit a SWI that we placed ourselves as EOB. retrieve the real EOB...
      u32int cacheIndex = svcCode - 1;
      if (cacheIndex >= BLOCK_CACHE_SIZE)
      {
        printf("scanThumbBlock: instruction %#.8x @ %p", instruction, end);
        DIE_NOW(context, "scanThumbBlock: block cache index in SWI out of range");
      }
      DEBUG(SCANNER_EXTRA, "scanThumbBlock: EOB instruction is SWI @ %p code %#x" EOL, end, cacheIndex);
      BCENTRY * bcEntry = getBlockCacheEntry(context->blockCache, cacheIndex);
      // retrieve end of block instruction and handler function pointer
      context->endOfBlockInstr = bcEntry->hyperedInstruction;
      blockType = bcEntry->type;
      endIs16Bit = blockType == BCENTRY_TYPE_THUMB && !txxIsThumb32(bcEntry->hyperedInstruction);
      context->hdlFunct = bcEntry->hdlFunct;
    }
    else
    {
      context->endOfBlockInstr = instruction;
      endIs16Bit = TRUE;
      context->hdlFunct = handler;
    }

  }
  /* If the instruction is not a SWI placed by the hypervisor OR
   * it is a non-SWI instruction, then proceed as normal
   */
  else
  {
    /* Replace policy:
     * CurrAddress can point to any of these
     * 1) 32-bit word where lowest halfword is a single Thumb 16-bit instruction
     * 2) 32-bit word where lowest halfword is the second halfword of a Thumb 32-bit instruction
     * . In this case, the high halfword instruction is located in CurrAddres-2bytes
     * 3) 32-bit word where lowest halfword is the high halfword of a Thumb-32 instruction. In
     * this case, the remaining halfword is located in CurrAddress+2 bytes
     * To identify what kind of instruction this is, each 16bit portion has to be checked for
     * Thumb-32 compatible encoding.
     */
    end = currtmpAddress; // restore starting pointer and do what we did before
    instruction = * end;
    switch(instruction & THUMB32)
    {
      case THUMB32_1:
      case THUMB32_2:
      case THUMB32_3:
      {
        instruction = *end;
        end ++;
        instruction = (instruction<<16)|*end;
        context->endOfBlockInstr = instruction;
        endIs16Bit = FALSE;
        end --;
        *end = INSTR_NOP_THUMB;
        end ++;
        *end = INSTR_SWI_THUMB|((cacheIndex+1) & 0xFF);
        break;
      }
      default:
      {
        instruction = *end;
        context->endOfBlockInstr = instruction;
        endIs16Bit = TRUE;
        *end = INSTR_SWI_THUMB | ((cacheIndex+1) & 0xFF);
        break;
      }
    }
    DEBUG(SCANNER_EXTRA, "scanThumbBlock: svc on %#.8x" EOL, (u32int)end);

    context->hdlFunct = handler;
  }

  DEBUG(SCANNER_EXTRA, "scanThumbBlock: EOB %#.8x @ %p SVC code %#x hdlrFuncPtr %p" EOL,
      context->endOfBlockInstr, end, ((cacheIndex + 1) << 8), context->hdlFunct);

  addToBlockCache(context->blockCache, cacheIndex, (u32int)start, (u32int)end,
      context->endOfBlockInstr, blockType, context->hdlFunct);

  /* To ensure that subsequent fetches from eobAddress get a hypercall
   * rather than the old cached copy...
   * 1. clean data cache entry by address
   * DCCMVAU, Clean data cache line by MVA to PoU: c7, 0, c11, 1
   * 2. invalidate instruction cache entry by address.
   * ICIMVAU, Invalidate instruction caches by MVA to PoU: c7, 0, c5, 1
   */
  if (endIs16Bit)
  {
    asm("mcr p15, 0, %0, c7, c11, 1"
        :
        :"r"(end)
        :"memory"
    );
    asm("mcr p15, 0, %0, c7, c5, 1"
        :
        :"r"(end)
        :"memory"
    );
  }
  else
  {
    asm("mcr p15, 0, %0, c7, c11, 1"
        :
        :"r"(end)
        :"memory"
    );
    asm("mcr p15, 0, %0, c7, c5, 1"
        :
        :"r"(end)
        :"memory"
    );
    end--;
    asm("mcr p15, 0, %0, c7, c11, 1"
        :
        :"r"(end)
        :"memory"
    );
    asm("mcr p15, 0, %0, c7, c5, 1"
        :
        :"r"(end)
        :"memory"
    );
  }

  /*
   * FIXME: is end still correct for the case endIs16Bit == FALSE?
   */
  guestWriteProtect((u32int)start, (u32int)end);
}

#endif // THUMB2
