#include "common/debug.h"
#include "common/stddef.h"

#include "cpuArch/constants.h"

#include "guestManager/blockCache.h"

#ifdef CONFIG_THUMB2
#include "guestManager/guestExceptions.h"
#endif

#include "instructionEmu/commonInstrFunctions.h"
#include "instructionEmu/decoder.h"
#include "instructionEmu/scanner.h"

#include "memoryManager/mmu.h"
#include "memoryManager/pageTable.h"


// uncomment to enable remaining debug code not triggered from config: #define SCANNER_DEBUG


#define INSTR_SWI            0xEF000000U
#define INSTR_SWI_THUMB      0x0000DF00U
#define INSTR_NOP_THUMB      0x0000BF00U
#define INSTR_SWI_THUMB_MIX  ((INSTR_SWI_THUMB << 16) | INSTR_NOP_THUMB)


static inline u32int getHash(u32int key);

static void scanArmBlock(GCONTXT *context, u32int *start, u32int cacheIndex);
static void scanThumbBlock(GCONTXT *context, u16int *start, u32int cacheIndex);

static void protectScannedBlock(void *start, void *end);


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
        printf("Instr %.8x@%p", instruction, end);
        DIE_NOW(context, "scanner: block cache index in SWI out of range.");
      }
#ifdef SCANNER_DEBUG
      printf("scanner: EOB instruction is SWI @ %p code %x" EOL, end, svcCacheIndex);
#endif
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

  #ifdef SCANNER_DEBUG
    printf("scanner: EOB @ %#.8x insr %#.8x SVC code %x hdlrFuncPtr %x" EOL,
        currAddress, context->endOfBlockInstr, ((bcIndex + 1) << 8), (u32int)context->hdlFunct);
  #endif

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
        printf("Instr %.8x@%p", instruction, start);
        DIE_NOW(context, "scanThumbBlock: block cache index in SWI out of range");
      }
#ifdef SCANNER_DEBUG
      printf("scanner: EOB instruction is SWI @ %#.8x code %x" EOL, (u32int)start, cacheIndex);
#endif
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
#ifdef SCANNER_DBG
    printf("Thumb svc on %#.8x" EOL,(u32int)end);
#endif

    context->hdlFunct = handler;
  }

#ifdef SCANNER_DEBUG
printf("scanner: EOB @ %#.8x insr %#.8x SVC code %x hdlrFuncPtr %x" EOL,
    start, context->endOfBlockInstr, ((bcIndex + 1) << 8), (u32int)context->hdlFunct);
#endif

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

#endif


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
