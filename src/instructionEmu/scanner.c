#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"

#include "cpuArch/constants.h"

#include "exceptions/exceptionHandlers.h"

#ifdef CONFIG_THUMB2
#include "guestManager/guestExceptions.h"
#endif

#include "instructionEmu/decoder.h"
#include "instructionEmu/decoder/arm/structs.h"
#include "instructionEmu/scanner.h"
#include "instructionEmu/translator/translator.h"

#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/translator/blockCopy.h"

#include "memoryManager/memoryProtection.h"
#include "memoryManager/mmu.h"
#include "memoryManager/pageTable.h"


static inline bool isBranch(u32int instruction);
static inline bool isServiceCall(u32int instruction);
static inline bool isUndefinedCall(u32int instruction);

static void scanArmBlock(GCONTXT *context, u32int *guestStart, u32int blockStoreIndex, BasicBlock* basicBlock);
static bool armIsPCInsensitiveInstruction(u32int instruction);

#ifdef CONFIG_THUMB2
static void scanThumbBlock(GCONTXT *context, u16int *start, u32int metaIndex);
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


void scanBlock(GCONTXT *context, u32int startAddress)
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
    DEBUG(SCANNER_MARK, "scanBlock: #B = %#.16Lx; startAddress = %#.8x" EOL, getScanBlockCounter(), startAddress);
  }
  u32int blockIndex = getBasicBlockStoreIndex(startAddress);
  BasicBlock* basicBlock = getBasicBlockStoreEntry(context->translationStore, blockIndex);
  
  bool blockFound = FALSE;
  u32int addressMap = context->virtAddrEnabled ? (u32int)context->pageTables->guestPhysical : 0;
  if (basicBlock->type != BB_TYPE_INVALID)
  {
    if ((basicBlock->addressMap == addressMap) && ((u32int)basicBlock->guestStart == startAddress))
    {
      // entry valid, address map maches, and start address maches. really found!
      blockFound = TRUE;
    }
    else
    {
      // conflict!
      if (basicBlock->type == GB_TYPE_ARM)
      {
        context->groupBlockVersion++;
      }
      invalidateBlock(basicBlock);
      blockFound = FALSE;
    }
  }

  DEBUG(SCANNER_BLOCK_TRACE, "scanBlock: @%.8x, source = %#x, count = %#Lx; %s" EOL, startAddress,
    getScanBlockCallSource(), getScanBlockCounter(), (blockFound ? "HIT" : "MISS"));

  context->lastEntryBlockIndex = blockIndex;

  if (blockFound)
  {
    // block was found in index. but if it is a group block
    // we must check its version number: it could be out of date!
    if ((basicBlock->type == GB_TYPE_ARM) && 
        (basicBlock->versionNumber != context->groupBlockVersion))
    {
      // group block is out of date. remove it, scan again
      unlinkBlock(basicBlock, blockIndex);
    }
    basicBlock->hotness++;
    context->R15 = (u32int)basicBlock->codeStoreStart;
    // But also the PC of the last instruction of the block should be set
    context->lastGuestPC = (u32int)basicBlock->guestEnd;
    DEBUG(SCANNER_BLOCK_TRACE, "scanBlock: lastGuestPC = %08x, context->R15 @ %08x"
                                        EOL, context->lastGuestPC, context->R15);
    setScanBlockCallSource(CALL_SOURCE_NOT_SET);
    return;
  }

  basicBlock->hotness = 1;
#ifdef CONFIG_THUMB2
  if (context->CPSR & PSR_T_BIT)
  {
    scanThumbBlock(context, (void*)startAddress, blockIndex);
  }
  else
#endif /* CONFIG_THUMB2 */
  {
    scanArmBlock(context, (u32int*)startAddress, blockIndex, basicBlock);
  }
  setScanBlockCallSource(CALL_SOURCE_NOT_SET);
}


/*
 * TODO: caching is disabled on the code store
 * so no cache maintenance operations are called from here
 */
