#ifndef __GUEST_MANAGER__TRANSLATION_CACHE_H__
#define __GUEST_MANAGER__TRANSLATION_CACHE_H__

#include "common/assert.h"
#include "common/debug.h"
#include "common/compiler.h"
#include "common/stddef.h"
#include "common/types.h"

#include "instructionEmu/decoder.h"


#ifdef CONFIG_BLOCK_COPY

#define TRANSLATION_CACHE_CODE_SIZE_B  4096
#define TRANSLATION_CACHE_META_SIZE_N   128

#else

#define TRANSLATION_CACHE_META_SIZE_N   256

#endif

#define TRANSLATION_CACHE_NUMBER_OF_BITMAPS       16
#define TRANSLATION_CACHE_MEMORY_PER_BITMAP       0x10000000
#define TRANSLATION_CACHE_MEMORY_PER_BITMAP_BIT   (TRANSLATION_CACHE_MEMORY_PER_BITMAP / 32) // should be 8 megabytes


enum metaCacheEntryType
{
  MCE_TYPE_INVALID = 0,
  MCE_TYPE_ARM,
  MCE_TYPE_THUMB
};


/* idea to make it
struct metaCacheEntry2
{
  u16int *start;
  struct
  {
    unsigned codeCacheIndex : 12; // based on halfword offset
    unsigned type : 2;
    unsigned codeCacheSize : 9;
    unsigned originalSize : 8;
  } bits;
  u32int hyperedInstruction;
  InstructionHandler handler;
};

COMPILE_TIME_ASSERT(sizeof(struct metaCacheEntry2) == 2*sizeof(u64int), __epic_fail);*/

struct guestContext;
struct metaCacheEntry;

// this looks useless but its not:
// now we can search for usage of the backpointer
// currently it is only used for clearing the cache when running out of space in C$
typedef struct codeCacheEntry
{
  u32int metaIndex;
  u32int codeStart;
} CodeCacheEntry;

typedef struct metaCacheEntry
{
  u32int startAddress;
  u32int endAddress;
  u32int hyperedInstruction;
  u8int type;
#ifdef CONFIG_BLOCK_COPY
  u16int codeSize;
  CodeCacheEntry *code; // pointer to code cache entry
#endif
  void *hdlFunct;
} MetaCacheEntry;


typedef struct translationCache
{
  u32int *codeCache;
  CodeCacheEntry *codeCacheNextEntry;
  u32int *codeCacheLastEntry;
  u32int *spillPage;
  MetaCacheEntry metaCache[TRANSLATION_CACHE_META_SIZE_N];
  u32int execBitMap[TRANSLATION_CACHE_NUMBER_OF_BITMAPS];
#ifdef CONFIG_BLOCK_CACHE_COLLISION_COUNTER
  u64int collisionCounter;
#endif
} TranslationCache;


void clearTranslationCache(TranslationCache *tc);
void clearTranslationCacheByAddress(TranslationCache *tc, u32int address);
void clearTranslationCacheByAddressRange(TranslationCache *tc, u32int startAddress, u32int endAddress);

void dumpMetaCacheEntry(MetaCacheEntry *entry);
void dumpMetaCacheEntryByIndex(TranslationCache *tc, u32int metaIndex);

__macro__ MetaCacheEntry *getMetaCacheEntry(TranslationCache *tc, u32int metaIndex, u32int startAddress);
__macro__ u32int getMetaCacheIndex(u32int startAddress);

void initialiseTranslationCache(struct guestContext *context);


#ifdef CONFIG_BLOCK_COPY

void addMetaCacheEntry(TranslationCache *tc, u32int index, MetaCacheEntry *entry);

/*
 * updateCodeCachePointer
 * MUST be called on a pointer before using that pointer to write to C$. ALWAYS use the returned address.
 */
u32int *updateCodeCachePointer(TranslationCache *tc, u32int *pointer);

#else

void addMetaCacheEntry(TranslationCache *tc, u32int index, u32int *start, u32int *end,
                       u32int hypInstruction, u32int type, void *hdlFunct);

#endif /* CONFIG_BLOCK_COPY */


__macro__ MetaCacheEntry *getMetaCacheEntry(TranslationCache *tc, u32int metaIndex, u32int startAddress)
{
  DEBUG(BLOCK_CACHE, "getMetaCacheEntry: index = %#x" EOL, metaIndex);

  MetaCacheEntry *entry = &tc->metaCache[metaIndex];
  return (entry->type != MCE_TYPE_INVALID && entry->startAddress == startAddress) ? entry : NULL;
}

// http://www.concentric.net/~Ttwang/tech/inthash.htm
// 32bit mix function
__macro__ u32int getMetaCacheIndex(u32int startAddress)
{
  startAddress = ~startAddress + (startAddress << 15); // key = (key << 15) - key - 1;
  startAddress = startAddress ^ (startAddress >> 12);
  startAddress = startAddress + (startAddress << 2);
  startAddress = startAddress ^ (startAddress >> 4);
  startAddress = startAddress * 2057; // key = (key + (key << 3)) + (key << 11);
  startAddress = startAddress ^ (startAddress >> 16);
  startAddress >>= 2;
  return (startAddress & (TRANSLATION_CACHE_META_SIZE_N - 1));
}

#endif /* __GUEST_MANAGER__TRANSLATION_CACHE_H__ */
