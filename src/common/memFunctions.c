#include "common/debug.h"
#include "common/memFunctions.h"



u32int heapStart;
u32int heapSize;
u32int heapEnd;

u32int freePtr;

u32int nrOfChunksAllocd;
memchunkListElem * chunkList;
memchunkListElem * chunkListRoot;


void mallocInit(u32int startAddr, u32int size)
{
#ifdef MALLOC_DEBUG
  DEBUG_STRING("mallocInit(");
  DEBUG_INT_NOZEROS(startAddr);
  DEBUG_STRING(", ");
  DEBUG_INT_NOZEROS(size);
  DEBUG_STRING(");");
  DEBUG_NEWLINE();
#endif

  heapStart = startAddr;
  heapSize = size;
  heapEnd = startAddr + size;

  freePtr = heapStart;

  // root element...
  chunkListRoot = (memchunkListElem*)startAddr;
  // chunkList assigned the root linked list element
  chunkList = chunkListRoot;

  nrOfChunksAllocd = 1;
  chunkList->prevChunk = 0;
  chunkList->nextChunk = 0;
  chunkList->chunk.startAddress = freePtr;
  chunkList->chunk.size = sizeof(memchunkListElem)*256;
  freePtr = freePtr + sizeof(memchunkListElem)*256;
  

  int i = 0;
  for (i = 1; i < 256; i++)
  {
    chunkList->nextChunk = (memchunkListElem*)(((u32int)chunkList) + sizeof(memchunkListElem));
    memchunkListElem * tmp = chunkList;
    chunkList = chunkList->nextChunk;
    chunkList->prevChunk = tmp;
    chunkList->chunk.startAddress = 0;
    chunkList->chunk.size = 0;
  }
  chunkList = chunkListRoot;
}

/**
 * memory move
 */
void * memmove(void * dest,const void *src, u32int count)
{
  char *tmp, *s;

  if (dest <= src)
  {
    tmp = (char *) dest;
    s = (char *) src;
    while (count--)
    {
      *tmp++ = *s++;
    }
  }
  else
  {
    tmp = (char *) dest + count;
    s = (char *) src + count;
    while (count--)
    {
      *--tmp = *--s;
    }
  }
  return dest;
}

/**
* memory set
*/
void * memset(void * dest, u32int c, u32int count)
{
  //This tests to see if dest is word aligned & that count is a multiple of 4
  if (((u32int)dest & 0x3F) && ((count & 0x3) == count))
  {
    //lets optimize to store words instead of bytes
    u32int* d;
    d = (u32int*) dest;

    //turn the byte pattern into a word pattern
    u32int wordPattern = (c || (c << 8) || (c << 16) || (c << 24));

    count = count >> 2; //count / 4

    while(count--)
    {
      *d = wordPattern;
      d++;
    }
  }
  else
  {
    //Standard bytewise memset
    char* d;
    d = (char*) dest;

    while(count--)
    {
      *d = c;
      d++;
    }
  }

  return dest;
}

u32int mallocBytes(u32int size)
{
#ifdef MALLOC_DEBUG
  DEBUG_STRING("mallocBytes(");
  DEBUG_INT_NOZEROS(size);
  DEBUG_STRING(");");
  DEBUG_NEWLINE();
#endif

  if ((size & 0x3) != 0)
  {
    DIE_NOW(0, "mallocBytes not word aligned.");
  }

  if ((freePtr + size) >= heapEnd)
  {
    DIE_NOW(0, "malloc out of heap space.");
  }

  chunkList->nextChunk = (memchunkListElem*)(((u32int)chunkList) + sizeof(memchunkListElem));

  memchunkListElem * tmp = chunkList;
  chunkList = chunkList->nextChunk;

  chunkList->prevChunk = tmp;

  chunkList->chunk.startAddress = freePtr;
  chunkList->chunk.size = size;
  nrOfChunksAllocd++;
 
  freePtr = freePtr + size;
  return chunkList->chunk.startAddress;
}

void dumpMallocs()
{
  u32int i = 0;
  memchunkListElem * listPtr = chunkListRoot;
  DEBUG_STRING("Dumping malloc internal structures:");
  DEBUG_STRING("***********************************");
  DEBUG_NEWLINE();
  for (i = 0; i < nrOfChunksAllocd; i++)
  {
    DEBUG_STRING("Chunk ");
    DEBUG_INT_NOZEROS(i);
    DEBUG_STRING(": prev = ");
    DEBUG_INT((u32int)listPtr->prevChunk);
    DEBUG_STRING("; next = ");
    DEBUG_INT((u32int)listPtr->nextChunk);
    DEBUG_NEWLINE();
    DEBUG_STRING("Start address: ");
    DEBUG_INT(listPtr->chunk.startAddress);
    DEBUG_STRING("; Size: ");
    DEBUG_INT(listPtr->chunk.size);
    DEBUG_NEWLINE();
    DEBUG_STRING("-----------------------------------");
    DEBUG_NEWLINE();
    listPtr = listPtr->nextChunk;
  }
  DIE_NOW(0, "done");
}

/* This version of memcpy assumes disjoint ptrs src, dst */
void *memcpy(void *dst, const void *src, u32int count)
{
  int i;
  char *dst_tmp = dst;
  const char *src_tmp = src;

  if (!((unsigned int)src & 0xC) && !((unsigned int)dst & 0xC))
  {
    //word aligned so we can safely do word copies
    for (i=0; i < count; i+=4)
    {
      if (i + 3 > count - 1)
        break; //don't copy too much

      *(u32int *)dst_tmp = *(u32int *)src_tmp;
      dst_tmp += 4;
      src_tmp += 4;
    }
    if (i < count - 1)
    {
      for (; i < count; i++)
      {
        *dst_tmp = *src_tmp;
        dst_tmp++;
        src_tmp++;
      }
    }
  }
  else
  {
    //generic version
    for (i=0; i < count; i++)
      dst_tmp[i] = src_tmp[i];
  }
  return dst;
}

