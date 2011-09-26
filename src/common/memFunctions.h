#ifndef __COMMON__MEM_FUNCTIONS_H__
#define __COMMON__MEM_FUNCTIONS_H__

#include "common/types.h"


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
  memchunkListElem * prevChunk;
  memchunkListElem * nextChunk;
};



void mallocInit(u32int startAddr, u32int size);

u32int mallocBytes(u32int size);

void * memmove(void * dest,const void *src, u32int count);
void * memset(void * dest, u32int c, u32int count);
void * memcpy(void *dst, const void *src, u32int count);

void dumpMallocs(void);

#endif
