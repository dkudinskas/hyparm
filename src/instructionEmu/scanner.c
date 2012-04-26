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

#include "instructionEmu/interpreter/internals.h"

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
static bool isPCSensitiveInstruction(u32int instruction);
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
    /* First word is a backpointer, second may be reserved */
    u32int *addressInBlockCopyCache = meta->code + (meta->reservedWord ? 2 : 1);
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
      blockType = meta->type;
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


void scanAndCopyArmBlock(GCONTXT *context, u32int *startAddress, u32int metaIndex)
{
  /*
   * Check if there is room on blockCopyCacheCurrAddres and if not make it
   */
  u32int *codeCacheStart = context->translationCache.codeCacheNextEntry;
  u32int *codeCacheCurrent = updateCodeCachePointer(&context->translationCache, codeCacheStart);
  /*
   * Install backpointer in C$
   * TODO: seriously, the full address? lol.
   */
  *(codeCacheCurrent++) = (u32int)(&context->translationCache.metaCache[metaIndex]);
  /*
   * Instructions follow, so next R15 is here.
   */
  context->R15 = (u32int)updateCodeCachePointer(&context->translationCache, codeCacheCurrent);
  /*
   * Scan and copy
   */
  u32int *guestCurrent = startAddress;
  u32int instruction = *guestCurrent;
  u32int blockCopyCacheSize  =0;
  bool reservedWord = FALSE;
  struct decodingTableEntry *decodedInstruction;
  for (; (decodedInstruction = decodeArmInstruction(instruction))->replace == IRC_SAFE; instruction = *++guestCurrent)
  {
    /*
     * Safe instruction; but does it use PC?
     */
    if (!isPCSensitiveInstruction(instruction) || (decodedInstruction->pcHandler == NULL))
    {
      codeCacheCurrent = updateCodeCachePointer(&context->translationCache, codeCacheCurrent);
      *(codeCacheCurrent++) = instruction;
      continue;
    }
    /*
     * PC used as source register; call InstructionPCHandler. We abuse endOfBlockInstr...
     */
    context->endOfBlockInstr = instruction;
    codeCacheCurrent = decodedInstruction->pcHandler(&context->translationCache, guestCurrent, codeCacheCurrent, codeCacheStart);
    if (((u32int)codeCacheCurrent & 0b1) == 0b1)
    {
      /* Last bit of returnAddress is used to indicate that a reserved word is necessary
       * -> we can assume 2 byte alignment (even in worst case scenario (thumb))*/
      if (reservedWord)
      {
        /* Place has already been made -> just restore blockCopyCacheCurrAddress */
        codeCacheCurrent=(u32int*)((u32int)codeCacheCurrent & ~1);/*Set last bit back to zero*/
      }
      else
      {
        /* Entry in blockCopyCache will have to look like:
         * |-------------------|
         * |  backpointer      |  = indicated by blockCopyCacheStartAddress
         * |  emptyWord        |  = resevedWord for storing backup registers
         * |      ...          |  = Here starts the translated block
         *
         * blockCopyCacheStartAddress**/
        u32int *emptyWordPointer;
        u32int *destEmptyWord = codeCacheStart + 1;
        u32int *tempWordPointer;
        codeCacheCurrent = (u32int *)((u32int)codeCacheCurrent & ~1);/*Set last bit back to zero*/
        /* destEmptyWord can be set incorrectly if blockCopyCacheStartAddress is blockCopyCacheEnd -4
         * Because then destEmptyWord will be blockCopyCacheEnd but blockCopyCacheEnd contains a static branch
         * to start of blockCopyCache*/
        if (destEmptyWord == context->translationCache.codeCacheLastEntry)
        {
          destEmptyWord = context->translationCache.codeCache; //Reserved word will be at start of blockCopyCache
        }

        /*Set place for the reserved word correct now it is right before the instructions for the last instruction*/
        /*set pointer to the empty word.*/
        emptyWordPointer = codeCacheCurrent - 6;
        if (emptyWordPointer < context->translationCache.codeCache)
        {
          /* If we are before the start of the Block Copy Cache than we need to go to the corresponding place near the end*/
          emptyWordPointer = context->translationCache.codeCacheLastEntry - (context->translationCache.codeCache - emptyWordPointer);
        }
        /* emptyWordPointer now points to the empty word*/

        DEBUG(SCANNER, "emptyWordPointer : %p" EOL, emptyWordPointer);
        DEBUG(SCANNER, "destEmptyWord : %p" EOL, destEmptyWord);

        while (emptyWordPointer != destEmptyWord)
        {
          /* As long as the empty word isn't at its place keep on moving instructions */
          tempWordPointer = emptyWordPointer - 1; /* previous word */
          if(tempWordPointer == (context->translationCache.codeCache - 1))
          {
            /* Be careful when exceeding start of blockCopyCache */
            tempWordPointer = context->translationCache.codeCacheLastEntry - 1;
          }
          *((u32int *)emptyWordPointer) = *((u32int *)tempWordPointer);
          emptyWordPointer=tempWordPointer;
        }
        *emptyWordPointer= 0;/*Clear it so it cannot be a cause for confusion while debugging*/
        reservedWord = 1;/*From now on there is a reserved word to save backups*/
        /* Indicate that a free word is available at start of blockCopyCache */
        codeCacheStart= (u32int*) ( ((u32int)codeCacheStart) |0b1);
      }
    }
  } /* for safe */
  /*
   * Critical instruction!
   */
  /*----------------Install HdlFunct----------------*/
  /*Non of the source registers is the ProgramCounter -> Just End Of Block
   *Finish block by installing SVC
   *Save end of block instruction and handler function pointer close to us... */
  context->endOfBlockInstr = instruction;
  context->hdlFunct = decodedInstruction->handler;
  context->PCOfLastInstruction = (u32int)guestCurrent;
  /* replace end of block instruction with hypercall of the appropriate code
   *Check if there is room on blockCopyCacheCurrAddress and if not make it */
  codeCacheCurrent = updateCodeCachePointer(&context->translationCache, codeCacheCurrent);
  *(codeCacheCurrent++) = INSTR_SWI | ((metaIndex + 1) << 8);

  DEBUG(SCANNER, "EOB %p instr %#.8x SWIcode %#.2x hdlrFuncPtr %p" EOL, guestCurrent,
      context->endOfBlockInstr, metaIndex, context->hdlFunct);
  /*
   * We have to determine the size of the BlockCopyCache & If necessary patch the code
   * Patching of code is necessary when block is split up and a reserved word is used!
   */
  if (codeCacheCurrent < codeCacheStart)
  {
    if (reservedWord)
    {
      DEBUG(SCANNER, "Reserved WORD" EOL);

      u32int* blockCopyLast = mergeCodeBlockAsNeeded(&context->translationCache, codeCacheStart, codeCacheCurrent);

      /*
       * Make sure that block is safed correctly in blockCopyCache
       */
      //UPDATE blockCopyCacheStartAddress
      if (blockCopyLast != codeCacheCurrent)
      {
        codeCacheCurrent = blockCopyLast;
        codeCacheStart = context->translationCache.codeCache;
        //Set blockCopyCacheSize
        blockCopyCacheSize = blockCopyLast - codeCacheStart;
        /* Indicate that a free word is available at start of blockCopyCache */
        codeCacheStart = (u32int*)(((u32int)codeCacheStart) |0b1);
        /* Block is merged and moved to start of blockCopyCache.  We have to execute instructions from there!
         * But first word is backpointer and 2nd word is reserved word*/
        context->R15 = (u32int)(context->translationCache.codeCache + 2);

        DEBUG(SCANNER, "Block Merged.  New size = %#.8x" EOL, blockCopyCacheSize);
      }
      else
      {
        /* No patching needs to be done just set blockCopyCacheSize */
        blockCopyCacheSize = (u32int)context->translationCache.codeCacheLastEntry - (((u32int)codeCacheStart) & 0xFFFFFFFE);
        blockCopyCacheSize += (u32int)(codeCacheCurrent - context->translationCache.codeCache);
        blockCopyCacheSize = blockCopyCacheSize >> 2; //we have casted pointers to u32int thus divide by 4 to get size in words
      }
    }//end of reserved word case
    else
    {
        blockCopyCacheSize = (u32int)context->translationCache.codeCacheLastEntry - (((u32int)codeCacheStart) & 0xFFFFFFFE);
        blockCopyCacheSize += (u32int)(codeCacheCurrent - context->translationCache.codeCache);
        blockCopyCacheSize = blockCopyCacheSize >> 2;//we have casted pointers to u32int thus divide by 4 to get size in words

        DEBUG(SCANNER, "Block exceeding end: blockCopyCacheSize=%#.8x" EOL, blockCopyCacheSize);
    }
  }
  else
  {
    blockCopyCacheSize = codeCacheCurrent - (u32int *)(((u32int)codeCacheStart) & 0xFFFFFFFE);
  }
  /* This check is probably not necessary in production code but this can catch errors early on */
  if (blockCopyCacheSize > 0xFFFF)
  {
    printf("blockCache: ADD[%08x] start@%08x end@%08x hdlPtr %08x eobInstr %08x "
      "blockCopyCacheSize %08x blockCopyCache@%08x\n", metaIndex, (u32int)startAddress,
      (u32int)guestCurrent, (u32int)context->hdlFunct, context->endOfBlockInstr,
      blockCopyCacheSize, (u32int)codeCacheStart);
  }
  /* add the block we just scanned to block cache */
  addMetaCacheEntry(&context->translationCache, metaIndex, startAddress, guestCurrent,
    context->endOfBlockInstr, context->hdlFunct, blockCopyCacheSize,
    codeCacheStart);
  codeCacheStart = (u32int *)(((u32int)codeCacheStart) & 0xFFFFFFFE);
  guestWriteProtect((u32int)startAddress, (u32int)guestCurrent);
  context->translationCache.codeCacheNextEntry = codeCacheCurrent;

  DEBUG(SCANNER, "Block added with size of %#.8x words" EOL, codeCacheCurrent - codeCacheStart);
}


/* allSrcRegNonPC will return true if all source registers of an instruction are zero  */
static bool isPCSensitiveInstruction(u32int instruction)
{
  /*Source registers correspond with the bits [0..3],[8..11] or [16..19],  STMDB and PUSH may not write PC to mem :
   * STM   1000|10?0
   * STMDA 1000|00?0
   * STMDB 1001|00?0
   * STMIB 1001|10?0
         * --------
         * 100?|?0?0
   * mask    E |  5
   * value   8 |  0
   * In registerlist only the bit of PC has to be checked.  And the source register for memory location is not important
   */
  return (instruction & 0xF0000) == 0xF0000 || (instruction & 0xF00) == 0xF00
         || (instruction & 0xF) == 0xF || (instruction & 0x0E508000) == 0x08008000;
}

#endif // CONFIG_BLOCK_COPY
