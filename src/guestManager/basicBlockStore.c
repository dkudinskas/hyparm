#include "guestManager/basicBlockStore.h"
#include "guestManager/translationStore.h"
#include "guestManager/guestContext.h"

#include "common/debug.h"
#include "common/linker.h"
#include "common/stdlib.h"

#include "memoryManager/mmu.h"


u32int getBasicBlockStoreIndex(u32int startAddress)
{
  return (startAddress >> 2) & (BASIC_BLOCK_STORE_SIZE - 1);
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
    *ts->codeStoreFreePtr = instruction;
  }

  /* to ensure data and instruction cache coherency
   * 1. clean data cache entry by start and end address of code store
   * (DCCMVAU, Clean data cache line by MVA to PoU: c7, 0, c11, 1)
   * 2. invalidate instruction cache entry by start and end address.
   * (ICIMVAU, Invalidate instruction caches by MVA to PoU: c7, 0, c5, 1) */
  mmuCleanDCacheByMVAtoPOU((u32int)ts->codeStoreFreePtr);
  mmuInvIcacheByMVAtoPOU((u32int)ts->codeStoreFreePtr);
  DEBUG(BLOCK_STORE, "addInstructionToBlock: codeStore loc %p is now %08x\n",
                                 ts->codeStoreFreePtr, *ts->codeStoreFreePtr);

  ts->codeStoreFreePtr++;
  basicBlock->codeStoreSize++;

  if ((u32int)ts->codeStoreFreePtr >= RAM_CODE_CACHE_POOL_END)
  {
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
    basicBlock->addressMap = tempBlock.addressMap;
  }
  
  DEBUG(BLOCK_STORE, "addInstructionToBlock: codeStoreSize %x\n", basicBlock->codeStoreSize);
}


void invalidateBlock(BasicBlock* block)
{
  memset(block, 0, sizeof(BasicBlock));
}

void setExecBitmap(GCONTXT* context, u32int start, u32int end)
{
  // calculate which byte and bit in bitmap
  u32int startingSection = start >> 20;
  u32int endingSection = end >> 20;
  u32int byteIndex = startingSection >> 3;
  u32int bitIndex = startingSection - (byteIndex << 3);
  u8int actualByte = context->execBitmap[byteIndex];
  u8int actualBit = (actualByte >> bitIndex) & 1;
  if (actualBit == 0)
  {
    context->execBitmap[byteIndex] = actualByte | (1 << bitIndex);
  }
  
  if (startingSection != endingSection)
  {
    // we span two sections. do another bit..
    u32int byteIndexEnd = endingSection >> 3;
    u32int bitIndexEnd = endingSection - (byteIndexEnd << 3);
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
  u32int section = addr >> 20;
  u32int byteIndex = section >> 3;
  u32int bitIndex = section - (byteIndex << 3);
  u8int actualByte = context->execBitmap[byteIndex];
  u8int actualBit = (actualByte >> bitIndex) & 1;
  return (actualBit != 0) ? TRUE : FALSE;
}
