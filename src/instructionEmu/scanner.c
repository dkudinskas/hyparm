#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"

#include "cpuArch/constants.h"

#include "exceptions/exceptionHandlers.h"

#ifdef CONFIG_THUMB2
#include "guestManager/guestExceptions.h"
#endif

#include "instructionEmu/blockLinker.h"
#include "instructionEmu/decoder.h"
#include "instructionEmu/decoder/arm/structs.h"
#include "instructionEmu/scanner.h"
#include "instructionEmu/translator/translator.h"

#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/translator/blockCopy.h"

#include "memoryManager/memoryProtection.h"
#include "memoryManager/mmu.h"
#include "memoryManager/pageTable.h"



static BasicBlock* scanArmBlock(GCONTXT *context, u32int *guestStart, u32int blockStoreIndex, BasicBlock* basicBlock);

#ifdef CONFIG_THUMB2
static BasicBlock* scanThumbBlock(GCONTXT *context, u16int *start, u32int metaIndex);
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


#ifdef CONFIG_SCANNER_STATISTICS
struct bigBlockList;
struct blockSizeFrequencyList;

typedef struct bigBlockList
{
  u32int startAddress;
  u32int size;
  struct bigBlockList *next;
} BigBlockList;

typedef struct blockSizeFrequencyList
{
  u32int size;
  struct blockSizeFrequencyList *next;
  u32int frequency;
} BlockSizeFrequencyList;

static BlockSizeFrequencyList *scanBlockSizes = NULL;
static BigBlockList *bigBlocks = NULL;

static BlockSizeFrequencyList *createBlockSizeFrequencyList(u32int size, BlockSizeFrequencyList *next);
void dumpScannerStatistics(void);
static void reportBlockSize(u32int *start, u32int size);

static BlockSizeFrequencyList *createBlockSizeFrequencyList(u32int size, BlockSizeFrequencyList *next)
{
  BlockSizeFrequencyList *entry = malloc(sizeof(BlockSizeFrequencyList));
  entry->size = size;
  entry->next = next;
  entry->frequency = 1;
  return entry;
}

void dumpScannerStatistics()
{
  printf("Block size    frequency" EOL);
  for (BlockSizeFrequencyList *entry = scanBlockSizes; entry != NULL; entry = entry->next)
  {
    printf("%#.8x    #%#.8x" EOL, entry->size, entry->frequency);
  }
  printf(EOL);
  printf("Big blocks" EOL);
  for (BigBlockList *entry = bigBlocks; entry != NULL; entry = entry->next)
  {
    printf("@%#.8x--%#.8x: size %x" EOL, entry->startAddress,
           entry->startAddress + 4 * (entry->size - 1), entry->size);
  }
  printf(EOL);
}

static void reportBlockSize(u32int *start, u32int size)
{
  /*
   * Increase frequency count
   */
  if (scanBlockSizes == NULL)
  {
    scanBlockSizes = createBlockSizeFrequencyList(size, NULL);
  }
  else
  {
    BlockSizeFrequencyList *entry = scanBlockSizes;
    while (TRUE)
    {
      if (entry->size == size)
      {
        entry->frequency++;
        break;
      }
      if (!entry->next || entry->next->size > size)
      {
        entry->next = createBlockSizeFrequencyList(size, entry->next);
        break;
      }
      entry = entry->next;
    }
  }
  /*
   * Update big block toplist
   */
  if (size > CONFIG_SCANNER_STATISTICS_BIG_BLOCK_TRESHOLD)
  {
    for (BigBlockList *entry = bigBlocks; entry != NULL; entry = entry->next)
    {
      if (entry->startAddress == (u32int)start && entry->size == size)
      {
        return;
      }
    }
    BigBlockList *bigBlock = malloc(sizeof(BigBlockList));
    bigBlock->startAddress = (u32int)start;
    bigBlock->size = size;
    bigBlock->next = bigBlocks;
    bigBlocks = bigBlock;
  }
}

#else
#define reportBlockSize(start, size)
#endif /* CONFIG_SCANNER_STATISTICS */

#ifdef CONFIG_DEBUG_SCANNER_MARK_INTERVAL
#define MARK_MASK  ((1 << CONFIG_DEBUG_SCANNER_MARK_INTERVAL) - 1)
#else
#define MARK_MASK  (0U)
#endif /* CONFIG_DEBUG_SCANNER_MARK_INTERVAL */