void scanArmBlock(GCONTXT* context, u32int* guestStart, u32int blockStoreIndex, BasicBlock* basicBlock)
{
  DEBUG(SCANNER, "scanArmBlock: guestStart %p block store index %x block %p\n", guestStart, blockStoreIndex, basicBlock);

  basicBlock->guestStart = guestStart;
  basicBlock->guestEnd = guestStart;
  basicBlock->codeStoreStart = context->translationStore->codeStoreFreePtr;
  basicBlock->codeStoreSize = 0;
  basicBlock->handler = NULL;
  basicBlock->type = BB_TYPE_ARM;

  basicBlock->addressMap = (context->virtAddrEnabled) ? (u32int)context->pageTables->guestPhysical : 0;


  // Scan guest code and copy to code store
  // translating instructions on the fly
  u32int* instructionPtr = guestStart;
  DecodedInstruction* decodedInstr;

  while((decodedInstr = decodeArmInstruction(*instructionPtr))->code != IRC_REPLACE)
  {
    if (armIsPCInsensitiveInstruction(*instructionPtr) || decodedInstr->pcHandler == NULL)
    {
      translate(context, basicBlock, decodedInstr, *instructionPtr);
    }
    else
    {
      DEBUG(SCANNER, "scanArmBlock: instruction %#.8x @ %p possibly uses PC as source operand" EOL, *instructionPtr, instructionPtr);
      decodedInstr->pcHandler(context->translationStore, basicBlock, (u32int)instructionPtr, *instructionPtr);
    }
    instructionPtr++;
  }


  // Next instruction must be translated into hypercall.
  DEBUG(SCANNER, "scanArmBlock: instruction %s must be translated\n", decodedInstr->instructionString);
  u32int instruction = *instructionPtr;
  u32int cc = instruction & 0xF0000000;
  if ((isBranch(instruction)) && (cc != 0xE0000000))
  {
    // conditional branch! two hypercalls
    u32int hypercall = (INSTR_SWI | (blockStoreIndex+0x100)) & 0x0FFFFFFF;
    addInstructionToBlock(context->translationStore, basicBlock, hypercall | cc);
    addInstructionToBlock(context->translationStore, basicBlock, hypercall | (CC_AL << 28));
    basicBlock->oneHypercall = FALSE;
  }
  else
  {
    // not a branch or not-conditional branch. single hypercall.
    addInstructionToBlock(context->translationStore, basicBlock, INSTR_SWI | (blockStoreIndex+0x100));
    basicBlock->oneHypercall = TRUE;
  }
  // plant block index as well. dirty hack, but fixes interrupt handling.
  addInstructionToBlock(context->translationStore, basicBlock, blockStoreIndex);

  // set guest end of block address
  basicBlock->guestEnd = instructionPtr;
  basicBlock->handler = decodedInstr->handler;
  
  context->lastGuestPC = (u32int)instructionPtr;
  DEBUG(SCANNER, "scanArmBlock: last guest PC set to %08x\n", context->lastGuestPC);
  DEBUG(SCANNER, "scanArmBlock: instr %08x @ %p SWIcode %02x hdlrFuncPtr %p" EOL, 
        *instructionPtr, instructionPtr, blockStoreIndex, basicBlock->handler);

  // we will start executing at the first instruction of this block inside code store
  context->R15 = (u32int)basicBlock->codeStoreStart;
  DEBUG(SCANNER, "scanArmBlock: code store start of block %08x\n", context->R15);

  // Protect guest against self-modification.
  guestWriteProtect(context, (u32int)guestStart, (u32int)instructionPtr);

  setExecBitmap(context, (u32int)basicBlock->guestStart, (u32int)basicBlock->guestEnd);
}


