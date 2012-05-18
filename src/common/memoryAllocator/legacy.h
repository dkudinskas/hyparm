#ifndef __COMMON__MEMORY_ALLOCATOR__LEGACY_H__
#define __COMMON__MEMORY_ALLOCATOR__LEGACY_H__

/*
 * Private header for the legacy memory allocator.
 */


#include "common/types.h"

#define HIDDEN_RAM_START   0x8f000000
#define HIDDEN_RAM_SIZE    0x01000000 // 16 MB

struct allocatedChunk
{
  u32int startAddress;
  u32int size;
};
typedef struct allocatedChunk memchunk;


struct chunkLinkedListElement;
typedef struct chunkLinkedListElement memchunkListElem;


struct chunkLinkedListElement
{
  memchunk chunk;
  memchunkListElem* prevChunk;
  memchunkListElem* nextChunk;
};

#endif /* __COMMON__MEMORY_ALLOCATOR__LEGACY_H__ */
