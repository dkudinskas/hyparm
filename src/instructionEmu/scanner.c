#include "common/debug.h"
#include "common/stddef.h"

#include "cpuArch/constants.h"

#include "guestManager/blockCache.h"

#ifdef CONFIG_THUMB2
#include "guestManager/guestExceptions.h"
#endif

#include "instructionEmu/decoder.h"
#include "instructionEmu/scanner.h"

#ifdef CONFIG_BLOCK_COPY
#include "instructionEmu/tableSearchBlockCopyDecoder.h"
#endif

#include "memoryManager/mmu.h"
#include "memoryManager/pageTable.h"


#define INSTR_SWI            0xEF000000U
#define INSTR_SWI_THUMB      0x0000DF00U
#define INSTR_NOP_THUMB      0x0000BF00U
#define INSTR_SWI_THUMB_MIX  ((INSTR_SWI_THUMB << 16) | INSTR_NOP_THUMB)


static inline u32int getHash(u32int key);

static void scanArmBlock(GCONTXT *context, u32int *start, u32int cacheIndex);

#ifdef CONFIG_THUMB2
static void scanThumbBlock(GCONTXT *context, u16int *start, u32int cacheIndex);
#endif

static void protectScannedBlock(void *start, void *end);

static void scanAndCopyArmBlock(GCONTXT *context, u32int *start, u32int cacheIndex);


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


// http://www.concentric.net/~Ttwang/tech/inthash.htm
// 32bit mix function
static inline u32int getHash(u32int key)
{
  key = ~key + (key << 15); // key = (key << 15) - key - 1;
  key = key ^ (key >> 12);
  key = key + (key << 2);
  key = key ^ (key >> 4);
  key = key * 2057; // key = (key + (key << 3)) + (key << 11);
  key = key ^ (key >> 16);
  return key >> 2;
}

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
      DEBUG(SCANNER_COUNT_BLOCKS, "scanBlock: scanned block count is %#Lx" EOL, getScanBlockCounter());
    }
    DIE_NOW(context, "scanBlock() called with NULL pointer");
  }