BasicBlock* scanBlock(GCONTXT *context, u32int startAddress)
{
  incrementScanBlockCounter();
#ifdef CONFIG_SCANNER_EXTRA_CHECKS
  if (getScanBlockCallSource() == SCANNER_CALL_SOURCE_NOT_SET)
  {
    DEBUG(SCANNER, "scanBlock: gc = %p, blkStartAddr = %#.8x" EOL, context, startAddress);
    DIE_NOW(context, "scanBlock() called from unknown source");
  }
#endif /* CONFIG_SCANNER_EXTRA_CHECKS */
  if ((getScanBlockCounter() & MARK_MASK) == 1)
  {
    DEBUG(SCANNER_MARK, "scanBlock: #B = %#.16Lx; startAddress = %#.8x" EOL,
                                       getScanBlockCounter(), startAddress);
  }

  BlockInfo blockInfo = getBlockInfo(context->translationStore, startAddress);
  u32int blockIndex = blockInfo.blockIndex;
  bool blockFound = blockInfo.blockFound;
  BasicBlock* basicBlock = blockInfo.blockPtr;

  DEBUG(SCANNER_BLOCK_TRACE, "scanBlock: @%.8x, source = %#x, count = %#Lx; %s" EOL, startAddress,
    getScanBlockCallSource(), getScanBlockCounter(), (blockFound ? "HIT" : "MISS"));

  context->lastEntryBlockIndex = blockIndex;

  if (blockFound)
  {
    // block was found in index.
    context->R15 = (u32int)basicBlock->codeStoreStart;
    DEBUG(SCANNER_BLOCK_TRACE, "scanBlock: context->R15 @ %08x"
                               EOL, context->R15);
    setScanBlockCallSource(SCANNER_CALL_SOURCE_NOT_SET);
    return basicBlock;
  }

#ifdef CONFIG_THUMB2
  if (context->CPSR.bits.T)
  {
    return scanThumbBlock(context, (void*)startAddress, blockIndex);
  }
  else
#endif /* CONFIG_THUMB2 */
  {
    return scanArmBlock(context, (u32int*)startAddress, blockIndex, basicBlock);
  }
  setScanBlockCallSource(SCANNER_CALL_SOURCE_NOT_SET);
}


/*
 * scan and translate a guest basic block, copying it into translation cache
 */
