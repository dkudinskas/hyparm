#ifndef __COMMON__MEM_FUNCTIONS_H__
#define __COMMON__MEM_FUNCTIONS_H__

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


void mallocInit(void);

void* memmove(void * dest,const void *src, u32int count);
void* memset(void * dest, u32int c, u32int count);
void* memcpy(void *dst, const void *src, u32int count);

void dumpMallocs(void);

void* mallocBytes(u32int size);
void* mallocBytesWithAlign(u32int size, u32int alignBits);

void free(void *pointer);

#endif
