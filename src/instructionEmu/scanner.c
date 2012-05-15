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
#include "instructionEmu/translationInfo.h"

#include "instructionEmu/interpreter/internals.h"

#include "memoryManager/mmu.h"
#include "memoryManager/pageTable.h"


#define INSTR_SWI            0xEF000000U
#define INSTR_SWI_THUMB      0x0000DF00U
#define INSTR_NOP_THUMB      0x0000BF00U
#define INSTR_SWI_THUMB_MASK 0x0000FF00U
#define INSTR_SWI_THUMB_MIX  ((INSTR_SWI_THUMB << 16) | INSTR_NOP_THUMB)


static void scanArmBlock(GCONTXT *context, u32int *start, u32int metaIndex);

#ifdef CONFIG_THUMB2
static void scanThumbBlock(GCONTXT *context, u16int *start, u32int metaIndex);
#endif

#ifdef CONFIG_BLOCK_COPY

static void scanAndCopyArmBlock(GCONTXT *context, u32int *start, u32int metaIndex);
static bool armIsPCInsensitiveInstruction(u32int instruction);
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
  incrementScanBlockCounter();

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

  if ((getScanBlockCounter() & MARK_MASK) == 1)
  {
    DEBUG(SCANNER_MARK, "scanBlock: #B = %#.16Lx; #DABT = %#.16Lx; #IRQ = %#.16Lx; startAddress = "
        "%#.8x" EOL, getScanBlockCounter(), getDataAbortCounter(), getIrqCounter(), startAddress);
  }

  u32int cacheIndex = getMetaCacheIndex(startAddress);
  MetaCacheEntry *meta = getMetaCacheEntry(&context->translationCache, cacheIndex, startAddress);

  DEBUG(SCANNER_BLOCK_TRACE, "scanBlock: @%.8x, source = %#x, count = %#Lx; %s" EOL, startAddress,
    getScanBlockCallSource(), getScanBlockCounter(), (meta != NULL ? "HIT" : "MISS"));

  setScanBlockCallSource(SCANNER_CALL_SOURCE_NOT_SET);

  if (meta != NULL)
  {
    context->hdlFunct = meta->hdlFunct;
    context->endOfBlockInstr = meta->hyperedInstruction;
#ifdef CONFIG_BLOCK_COPY
    /* First word is a backpointer */
    u32int *addressInBlockCopyCache = &meta->code->codeStart;
    // The programcounter of the code that is executing should be set to the code in the blockCache
    if (addressInBlockCopyCache >= context->translationCache.codeCacheLastEntry)
    {
      /* blockCopyCacheAddresses will be used in a  cyclic manner
      -> if end of blockCopyCache is passed blockCopyCacheCurrAddress must be updated */
      addressInBlockCopyCache = addressInBlockCopyCache - (TRANSLATION_CACHE_CODE_SIZE_B - 1);
    }
    context->R15 = (u32int)addressInBlockCopyCache;
    //But also the PC of the last instruction of the block should be set
    context->PCOfLastInstruction = (u32int)meta->endAddress;
#endif
    return;
  }

#ifdef CONFIG_THUMB2
  if (context->CPSR & PSR_T_BIT)
  {
    scanThumbBlock(context, (void *)startAddress, cacheIndex);
  }
  else
#endif /* CONFIG_THUMB2 */
  {
#ifdef CONFIG_BLOCK_COPY
    scanAndCopyArmBlock(context, (u32int *)startAddress, cacheIndex);
#else
    scanArmBlock(context, (u32int *)startAddress, cacheIndex);
#endif
  }
}

#ifndef CONFIG_BLOCK_COPY

