#include "guestManager/basicBlockStore.h"
#include "guestManager/translationStore.h"
#include "guestManager/guestContext.h"

#include "common/debug.h"
#include "common/helpers.h"
#include "common/linker.h"
#include "common/stdlib.h"

#include "memoryManager/mmu.h"


static inline u32int getBasicBlockStoreIndex(u32int startAddress)
{
  return (startAddress >> 2) & (BASIC_BLOCK_STORE_SIZE - 1);
}


BlockInfo getBlockInfo(TranslationStore* ts, u32int startAddress)
{
  BlockInfo info = {.blockFound = FALSE, .blockIndex = 0};
  u32int index = getBasicBlockStoreIndex(startAddress);

  BasicBlock* block;

  do
  {
    block = &ts->basicBlockStore[index];
    if (block->type == BB_TYPE_INVALID)
    {
      // no block here
      info.blockFound = FALSE;
      info.blockIndex = index;
      info.blockPtr = block;
      return info;
    }
    else
    {
      if ((u32int)block->guestStart == startAddress)
      {
        // found block here
        info.blockFound = TRUE;
        info.blockIndex = index;
        info.blockPtr = block;
        return info;
      }
      else
      {
        // some other block lives here, keep calm and carry on.
        index++;
      }
    }
  } // really loop until block is found of placed.
  while (TRUE);
}


BasicBlock* getBasicBlockStoreEntry(TranslationStore* ts, u32int index)
{
  DEBUG(BLOCK_STORE, "getBasicBlockStoreEntry: index = %x\n", index);
  return &ts->basicBlockStore[index];
}


void addInstructionToBlock(struct TranslationStore* ts, BasicBlock* basicBlock, u32int instruction)
{
  DEBUG(BLOCK_STORE, "addInstructionToBlock: block %p, instruction %08x\n", basicBlock, instruction);

  if (ts->write)
  {
    basicBlock->codeStoreSize++;
    *ts->codeStoreFreePtr = instruction;
    mmuCleanDCacheByMVAtoPOU((u32int)ts->codeStoreFreePtr);
    mmuInvIcacheByMVAtoPOU((u32int)ts->codeStoreFreePtr);
    DEBUG(BLOCK_STORE, "addInstructionToBlock: codeStore loc %p is now %08x\n",
                                   ts->codeStoreFreePtr, *ts->codeStoreFreePtr);
  }
  ts->codeStoreFreePtr++;

  if ((u32int)ts->codeStoreFreePtr >= RAM_CODE_CACHE_POOL_END)
  {
    printf("reached the end of code cache pool!\n");
    dumpBlockStoreStats(getActiveGuestContext());
    // reset free pointer
    ts->codeStoreFreePtr = ts->codeStore;

    // copy instructions that have already been copied for this block to front of CS
    u32int i = 0;
    u32int* instrPtr = basicBlock->codeStoreStart;
    for (i = 0; i < basicBlock->codeStoreSize; i++)
    {
      if (ts->write)
      {
        *ts->codeStoreFreePtr = *instrPtr;
      }
      ts->codeStoreFreePtr++;
      instrPtr++;
    }

    if (ts->write)
    {
      basicBlock->codeStoreStart = ts->codeStore;

      // save all information from basic block entry
      // as basic block store is going to be zeroed
      BasicBlock tempBlock = *basicBlock;

      // invalidate the basic block store
      memset(ts->basicBlockStore, 0, BASIC_BLOCK_STORE_SIZE * sizeof(BasicBlock));

      // restore basic block store entry
      basicBlock->guestStart = tempBlock.guestStart;
      basicBlock->guestEnd = tempBlock.guestEnd;
      basicBlock->codeStoreStart = tempBlock.codeStoreStart;
      basicBlock->codeStoreSize = tempBlock.codeStoreSize;
      basicBlock->handler = tempBlock.handler;
      basicBlock->type = tempBlock.type;
    }
  }

  DEBUG(BLOCK_STORE, "addInstructionToBlock: codeStoreSize %x\n", basicBlock->codeStoreSize);
}


void invalidateBlock(BasicBlock* block)
{
  // its enough to change type to invalid.
  block->type = BB_TYPE_INVALID;
}

void setExecBitmap(GCONTXT* context, u32int start, u32int end)
{
  // calculate which byte and bit in bitmap
  u32int startingPage = start >> 12;
  u32int endingPage = end >> 12;
  u32int byteIndex = startingPage >> 3;
  u32int bitIndex = startingPage - (byteIndex << 3);
  u8int actualByte = context->execBitmap[byteIndex];
  u8int actualBit = (actualByte >> bitIndex) & 1;
  if (actualBit == 0)
  {
    context->execBitmap[byteIndex] = actualByte | (1 << bitIndex);
  }

  if (startingPage != endingPage)
  {
    // we span two sections. do another bit..
    u32int byteIndexEnd = endingPage >> 3;
    u32int bitIndexEnd = endingPage - (byteIndexEnd << 3);
    u8int actualByteEnd = context->execBitmap[bitIndexEnd];
    u8int actualBitEnd = (actualByteEnd >> bitIndexEnd) & 1;
    if (actualBitEnd == 0)
    {
      context->execBitmap[byteIndexEnd] = actualByteEnd | (1 << bitIndexEnd);
    }
  }
}


bool isExecBitSet(GCONTXT* context, u32int addr)
{
  u32int page = addr >> 12;
  u32int byteIndex = page >> 3;
  u32int bitIndex = page - (byteIndex << 3);
  u8int actualByte = context->execBitmap[byteIndex];
  u8int actualBit = (actualByte >> bitIndex) & 1;
  return (actualBit != 0) ? TRUE : FALSE;
}


void dumpBlock(BasicBlock* block)
{
  printf("dumpBlock: type %x\n", block->type);
  printf("dumpBlock: guestStart %p\n", block->guestStart);
  printf("dumpBlock: guestEnd %p\n", block->guestEnd);
  printf("dumpBlock: codeStoreStart %p\n", block->codeStoreStart);
  printf("dumpBlock: codeStoreSize %d\n", block->codeStoreSize);
  printf("dumpBlock: handler %p\n", block->handler);
  printf("dumpBlock: oneHypercall %x\n", block->oneHypercall);
}


void dumpBlockStoreStats(GCONTXT* context)
{
  BasicBlock* index = context->translationStore->basicBlockStore;
  u32int occupied = 0, free = 0;
  u32int i;
  u32int sizeOfBlocks = 0;
  for (i = 0; i < BASIC_BLOCK_STORE_SIZE; i++)
  {
    if (index[i].type == BB_TYPE_INVALID)
    {
      free++;
    }
    else
    {
      occupied++;
      sizeOfBlocks += index[i].codeStoreSize;
    }
  }
  printf("======================================================\n");
  printf("Basic Block Index Entries:    %08x\n", BASIC_BLOCK_STORE_SIZE);
  printf("Basic Block entries free:     %08x\n", free);
  printf("Basic Block entries occupied: %08x\n", occupied);
  printf("total used block store space %08x\n", sizeOfBlocks);
  printf("======================================================\n");

}