BasicBlock* scanArmBlock(GCONTXT* context, u32int* guestStart, u32int blockStoreIndex, BasicBlock* basicBlock)
{
  DEBUG(SCANNER, "scanArmBlock: guestStart %p block store index %x block %p\n", guestStart, blockStoreIndex, basicBlock);

  basicBlock->guestStart = guestStart;
  basicBlock->guestEnd = guestStart;
  basicBlock->codeStoreStart = context->translationStore->codeStoreFreePtr;
  basicBlock->codeStoreSize = 0;
  basicBlock->handler = NULL;
  basicBlock->type = BB_TYPE_ARM;

  // Scan guest code and copy to code store
  // translating instructions on the fly
  u32int* instructionPtr = guestStart;
#ifdef CONFIG_DECODER_AUTO
  TranslateCode code;
  AnyHandler handler;
  while ((code = decodeArmInstruction(*instructionPtr, &handler)) != IRC_REPLACE)
  {
    if (code == IRC_SAFE)
    {
      addInstructionToBlock(context->translationStore, block, *instructionPtr);
    }
    else if (code == IRC_PATCH_PC)
    {
      handler.pcHandler(context->translationStore, basicBlock, (u32int)instructionPtr, *instructionPtr);
    }
#else
  DecodedInstruction *decodedInstr;
  while((decodedInstr = decodeArmInstruction(*instructionPtr))->code != IRC_REPLACE)
  {
    if (decodedInstr->code == IRC_SAFE)
    {
      addInstructionToBlock(context->translationStore, basicBlock, *instructionPtr);
    }
    else if (decodedInstr->code == IRC_PATCH_PC)
    {
      decodedInstr->pcHandler(context->translationStore, basicBlock, (u32int)instructionPtr, *instructionPtr);
    }
#endif
    instructionPtr++;
  }

  // Next instruction must be translated into hypercall.
#ifdef CONFIG_DECODER_AUTO
  DEBUG(SCANNER, "scanArmBlock: instruction %#.8x must be translated; handler = %p" EOL, *instructionPtr, handler.barePtr);
#else
  DEBUG(SCANNER, "scanArmBlock: instruction %s must be translated; handler = %p" EOL, decodedInstr->instructionString, decodedInstr->handler);
#endif
  Instruction instruction = {.raw = *instructionPtr};
  u32int cc = instruction.raw & 0xF0000000;

  if (isBranch(instruction))
  {
    if (branchLinks(instruction))
    {
      // we need to store guest PC to R14
      armWritePCToRegister(context->translationStore, basicBlock,
                           cc >> 28, GPR_LR, ((u32int)instructionPtr)-4);
    }

    if (isConditional(instruction))
    {
      // conditional branch, two hypercalls
      u32int hypercall = (INSTR_SWI | (blockStoreIndex+0x100)) & 0x0FFFFFFF;
      addInstructionToBlock(context->translationStore, basicBlock, hypercall | cc);
      addInstructionToBlock(context->translationStore, basicBlock, hypercall | (AL << 28));
      basicBlock->oneHypercall = FALSE;
    }
    else
    {
      // not-conditional branch. single hypercall.
      addInstructionToBlock(context->translationStore, basicBlock, INSTR_SWI | (blockStoreIndex+0x100));
      basicBlock->oneHypercall = TRUE;
    }
  }
  else
  {
    // not a branch
    addInstructionToBlock(context->translationStore, basicBlock, INSTR_SWI | (blockStoreIndex+0x100));
    basicBlock->oneHypercall = TRUE;
  }

  // plant block index as well. dirty hack, but fixes interrupt handling.
  addInstructionToBlock(context->translationStore, basicBlock, blockStoreIndex);

  // set guest end of block address
  basicBlock->guestEnd = instructionPtr;
#ifdef CONFIG_DECODER_AUTO
  basicBlock->handler = handler.handler;
#else
  basicBlock->handler = decodedInstr->handler;
#endif

  DEBUG(SCANNER, "scanArmBlock: instr %08x @ %p SWIcode %02x hdlrFuncPtr %p" EOL,
        *instructionPtr, instructionPtr, blockStoreIndex, basicBlock->handler);

  // we will start executing at the first instruction of this block inside code store
  context->R15 = (u32int)basicBlock->codeStoreStart;
  DEBUG(SCANNER, "scanArmBlock: code store start of block %08x\n", context->R15);

  // Protect guest against self-modification.
  guestWriteProtect(context, (u32int)guestStart, (u32int)instructionPtr);

  // unify L1 instruction and data caches
  mmuUnifyCaches((u32int)basicBlock->codeStoreStart, basicBlock->codeStoreSize);

  setExecBitmap(context, (u32int)basicBlock->guestStart, (u32int)basicBlock->guestEnd);
  return basicBlock;
}


/*
 * re-scan-and-translate a guest basic block
 * do NOT copy anything into code store
 * this called upon a bad case when we have to somehow map
 * a translation store PC to its original guest PC. RARE case. 
 */
u32int rescanBlock(GCONTXT *context, u32int blockStoreIndex, BasicBlock* block, u32int hostPC)
{
  DEBUG(SCANNER, "rescanBlock: hostPC %08x block store index %x block %p\n", hostPC, blockStoreIndex, block);
  DEBUG(SCANNER, "rescanBlock: bb->gStart %p bb->gEnd %p bb->csStart %p bb->csSize %08x\n",
         block->guestStart, block->guestEnd, block->codeStoreStart, block->codeStoreSize);

  // backup code store pointer
  u32int* csFreeBackup = context->translationStore->codeStoreFreePtr;

  // set the pointer to the start of current block for rescanning
  context->translationStore->codeStoreFreePtr = block->codeStoreStart;

  // disable code store writes - all calls to addInstructionToBlock will NOT write
  context->translationStore->write = FALSE;

  // Scan guest code
  u32int* instructionPtr = block->guestStart;

#ifdef CONFIG_DECODER_AUTO
  TranslateCode code;
  AnyHandler handler;
  while ((code = decodeArmInstruction(*instructionPtr, &handler)) != IRC_REPLACE)
  {
    DEBUG(SCANNER, "rescanBlock: guest instr ptr %p host %p" EOL, instructionPtr, context->translationStore->codeStoreFreePtr);
    if (code == IRC_SAFE)
    {
      addInstructionToBlock(context->translationStore, block, *instructionPtr);
    }
    else if (code == IRC_PATCH_PC)
    {
      handler.pcHandler(context->translationStore, block, (u32int)instructionPtr, *instructionPtr);
    }
#else
  DecodedInstruction* decodedInstr;
  while((decodedInstr = decodeArmInstruction(*instructionPtr))->code != IRC_REPLACE)
  {
    DEBUG(SCANNER, "rescanBlock: guest instr ptr %p host %p" EOL, instructionPtr, context->translationStore->codeStoreFreePtr);
    if (decodedInstr->code == IRC_SAFE)
    {
      addInstructionToBlock(context->translationStore, block, *instructionPtr);
    }
    else if (decodedInstr->code == IRC_PATCH_PC)
    {
      decodedInstr->pcHandler(context->translationStore, block, (u32int)instructionPtr, *instructionPtr);
    }
#endif

    // if we reached or 'gone past' the looked for address during the scanning
    // of the last guest instruction, we found the mapping.
    if ((u32int)context->translationStore->codeStoreFreePtr > hostPC)
    {
      DEBUG(SCANNER, "rescanBlock: current cs ptr %p\n", context->translationStore->codeStoreFreePtr);
      // restore code store free pointer
      context->translationStore->codeStoreFreePtr = csFreeBackup;
      // re-enable code store writes
      context->translationStore->write = TRUE;

      // and we can safely return now.
      return (u32int)instructionPtr;
    }

    // we havent reached the instruction we are looking for yet. carry on.
    instructionPtr++;
  }

  DIE_NOW(context, "rescanBlock: not sure if we should ever get here.\n");

  // just compiler happy
  return 0;
}