static void scanArmBlock(GCONTXT *context, u32int *start, u32int cacheIndex)
{
  DEBUG(SCANNER, "scanArmBlock @ %p (R15 = %#.8x)" EOL, start, context->R15);

  u32int *end;
  InstructionHandler handler;
  u32int instruction;
  instructionReplaceCode replaceCode;
  /*
   * Find the next sensitive instruction
   */
  end = start;

  while ((replaceCode = decodeArmInstruction(*end, &handler)) == IRC_SAFE)
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
      if (svcCacheIndex >= TRANSLATION_CACHE_META_SIZE_N)
      {
        printf("scanArmBlock: instruction %#.8x @ %p", instruction, end);
        DIE_NOW(context, "scanArmBlock: block cache index in SWI out of range.");
      }
      DEBUG(SCANNER_EXTRA, "scanArmBlock: EOB instruction is SWI @ %p code %#x" EOL, end, svcCacheIndex);
      MetaCacheEntry *meta = &context->translationCache.metaCache[svcCacheIndex];
      // retrieve end of block instruction and handler function pointer
      context->endOfBlockInstr = meta->hyperedInstruction;
      context->hdlFunct = meta->hdlFunct;
    }
    else //Handle guest SVC
    {
      context->endOfBlockInstr = instruction;
      context->hdlFunct = handler;
    }
  }
  /* If the instruction is not a SWI placed by the hypervisor OR
   * it is a non-SWI instruction, then proceed as normal
   */
  else
  {
    // save end of block instruction and handler function pointer close to us...
    context->endOfBlockInstr = instruction;
    context->hdlFunct = handler;
    // Thumb compatibility
    // replace end of block instruction with hypercall of the appropriate code
    *end = INSTR_SWI | ((cacheIndex + 1) << 8);
    // if guest instruction stream is mapped with caching enabled, must maintain
    // i and d cache coherency
    // iCacheFlushByMVA((u32int)currAddress);
  }

  DEBUG(SCANNER_EXTRA, "scanArmBlock: EOB %#.8x @ %p SVC code %#x hdlrFuncPtr %p" EOL,
      context->endOfBlockInstr, end, ((cacheIndex + 1) << 8), context->hdlFunct);

  addMetaCacheEntry(&context->translationCache, cacheIndex, start, end,
      context->endOfBlockInstr, MCE_TYPE_ARM, context->hdlFunct);

  /* To ensure that subsequent fetches from eobAddress get a hypercall
   * rather than the old cached copy...
   * 1. clean data cache entry by address
   * DCCMVAU, Clean data cache line by MVA to PoU: c7, 0, c11, 1
   * 2. invalidate instruction cache entry by address.
   * ICIMVAU, Invalidate instruction caches by MVA to PoU: c7, 0, c5, 1
   */
  mmuInvIcacheByMVAtoPOU((u32int)end);
  mmuCleanDCacheByMVAtoPOU((u32int)end);
  guestWriteProtect((u32int)start, (u32int)end);
}