#endif /* CONFIG_SCANNER_EXTRA_CHECKS */

  u32int cacheIndex = (getHash(startAddress) & (BLOCK_CACHE_SIZE-1));// 0x1FF mask for 512 entry cache
  bool cached = checkBlockCache(context->blockCache, cacheIndex, startAddress);

  DEBUG(SCANNER_BLOCK_TRACE, "scanBlock: @%.8x, source = %#x, count = %#Lx; %s" EOL, startAddress,
    getScanBlockCallSource(), getScanBlockCounter(), (cached ? "HIT" : "MISS"));

  setScanBlockCallSource(SCANNER_CALL_SOURCE_NOT_SET);

  if (cached)
  {
    BCENTRY *bcEntry = getBlockCacheEntry(context->blockCache, cacheIndex);
    context->hdlFunct = bcEntry->hdlFunct;
    context->endOfBlockInstr = bcEntry->hyperedInstruction;
#ifdef CONFIG_BLOCK_COPY
    /* First word is a backpointer, second may be reserved */
    u32int *addressInBlockCopyCache = ((u32int *)bcEntry->blockCopyCacheAddress) + (bcEntry->reservedWord ? 2 : 1);
    // The programcounter of the code that is executing should be set to the code in the blockCache
    if ((u32int)addressInBlockCopyCache >= context->blockCopyCacheEnd)
    {
      /* blockCopyCacheAddresses will be used in a  cyclic manner
      -> if end of blockCopyCache is passed blockCopyCacheCurrAddress must be updated */
      addressInBlockCopyCache = addressInBlockCopyCache - (BLOCK_COPY_CACHE_SIZE - 1);
    }
    context->R15 = (u32int)addressInBlockCopyCache;
    //But also the PC of the last instruction of the block should be set
    context->PCOfLastInstruction = (u32int)bcEntry->endAddress;
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
  u32int *end;
  instructionHandler handler;
  u32int instruction;
  /*
   * Find the next sensitive instruction
   */
  end = start;
  while ((handler = decodeArmInstruction(*end)) == NULL)
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
      DEBUG(SCANNER, "scanArmBlock: EOB instruction is SWI @ %p code %#x" EOL, end, svcCacheIndex);
      BCENTRY * bcEntry = getBlockCacheEntry(context->blockCache, svcCacheIndex);
      // retrieve end of block instruction and handler function pointer
      context->endOfBlockInstr = bcEntry->hyperedInstruction;
      context->hdlFunct = bcEntry->hdlFunct;
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

  DEBUG(SCANNER, "scanArmBlock: EOB %#.8x @ %p SVC code %#x hdlrFuncPtr %p" EOL,
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
  protectScannedBlock(start, end);
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
      DEBUG(SCANNER, "scanThumbBlock: EOB instruction is SWI @ %p code %#x" EOL, end, cacheIndex);
      BCENTRY * bcEntry = getBlockCacheEntry(context->blockCache, cacheIndex);
      // retrieve end of block instruction and handler function pointer
      context->endOfBlockInstr = bcEntry->hyperedInstruction;
      blockType = bcEntry->type;
      endIs16Bit = blockType == BCENTRY_TYPE_THUMB && !TXX_IS_T32(bcEntry->hyperedInstruction);
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
    DEBUG(SCANNER, "scanThumbBlock: svc on %#.8x" EOL, (u32int)end);

    context->hdlFunct = handler;
  }

  DEBUG(SCANNER, "scanThumbBlock: EOB %#.8x @ %p SVC code %#x hdlrFuncPtr %p" EOL,
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
  protectScannedBlock(start, end);
}

#endif /* CONFIG_THUMB2 */


#else /* CONFIG_BLOCK_COPY */


void scanAndCopyArmBlock(GCONTXT *context, u32int *startAddress, u32int cacheIndex)
{
  /*
   * Check if there is room on blockCopyCacheCurrAddres and if not make it
   */
  u32int *blockCopyCacheStartAddress = ((u32int *)(context->blockCopyCacheLastUsedLine)) + 1;
  u32int *blockCopyCacheCurrAddress = blockCopyCacheStartAddress;
  blockCopyCacheCurrAddress = checkAndClearBlockCopyCacheAddress(blockCopyCacheCurrAddress,
    context->blockCache, (u32int *)context->blockCopyCache, (u32int *)context->blockCopyCacheEnd);
  /*
   * Install Backpointer in BlockCache:
   * pointer arithmetic gc->blcokCache+bcIndex  and save pointer as u32int
   */
  *(blockCopyCacheCurrAddress++) = (u32int)(context->blockCache + cacheIndex);
  DEBUG(SCANNER, "Backpointer installed at: %#.8x; Contents=%#.8x" EOL,
      (u32int)(blockCopyCacheCurrAddress - 1), *(blockCopyCacheCurrAddress - 1));
  /*
   * After the Backpointer the instructions will be installed.
   * Here the guestprocess should continue it's execution.
   */
  context->R15 = (u32int)checkAndClearBlockCopyCacheAddress(blockCopyCacheCurrAddress,
    context->blockCache, (u32int *)context->blockCopyCache, (u32int *)context->blockCopyCacheEnd);
  /*
   * Scan and copy
   */
  u32int *currAddress = startAddress;
  u32int instruction = *currAddress;
  u32int blockCopyCacheSize  =0;
  bool reservedWord = 0;
  while (1)
  {
    //binary & checks types -> do a cast of function pointer to u32int
    struct instruction32bit *decodedInstruction = decodeInstr(context, instruction);
    if (decodedInstruction->replaceCode)
    {
      /*
       * Critical instruction!
       */
      /*----------------Install HdlFunct----------------*/
      /*Non of the source registers is the ProgramCounter -> Just End Of Block
       *Finish block by installing SVC
       *Save end of block instruction and handler function pointer close to us... */
      context->endOfBlockInstr = instruction;
      context->hdlFunct = decodedInstruction->hdlFunct;
      context->PCOfLastInstruction = (u32int)currAddress;
      /* replace end of block instruction with hypercall of the appropriate code
       *Check if there is room on blockCopyCacheCurrAddress and if not make it */
      blockCopyCacheCurrAddress = checkAndClearBlockCopyCacheAddress(blockCopyCacheCurrAddress,
          context->blockCache, (u32int *)context->blockCopyCache,
          (u32int *)context->blockCopyCacheEnd);
      *(blockCopyCacheCurrAddress++) = INSTR_SWI | ((cacheIndex + 1) << 8);

      DEBUG(SCANNER, "EOB %p instr %#.8x SWIcode %#.2x hdlrFuncPtr %p" EOL, currAddress,
          context->endOfBlockInstr, cacheIndex, context->hdlFunct);

      /*
       * We have to determine the size of the BlockCopyCache & If necessary patch the code
       * Patching of code is necessary when block is split up and a reserved word is used!
       */
      if (blockCopyCacheCurrAddress < blockCopyCacheStartAddress)
      {
        if (reservedWord)
        {
          DEBUG(SCANNER, "Reserved WORD" EOL);

          u32int* blockCopyLast = checkAndMergeBlock((u32int*)context->blockCopyCache,blockCopyCacheCurrAddress, context->blockCache,blockCopyCacheStartAddress, (u32int*)context->blockCopyCacheEnd);

          /*
           * Make sure that block is safed correctly in blockCopyCache
           */
          //UPDATE blockCopyCacheStartAddress
          if (blockCopyLast != blockCopyCacheCurrAddress)
          {
            blockCopyCacheCurrAddress = blockCopyLast;
            blockCopyCacheStartAddress = (u32int *)context->blockCopyCache;
            //Set blockCopyCacheSize
            blockCopyCacheSize = blockCopyLast - blockCopyCacheStartAddress;
            /* Indicate that a free word is available at start of blockCopyCache */
            blockCopyCacheStartAddress = (u32int*)(((u32int)blockCopyCacheStartAddress) |0b1);
            /* Block is merged and moved to start of blockCopyCache.  We have to execute instructions from there!
             * But first word is backpointer and 2nd word is reserved word*/
            context->R15 = context->blockCopyCache + 8;

            DEBUG(SCANNER, "Block Merged.  New size = %#.8x" EOL, blockCopyCacheSize);
          }
          else
          {
            /* No patching needs to be done just set blockCopyCacheSize */
            blockCopyCacheSize = context->blockCopyCacheEnd - (((u32int)blockCopyCacheStartAddress) & 0xFFFFFFFE);
            blockCopyCacheSize += (u32int)blockCopyCacheCurrAddress - context->blockCopyCache;
            blockCopyCacheSize = blockCopyCacheSize >> 2; //we have casted pointers to u32int thus divide by 4 to get size in words
          }
        }//end of reserved word case
        else
        {
            blockCopyCacheSize = context->blockCopyCacheEnd - (((u32int)blockCopyCacheStartAddress) & 0xFFFFFFFE);
            blockCopyCacheSize += (u32int)blockCopyCacheCurrAddress - context->blockCopyCache;
            blockCopyCacheSize = blockCopyCacheSize >> 2;//we have casted pointers to u32int thus divide by 4 to get size in words

            DEBUG(SCANNER, "Block exceeding end: blockCopyCacheSize=%#.8x" EOL, blockCopyCacheSize);
        }
      }
      else
      {
        blockCopyCacheSize = blockCopyCacheCurrAddress - (u32int *)(((u32int)blockCopyCacheStartAddress) & 0xFFFFFFFE);
      }
      /* This check is probably not necessary in production code but this can catch errors early on */
      if (blockCopyCacheSize > 0xFFFF)
      {
        printf("blockCache: ADD[%08x] start@%08x end@%08x hdlPtr %08x eobInstr %08x "
          "blockCopyCacheSize %08x blockCopyCache@%08x\n", cacheIndex, (u32int)startAddress,
          (u32int)currAddress, (u32int)context->hdlFunct, context->endOfBlockInstr,
          blockCopyCacheSize, (u32int)blockCopyCacheStartAddress);
      }
      /* add the block we just scanned to block cache */
      addToBlockCache(context->blockCache, cacheIndex, (u32int)startAddress, (u32int)currAddress,
        context->endOfBlockInstr, context->hdlFunct, blockCopyCacheSize,
        (u32int)blockCopyCacheStartAddress);
      blockCopyCacheStartAddress= (u32int *)(((u32int)blockCopyCacheStartAddress) & 0xFFFFFFFE);
      protectScannedBlock(startAddress, currAddress);
      //update blockCopyCacheLastUsedLine (blockCopyCacheLastUsedLine is u32int -> add nrOfInstructions*4
      /* It doesn't matter if it points to the word before the blockCopyCache. (Guestcontext was even initialized this way) */
      context->blockCopyCacheLastUsedLine=(u32int)(blockCopyCacheCurrAddress-1);

      DEBUG(SCANNER, "Block added with size of %#.8x words" EOL, (u32int)blockCopyCacheCurrAddress
        - (u32int)blockCopyCacheStartAddress);
      /*----------------END Install HdlFunct----------------*/
      return;
    }
    else
    {
      /*
       * Non critical instruction
       */
      if (allSrcRegNonPC(instruction))
      { 
        //Non of the source registers is the ProgramCounter -> Safe instruction
        //Check if there is room on blockCopyCacheCurrAddress and if not make it
        blockCopyCacheCurrAddress=checkAndClearBlockCopyCacheAddress(blockCopyCacheCurrAddress,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
        //copy instruction to Block Copy Cache
        *(blockCopyCacheCurrAddress++)=instruction;//Copy instruction and update pointer
      }
      else
      {  
        /* One of the source registers of the instruction is the ProgramCounter */
        /* Perform PCFunct-> necessary information = currAddress, */
        /* ----------------Execute PCFunct---------------- */
        context->endOfBlockInstr = instruction;  /* Not really the endOfBlockInstr but we can use it */
        blockCopyCacheCurrAddress= decodedInstruction->PCFunct(context,currAddress,blockCopyCacheCurrAddress,blockCopyCacheStartAddress);
        if(((u32int)blockCopyCacheCurrAddress & 0b1) == 0b1)
        {
          /* Last bit of returnAddress is used to indicate that a reserved word is necessary
           * -> we can assume 2 byte alignment (even in worst case scenario (thumb))*/
          if(reservedWord==1)
          {
            /* Place has already been made -> just restore blockCopyCacheCurrAddress */
            blockCopyCacheCurrAddress=(u32int*)((u32int)blockCopyCacheCurrAddress & 0xFFFFFFFE);/*Set last bit back to zero*/
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
            u32int emptyWordPointer;
            u32int destEmptyWord = (u32int)(blockCopyCacheStartAddress + 1);
            u32int tempWordPointer;
            blockCopyCacheCurrAddress=(u32int*)((u32int)blockCopyCacheCurrAddress & 0xFFFFFFFE);/*Set last bit back to zero*/
            /* destEmptyWord can be set incorrectly if blockCopyCacheStartAddress is blockCopyCacheEnd -4
             * Because then destEmptyWord will be blockCopyCacheEnd but blockCopyCacheEnd contains a static branch
             * to start of blockCopyCache*/
            if(destEmptyWord == context->blockCopyCacheEnd)
            {
              destEmptyWord = context->blockCopyCache; //Reserved word will be at start of blockCopyCache
            }

            /*Set place for the reserved word correct now it is right before the instructions for the last instruction*/
            /*set pointer to the empty word.*/
            emptyWordPointer=(u32int)(blockCopyCacheCurrAddress-6);
            if(emptyWordPointer < context->blockCopyCache)
            {
              /* If we are before the start of the Block Copy Cache than we need to go to the corresponding place near the end*/
              u32int diff = context->blockCopyCache - emptyWordPointer;
              emptyWordPointer = context->blockCopyCacheEnd-diff;
            }
            /* emptyWordPointer now points to the empty word*/

            DEBUG(SCANNER, "emptyWordPointer : %#.8x" EOL, emptyWordPointer);
            DEBUG(SCANNER, "destEmptyWord : %#.8x" EOL, destEmptyWord);

            while (emptyWordPointer != destEmptyWord)
            {
              /* As long as the empty word isn't at its place keep on moving instructions */
              tempWordPointer = emptyWordPointer - 4; /* previous word */
              if(tempWordPointer == (context->blockCopyCache - 4))
              {
                /* Be careful when exceeding start of blockCopyCache */
                tempWordPointer = context->blockCopyCacheEnd - 4;
              }
              *((u32int *)emptyWordPointer) = *((u32int *)tempWordPointer);
              emptyWordPointer=tempWordPointer;
            }
            *((u32int *)emptyWordPointer)= 0;/*Clear it so it cannot be a cause for confusion while debugging*/
            reservedWord = 1;/*From now on there is a reserved word to save backups*/
            /* Indicate that a free word is available at start of blockCopyCache */
            blockCopyCacheStartAddress=(u32int*) ( ((u32int)blockCopyCacheStartAddress) |0b1);
          }
        }
        /*----------------END Execute PCFunct----------------*/
      }
    }
    // Instruction handled -> go to next instruction
    currAddress++;
    instruction = *currAddress;
  } // decoding while ends
}

#endif /* CONFIG_BLOCK_COPY */


static void protectScannedBlock(void *start, void *end)
{
  u32int startAddress = (u32int)start;
  u32int endAddress = (u32int)end;
  // 1. get page table entry for this address
  descriptor* ptBase = mmuGetPt0();
  descriptor* ptEntryAddr = get1stLevelPtDescriptorAddr(ptBase, startAddress);

  switch(ptEntryAddr->type)
  {
    case SECTION:
    {
      if ((startAddress & 0xFFF00000) != (endAddress & 0xFFF00000))
      {
        u32int mbStart = startAddress & 0xFFF00000;
        u32int mbEnd   = endAddress & 0xFFF00000;
        if (mbStart != (mbEnd - 0x00100000))
        {
          printf("startAddress %#.8x, endAddress %#.8x" EOL, startAddress, endAddress);
          DIE_NOW(NULL, "protectScannedBlock: Basic block crosses non-sequential section boundary!");
        }
      }
      addProtection(startAddress, endAddress, 0, PRIV_RW_USR_RO);
      break;
    }
    case PAGE_TABLE:
    {
      u32int ptEntryLvl2 = *(u32int*)(get2ndLevelPtDescriptor((pageTableDescriptor*)ptEntryAddr, endAddress));
      switch(ptEntryLvl2 & 0x3)
      {
        case LARGE_PAGE:
          printf("Page size: 64KB (large), %#.8x" EOL, ptEntryLvl2);
          DIE_NOW(NULL, "Unimplemented.");
          break;
        case SMALL_PAGE:
          if ((ptEntryLvl2 & 0x30) != 0x20)
          {
            addProtection(startAddress, endAddress, 0, PRIV_RW_USR_RO);
          }
          break;
        case FAULT:
          printf("Page invalid, %#.8x" EOL, ptEntryLvl2);
          DIE_NOW(NULL, "Unimplemented.");
          break;
        default:
          DIE_NOW(NULL, "Unrecognized second level entry");
          break;
      }
      break;
    }
    case FAULT:
      printf("Entry for basic block: invalid, %p" EOL, ptEntryAddr);
      DIE_NOW(NULL, "Unimplemented.");
      break;
    case RESERVED:
      DIE_NOW(NULL, "Entry for basic block: reserved. Error.");
      break;
    default:
      DIE_NOW(NULL, "Unrecognized second level entry. Error.");
  }
}

#ifdef CONFIG_BLOCK_COPY

/* allSrcRegNonPC will return true if all source registers of an instruction are zero  */
u32int allSrcRegNonPC(u32int instruction)
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
  if((instruction & 0xF0000)==0xF0000 || (instruction & 0xF00)==0xF00 || (instruction & 0xF)==0xF || (instruction & 0x0E508000)==0x08008000)
  {
# ifdef PC_DEBUG
    serial_putstring("Instruction 0x");
    serial_putint(instruction);
    serial_putstring(" has a PC register");
    serial_newline();
# endif
    return 0;//false
  }
  else
  {
# ifdef PC_DEBUG
    serial_putstring("Instruction 0x");
    serial_putint(instruction);
    serial_putstring(" doesn't have a PC register");
    serial_newline();
# endif
    return 1;//true
  }
}
#endif // CONFIG_BLOCK_COPY