/*
 * armIsPCInsensitiveInstruction returns TRUE if it is CERTAIN that an instruction
 * does not depend on the current PC value.
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
  if ((instruction & 0x0e500000) == 0x08000000)
  {
    // this is an STM instruction
    if ((instruction & 0x0E508000) != 0x08008000)
    {
      return TRUE;
    }
    else
    {
      return FALSE;
    }
  }
  else
  {
    // not a store multiple
    u32int rd = ARM_EXTRACT_REGISTER(instruction, 16);
    u32int rn = ARM_EXTRACT_REGISTER(instruction, 12);
    u32int rm = ARM_EXTRACT_REGISTER(instruction, 0);

    return !((rd == GPR_PC) || (rn == GPR_PC) || (rm == GPR_PC));
  }
}


inline bool isBranch(u32int instruction)
{
  if ((instruction & 0x0f000000) == 0x0a000000)
  {
    return TRUE;
  }
  return FALSE;
}


inline bool isServiceCall(u32int instruction)
{
  // opcode must be F, condition code must not be F.
  if (((instruction & 0x0f000000) == 0x0f000000)
   && ((instruction & 0xf0000000) != 0xf0000000))
  {
    return TRUE;
  }
  return FALSE;
}


static inline bool isUndefinedCall(u32int instruction)
{
  // undefined call is 0xFFFFFFXX (last two numbers will hold spill register number)
  if ((instruction & UNDEFINED_CALL) == UNDEFINED_CALL)
  {
    return TRUE;
  }
  return FALSE;
}


void linkBlock(GCONTXT *context, u32int nextPC, u32int lastPC, BasicBlock* lastBlock)
{
  // get next block
  u32int blockIndex = getBasicBlockStoreIndex(nextPC);
  BasicBlock* nextBlock = getBasicBlockStoreEntry(context->translationStore, blockIndex);
  
  // if invalid, return.
  if ((nextBlock->type == BB_TYPE_INVALID) || (nextBlock->hotness < 10))
  {
    // next block not scanned.
    return;
  }

  u32int addressMap = context->virtAddrEnabled ? (u32int)context->pageTables->guestPhysical : 0;
  if (((u32int)nextBlock->guestStart != nextPC) || (nextBlock->addressMap != addressMap))
  {
    // conflict. return, let scanBlock() deal with conflict first, link later.
    return;
  }

  u32int eobInstruction = *(lastBlock->guestEnd);
  if (!isBranch(eobInstruction))
  {
    // not a branch. just leave hypercall.
    return;
  }


  // if we found a group block with an old version number, fail to link. 
  // scanner will then re-scan this block, thus validating it's correct
  if ((nextBlock->type == GB_TYPE_ARM) && (nextBlock->versionNumber != context->groupBlockVersion))
  {
    return;
  }
  if ((lastBlock->type == GB_TYPE_ARM) && (lastBlock->versionNumber != context->groupBlockVersion))
  {
    return;
  }

  // must check wether it was the first or second hypercall
  u32int lastInstrOfHostBlock = (u32int)lastBlock->codeStoreStart +
                                (lastBlock->codeStoreSize-1) * ARM_INSTRUCTION_SIZE;
  lastInstrOfHostBlock -= ARM_INSTRUCTION_SIZE;
  u32int conditionCode = 0xE0000000;
  if (lastPC != lastInstrOfHostBlock)
  {
    conditionCode = eobInstruction & 0xF0000000;
  }

  // must try to put a branch at lastPC direct to nextPC
  putBranch(lastPC, (u32int)nextBlock->codeStoreStart, conditionCode);

  // make sure both blocks are marked as a group-block.
  lastBlock->type = GB_TYPE_ARM;
  lastBlock->versionNumber = context->groupBlockVersion;
  nextBlock->type = GB_TYPE_ARM;
  nextBlock->versionNumber = context->groupBlockVersion;
}


void unlinkBlock(BasicBlock* block, u32int index)
{
  u32int hypercall = INSTR_SWI | (index + 0x100);
  u32int lastInstrOfHostBlock = (u32int)block->codeStoreStart +
                                (block->codeStoreSize-1) * ARM_INSTRUCTION_SIZE;
  lastInstrOfHostBlock -= ARM_INSTRUCTION_SIZE;

  // remove 'last' hypercall (or the only one if there werent more)
  *(u32int*)lastInstrOfHostBlock = hypercall;
  mmuCleanDCacheByMVAtoPOU(lastInstrOfHostBlock);
  mmuInvIcacheByMVAtoPOU(lastInstrOfHostBlock);

  if (!block->oneHypercall)
  {
    // two hypercalls! fix up condition code.
    u32int condition = *(u32int*)(lastInstrOfHostBlock-ARM_INSTRUCTION_SIZE);
    condition &= 0xF0000000; 
    hypercall = (hypercall & 0x0FFFFFFF) | condition;
    
    *(u32int*)(lastInstrOfHostBlock-ARM_INSTRUCTION_SIZE) = hypercall;
    mmuCleanDCacheByMVAtoPOU(lastInstrOfHostBlock-ARM_INSTRUCTION_SIZE);
    mmuInvIcacheByMVAtoPOU(lastInstrOfHostBlock-ARM_INSTRUCTION_SIZE);
  }
  block->type = BB_TYPE_ARM;
  block->versionNumber = 0;
}


void unlinkAllBlocks(GCONTXT *context)
{
  DIE_NOW(0, "unlinkAllBlocks unimplemented.\n");
  u32int i = 0;
  /* we traverse the complete block translation store
   * inside the loop we unlink all current group-blocks. */ 
  for (i = 0; i < BASIC_BLOCK_STORE_SIZE; i++)
  {
    if (context->translationStore->basicBlockStore[i].type == GB_TYPE_ARM)
    {
      unlinkBlock(&context->translationStore->basicBlockStore[i], i);
    }
  }
}


u32int findBlockIndexNumber(GCONTXT *context, u32int hostPC)
{
  bool found = FALSE;
  u32int* pc = (u32int*)hostPC;
  u32int index = 0;
  while (!found)
  {
    u32int instruction = *pc;
    if (isUndefinedCall(instruction))
    {
      // next word will be DATA! spill location. skip it and loop back around
      pc++;
    }
    else if (isServiceCall(instruction))
    {
      // lets get the code from the service call.
      index = (instruction & 0x00ffffff) - 0x100;
      found = TRUE;
    }
    else if (isBranch(instruction))
    {
      // found our first branch. skip it.
      pc++;
      instruction = *pc;

      // one branch down. lets see next one. MUST be branch, svc or the index
      if (isBranch(instruction))
      {
        // second branch! next word is the index 4 sure
        pc++;
        index = *pc;
      }
      else if (isServiceCall(instruction))
      {
        // ok, branch followed by hypecall. get the code from it THIS TIME
        index = (instruction & 0x00ffffff) - 0x100;
      }
      else
      {
        // since we had one branch, and the next instruction is not a branch or hypercall
        // by the power of deduction and logical reasoning, we found the code.
        index = instruction;
      }
      // now we really found it.
      found = TRUE;
    } // found first branch ends
    // instr was not a branch neither a hypercall or undefined call
    pc++;
  }
  return index;
}


void putBranch(u32int branchLocation, u32int branchTarget, u32int condition)
{
  u32int offset = branchTarget - (branchLocation + ARM_INSTRUCTION_SIZE*2);
  offset = (offset >> 2) & 0xFFFFFF;
  u32int branchInstruction = condition | BRANCH_BASE_VALUE | offset;
  *(u32int*)branchLocation = branchInstruction;

  mmuCleanDCacheByMVAtoPOU(branchLocation);
  mmuInvIcacheByMVAtoPOU(branchLocation);
}