#ifdef CONFIG_THUMB2
static void scanThumbBlock(GCONTXT *context, u16int *start, u32int cacheIndex)
{
  DEBUG(SCANNER, "scanThumbBlock @ %#.8x" EOL, context->R15);

  u16int *end;
  InstructionHandler handler;
  instructionReplaceCode replaceCode;
  u32int instruction;
  u32int blockType = MCE_TYPE_THUMB;
  u32int endIs16Bit;

  u16int *currtmpAddress = start;   //backup pointer  ?? seems to be start address of last instruction

  // Get current ITSTATE from CPSR
  u32int itState = ((context->CPSR & PSR_ITSTATE_7_2) >> 8) | ((context->CPSR & PSR_ITSTATE_1_0) >> 25);

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

    if ((replaceCode = decodeThumbInstruction(instruction, &handler)) != IRC_SAFE)
    {
      if (itState != 0)
      {
        // When sensitive instruction is found in IT block check whether it should be replaced
        if (evaluateConditionCode(context, (itState & 0xF0) >> 4))
        {
          break;
        }
      }
      else
      {
        break;
      }
    }

    end++;

    // ITAdvance()
    if (itState != 0)
    {
      if ((itState & 0x7) == 0)
      {
        itState = 0;
      }
      else
      {
        itState = (itState & 0xE0) | ((itState << 1) & 0x1F);
      }
    }
  }

  if (!txxIsThumb32(instruction) && ((instruction & INSTR_SWI_THUMB_MASK) == INSTR_SWI_THUMB))
  {
    u32int svcCode = instruction & 0xFF;
    if (svcCode > 0)
    {
      // we hit a SVC that we placed ourselves as EOB. retrieve the real EOB...
      u32int svcCacheIndex = svcCode - 1;
      if (svcCacheIndex >= BLOCK_CACHE_SIZE)
      {
        printf("scanThumbBlock: instruction %#.8x @ %p", instruction, end);
        DIE_NOW(context, "scanThumbBlock: block cache index in SVC out of range");
      }
      DEBUG(SCANNER_EXTRA, "scanThumbBlock: EOB instruction is SVC @ %p code %#x" EOL, end, svcCacheIndex);
      MetaCacheEntry *meta = context->translationCache.metaCache[svcCacheIndex];
      // retrieve end of block instruction and handler function pointer
      context->endOfBlockInstr = meta->hyperedInstruction;
      blockType = meta->type;  /*----------------Install HdlFunct----------------*/
      *Save end of block instruction and handler function pointer close to us... */

      endIs16Bit = blockType == MCE_TYPE_THUMB && !txxIsThumb32(meta->hyperedInstruction);
      context->hdlFunct = meta->hdlFunct;
    }
    else // Guest SVC
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
    if(txxIsThumb32(instruction))
    {
      context->endOfBlockInstr = instruction;
      endIs16Bit = FALSE;
      // Replace instruction with SVC and NOP, both 16 bit instructions
      *end = INSTR_SWI_THUMB | ((cacheIndex+1) & 0xFF);
      end++;
      *end = INSTR_NOP_THUMB;
      end--;
    }
    else
    {
      context->endOfBlockInstr = instruction;
      endIs16Bit = TRUE;
      *end = INSTR_SWI_THUMB | ((cacheIndex+1) & 0xFF);
    }
    DEBUG(SCANNER_EXTRA, "scanThumbBlock: SVC on %#.8x" EOL, (u32int)end);

    context->hdlFunct = handler;
  }

  DEBUG(SCANNER_EXTRA, "scanThumbBlock: EOB %#.8x @ %p SVC code %#x hdlrFuncPtr %p" EOL,
      context->endOfBlockInstr, end, ((cacheIndex + 1) << 8), context->hdlFunct);

  addMetaCacheEntry(context->translationCache, cacheIndex, (u32int)start, (u32int)end,
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
    __asm__ __volatile__("mcr p15, 0, %0, c7, c11, 1"
        :
        :"r"(end)
        :"memory"
    );
    __asm__ __volatile__("mcr p15, 0, %0, c7, c5, 1"
        :
        :"r"(end)
        :"memory"
    );
  }
  else
  {
    __asm__ __volatile__("mcr p15, 0, %0, c7, c11, 1"
        :
        :"r"(end)
        :"memory"
    );
    __asm__ __volatile__("mcr p15, 0, %0, c7, c5, 1"
        :
        :"r"(end)
        :"memory"
    );
    end++;
    __asm__ __volatile__("mcr p15, 0, %0, c7, c11, 1"
        :
        :"r"(end)
        :"memory"
    );
    __asm__ __volatile__("mcr p15, 0, %0, c7, c5, 1"
        :
        :"r"(end)
        :"memory"
    );
    end--;
  }

  guestWriteProtect((u32int)start, (u32int)end);
}

#endif /* CONFIG_THUMB2 */


#else /* CONFIG_BLOCK_COPY */


/*
 * TODO: caching is disabled on C$, so no cache maintenance operations are called from here (!)
 *
 * reserved words are painful here... we actually only need one spill location per C$
 * however...
 * for C$ size <= 4088 bytes we can use ONE spill location, at start of C$ (because of PC +8 offset)
 * for C$ size <= 4096 we can use ONE spill location at end of C$
 * for C$ size <= 8192 we use one at start and end
 * for any bigger C$ size ... embed in the middle
 * ....given that LDR literal can address any offset -4095..+4095 from the PC (in both arm &thumb2)
 * in thumb PC is always aligned to a 4 byte boundary when used in LDR
 * we will DABT on spill location store
 * to make sure no guest has an abs addr into the spill location we need to verify that
 * LR points to a hypervisor-placed spill store, if not its a store hardcoded into guest
 * this may require a bitmap
 */
