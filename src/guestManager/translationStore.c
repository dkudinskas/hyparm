#include "common/debug.h"
#include "common/stdlib.h"
#include "common/linker.h"

#include "cpuArch/constants.h"

#include "guestManager/translationStore.h"
#include "guestManager/codeStore.h"

#include "instructionEmu/blockLinker.h"
#include "instructionEmu/scanner.h"

#include "memoryManager/mmu.h"


void initialiseTranslationStore(TranslationStore* ts)
{
  DEBUG(TRANSLATION_STORE, "initialiseTranslationStore: translation store @ %p" EOL, ts);

  ts->codeStore = (u32int*)RAM_CODE_CACHE_POOL_BEGIN;
  DEBUG(TRANSLATION_STORE, "initialiseTranslationStore: code store @ %p\n", ts->codeStore);
  // STARFIX: remove all memset zero for naive memory allocator
  memset(ts->codeStore, 0, RAM_CODE_CACHE_POOL_END-RAM_CODE_CACHE_POOL_BEGIN);

  ts->codeStoreFreePtr = ts->codeStore;
  DEBUG(TRANSLATION_STORE, "initialiseTranslationStore: code store free ptr @ %p\n", ts->codeStoreFreePtr);

  DEBUG(TRANSLATION_STORE, "initialiseTranslationStore basic block entry size %x\n", sizeof(BasicBlock));
  ts->basicBlockStore = (BasicBlock*)malloc(BASIC_BLOCK_STORE_SIZE * sizeof(BasicBlock));
  if (ts->basicBlockStore == NULL)
  {
    DIE_NOW(context, "Failed to allocate code store");
  }
  DEBUG(TRANSLATION_STORE, "initialiseTranslationStore: basic block store @ %p\n", ts->basicBlockStore);
  // STARFIX: remove all memset zero for naive memory allocator
  memset(ts->basicBlockStore, 0, BASIC_BLOCK_STORE_SIZE * sizeof(BasicBlock));

  ts->write = TRUE;
}


void instructionToCodeStore(TranslationStore* ts, u32int instruction)
{
  *ts->codeStoreFreePtr = instruction;
  DEBUG(TRANSLATION_STORE, "instructionToCodeStore: codeStore loc %p is now %08x\n",
                                 ts->codeStoreFreePtr, *ts->codeStoreFreePtr);
  ts->codeStoreFreePtr++;
  if ((u32int)ts->codeStoreFreePtr >= RAM_CODE_CACHE_POOL_END)
  {
    DIE_NOW(0, "instructionToCodeStore: code store full!\n");
  }
}


void clearTranslationsAll(TranslationStore* ts)
{
  DEBUG(TRANSLATION_STORE, "clearTranslationsAll: clear all translations\n");

  ts->codeStore = (u32int*)RAM_CODE_CACHE_POOL_BEGIN;
  memset(ts->codeStore, 0, RAM_CODE_CACHE_POOL_END-RAM_CODE_CACHE_POOL_BEGIN);

  ts->codeStoreFreePtr = ts->codeStore;
  DEBUG(TRANSLATION_STORE, "clearTranslationsAll: code store free ptr @ %p\n", ts->codeStoreFreePtr);

  DEBUG(TRANSLATION_STORE, "clearTranslationsAll: basic block store @ %p\n", ts->basicBlockStore);
  memset(ts->basicBlockStore, 0, BASIC_BLOCK_STORE_SIZE * sizeof(BasicBlock));

  ts->write = TRUE;
}


void clearTranslationsByAddress(TranslationStore* ts, u32int address)
{
  bool executed = isExecBitSet(getActiveGuestContext(), address);
  if (!executed)
  {
    return;
  }

  u32int i = 0;
  /* we traverse the complete block translation store
   * looking for blocks containing this address
   * since there may be the case that part of a group block
   * falls in this range, and we cant remove a single part of a group block
   * inside the loop we unlink all current group-blocks. */
  for (i = 0; i < BASIC_BLOCK_STORE_SIZE; i++)
  {
    if (ts->basicBlockStore[i].type == GB_TYPE_ARM)
    {
      unlinkBlock(&ts->basicBlockStore[i], i);
    }
    u32int guestStart = (u32int)(ts->basicBlockStore[i].guestStart);
    u32int guestEnd = (u32int)(ts->basicBlockStore[i].guestEnd);
    if ((guestStart <= address) && (guestEnd >= address))
    {
      // STARFIX: remove all memset zero for naive memory allocator
      memset((void*)&ts->basicBlockStore[i], 0, sizeof(BasicBlock));
      ts->basicBlockStore[i].type = BB_TYPE_INVALID;
    }
  }
}


void clearTranslationsByAddressRange(TranslationStore* ts, u32int addressStart, u32int addressEnd)
{
  bool startSet = isExecBitSet(getActiveGuestContext(), addressStart);
  bool endSet = isExecBitSet(getActiveGuestContext(), addressEnd);
  bool set = startSet | endSet;
  if (!set)
  {
    // we have never cached any translations in this address range. nothing to clear.
    return;
  }
  u32int i = 0;
  /* we traverse the complete block translation store
   * looking for blocks matching this address range.
   * since there may be the case that part of a group block
   * falls in this range, and we cant remove a single part of a group block
   * inside the loop we unlink all current group-blocks. */
  for (i = 0; i < BASIC_BLOCK_STORE_SIZE; i++)
  {
    if (ts->basicBlockStore[i].type == GB_TYPE_ARM)
    {
      unlinkBlock(&ts->basicBlockStore[i], i);
    }
    u32int guestStart = (u32int)(ts->basicBlockStore[i].guestStart);
    u32int guestEnd = (u32int)(ts->basicBlockStore[i].guestEnd);
    if ( ((guestStart >= addressStart) && (guestStart <= addressEnd)) ||
         ((guestEnd >= addressStart) && (guestEnd <= addressEnd)) )
    {
      // STARFIX: remove all memset zero for naive memory allocator
      memset((void*)&ts->basicBlockStore[i], 0, sizeof(BasicBlock));
      ts->basicBlockStore[i].type = BB_TYPE_INVALID;
    }
  }
}

