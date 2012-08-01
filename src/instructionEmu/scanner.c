#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"

#include "cpuArch/constants.h"

#include "exceptions/exceptionHandlers.h"

#ifdef CONFIG_THUMB2
#include "guestManager/guestExceptions.h"
#endif

#include "instructionEmu/decoder.h"
#include "instructionEmu/scanner.h"

#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/translator/blockCopy.h"

#include "memoryManager/memoryProtection.h"
#include "memoryManager/mmu.h"
#include "memoryManager/pageTable.h"


#define INSTR_SWI            0xEF000000U
#define INSTR_SWI_THUMB      0x0000DF00U
#define INSTR_NOP_THUMB      0x0000BF00U
#define INSTR_SWI_THUMB_MASK 0x0000FF00U
#define INSTR_SWI_THUMB_MIX  ((INSTR_SWI_THUMB << 16) | INSTR_NOP_THUMB)


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
  u32int blockIndex = getBasicBlockStoreIndex(startAddress);
  BasicBlock* basicBlock = getBasicBlockStoreEntry(context->translationStore, blockIndex);
  
  bool blockFound = FALSE;
  
  u32int addressMap = context->virtAddrEnabled ? (u32int)context->pageTables->guestPhysical : 0;

  if (basicBlock->type != BB_TYPE_INVALID)
  {
    if ((basicBlock->addressMap == addressMap)
     && ((u32int)basicBlock->guestStart == startAddress))
    {
      // entry valid, address map maches, and start address maches. really found!
      blockFound = TRUE;
    }
    else
    {
      invalidateBlock(basicBlock);
      blockFound = FALSE;
    }
  }

  DEBUG(SCANNER_BLOCK_TRACE, "scanBlock: @%.8x, source = %#x, count = %#Lx; %s" EOL, startAddress,
    getScanBlockCallSource(), getScanBlockCounter(), (blockFound ? "HIT" : "MISS"));

  setScanBlockCallSource(SCANNER_CALL_SOURCE_NOT_SET);

  if (blockFound)
  {
    context->R15 = (u32int)basicBlock->codeStoreStart;
    //But also the PC of the last instruction of the block should be set
    context->lastGuestPC = (u32int)basicBlock->guestEnd;
    DEBUG(SCANNER_BLOCK_TRACE, "scanBlock: lastGuestPC = %08x, context->R15 @ %08x" 
                                        EOL, context->lastGuestPC, context->R15);
    return;
  }

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

  // Scan guest code and copy to C$, translating instructions that use the PC on the fly
  u32int* instructionPtr = guestStart;
  struct decodingTableEntry* decodedInstruction;

  for (; (decodedInstruction = decodeArmInstruction(*instructionPtr))->replace == IRC_SAFE; ++instructionPtr)
  {
    if (armIsPCInsensitiveInstruction(*instructionPtr) || decodedInstruction->pcHandler == NULL)
    {
      addInstructionToBlock(context->translationStore, basicBlock, *instructionPtr);
    }
    else
    {
      DEBUG(SCANNER, "scanArmBlock: instruction %#.8x @ %p possibly uses PC as source operand" EOL, *instructionPtr, instructionPtr);
      decodedInstruction->pcHandler(context->translationStore, basicBlock, (u32int)instructionPtr, *instructionPtr);
    }
  }

  // Next instruction must be translated into hypercall.
  DEBUG(SCANNER, "scanArmBlock: instruction %s must be translated\n", decodedInstruction->instructionString);
  addInstructionToBlock(context->translationStore, basicBlock, INSTR_SWI | (blockStoreIndex+0x100));

  // set guest end of block address
  basicBlock->guestEnd = instructionPtr;
  basicBlock->handler = decodedInstruction->handler;
  
  context->lastGuestPC = (u32int)instructionPtr;
  DEBUG(SCANNER, "scanArmBlock: last guest PC set to %08x\n", context->lastGuestPC);
  DEBUG(SCANNER, "scanArmBlock: instr %08x @ %p SWIcode %02x hdlrFuncPtr %p" EOL, 
        *instructionPtr, instructionPtr, blockStoreIndex, basicBlock->handler);

  // we will start executing at the first instruction of this block inside code store
  context->R15 = (u32int)basicBlock->codeStoreStart;
  DEBUG(SCANNER, "scanArmBlock: code store start of block %08x\n", context->R15);

  // Protect guest against self-modification.
  guestWriteProtect(context, (u32int)guestStart, (u32int)instructionPtr);
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