void scanAndCopyArmBlock(GCONTXT *context, u32int *startAddress, u32int metaIndex)
{
  /*
   * Check if there is room in the C$ and if not make it
   */
  ARMTranslationInfo block = {
                               .metaEntry = {
                                              .startAddress = (u32int)startAddress,
                                              .type = MCE_TYPE_ARM,
                                              .code = context->translationCache.codeCacheNextEntry,
                                            },
                               .pcRemapBitmapShift = 0
                             };
  block.code = updateCodeCachePointer(&context->translationCache, (u32int *)block.metaEntry.code);
  /*
   * Install backpointer in C$
   */
  ((CodeCacheEntry *)block.code)->metaIndex = metaIndex;
  /*
   * Instructions follow, so next R15 is here.
   */
  context->R15 = (u32int)updateCodeCachePointer(&context->translationCache, ++block.code);
  /*
   * Scan guest code and copy to C$, translating instructions that use the PC on the fly
   */
  u32int *instruction = startAddress;
  struct decodingTableEntry *decodedInstruction;
  for (; (decodedInstruction = decodeArmInstruction(*instruction))->replace == IRC_SAFE; ++instruction)
  {
    if (armIsPCInsensitiveInstruction(*instruction) || decodedInstruction->pcHandler == NULL)
    {
      DEBUG(SCANNER, "instruction %#.8x @ %p possibly uses PC as source operand" EOL, *instruction, instruction);
      *(block.code++) = *instruction;
      block.metaEntry.pcRemapBitmap |= PC_REMAP_INCREMENT << block.pcRemapBitmapShift;
      block.pcRemapBitmapShift += PC_REMAP_BIT_COUNT;
    }
    else
    {
      decodedInstruction->pcHandler(&context->translationCache, &block, (u32int)instruction, *instruction);
    }
    block.code = updateCodeCachePointer(&context->translationCache, block.code);
  }
  ASSERT(block.pcRemapBitmapShift < (sizeof(block.metaEntry.pcRemapBitmap) * CHAR_BIT), "block too long");
  /*
   * Next instruction must be translated into hypercall.
   */
  *(block.code++) = INSTR_SWI | ((metaIndex + 1) << 8);
  context->endOfBlockInstr = *instruction;
  context->hdlFunct = decodedInstruction->handler;
  context->PCOfLastInstruction = (u32int)instruction;
  DEBUG(SCANNER, "EOB %p instr %#.8x SWIcode %#.2x hdlrFuncPtr %p" EOL, instruction,
      context->endOfBlockInstr, metaIndex, context->hdlFunct);
  /*
   * Cache metadata for the translated block. Code may wrap around C$ boundaries; take this into
   * account when calculating the size of the translated code.
   */
  if (block.code < (u32int *)block.metaEntry.code)
  {
    block.metaEntry.codeSize = (u32int)context->translationCache.codeCacheLastEntry
                             - (u32int)block.metaEntry.code + (u32int)block.code
                             - (u32int)context->translationCache.codeCache;
    DEBUG(SCANNER, "Block exceeding end: blockCopyCacheSize=%#.8x" EOL, block.metaEntry.codeSize);
  }
  else
  {
    block.metaEntry.codeSize = (u32int)block.code - (u32int)block.metaEntry.code;
  }
  block.metaEntry.endAddress = (u32int)instruction;
  block.metaEntry.hyperedInstruction = context->endOfBlockInstr,
  block.metaEntry.hdlFunct = context->hdlFunct;
  addMetaCacheEntry(&context->translationCache, metaIndex, &block.metaEntry);
  /*
   * Protect guest against self-modification.
   */
  guestWriteProtect((u32int)startAddress, (u32int)instruction);
}

/*
 * armIsPCInsensitiveInstruction returns TRUE if it is CERTAIN that an instruction does not depend
 * on the current PC value.
 */
static bool armIsPCInsensitiveInstruction(u32int instruction)
{
  /* STMDB and PUSH may not write PC to mem :
   * STM   1000|10?0
   * STMDA 1000|00?0
   * STMDB 1001|00?0
   * STMIB 1001|10?0
         * --------
         * 100?|?0?0
   * mask    E |  5
   * value   8 |  0
   */
  return ARM_EXTRACT_REGISTER(instruction, 16) != GPR_PC
         && ARM_EXTRACT_REGISTER(instruction, 12) != GPR_PC
         && ARM_EXTRACT_REGISTER(instruction, 0) != GPR_PC
         && (instruction & 0x0E508000) != 0x08008000;
}

#endif // CONFIG_BLOCK_COPY
