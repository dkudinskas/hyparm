#include "common/debug.h"
#include "common/memFunctions.h"
#include "common/stddef.h"


u32int heapStart;
u32int heapSize;
u32int heapEnd;

u32int freePtr;

u32int nrOfChunksAllocd;
memchunkListElem * chunkList;
memchunkListElem * chunkListRoot;


void free(void *pointer)
{
  /*
   * TODO: implement free
   */
//  printf("free: pointer = %p (TODO)" EOL, pointer);
}


void mallocInit()
{
  u32int startAddr = HIDDEN_RAM_START;
  u32int size = HIDDEN_RAM_SIZE;

  DEBUG(MALLOC, "mallocInit(%#.8x, %x);" EOL, startAddr, size);

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
  chunkList->chunk.size = sizeof(memchunkListElem) * 1024;
  freePtr = freePtr + sizeof(memchunkListElem) * 1024;

  int i;
  for (i = 1; i < 1024; i++)
  {
    chunkList->nextChunk = (memchunkListElem*)(((u32int)chunkList) + sizeof(memchunkListElem));
    memchunkListElem * tmp = chunkList;
    chunkList = chunkList->nextChunk;
    chunkList->prevChunk = tmp;
    chunkList->chunk.startAddress = 0;
    chunkList->chunk.size = 0;
  }
  chunkList = chunkListRoot;

//  memset((void*)startAddr, 0, size);
}

/**
 * memory move
 */
void* memmove(void * dest,const void *src, u32int count)
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
void* memset(void * dest, u32int c, u32int count)
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




/* This version of memcpy assumes disjoint ptrs src, dst */
void *memcpy(void *dst, const void *src, u32int count)
{
  u32int i;
  char *dst_tmp = dst;
  const char *src_tmp = src;

  if (!((u32int)src & 0xC) && !((u32int)dst & 0xC))
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


void dumpMallocs()
{
  u32int i = 0;
  memchunkListElem * listPtr = chunkListRoot;
  printf("Dumping malloc internal structures:" EOL);
  printf("***********************************" EOL);
  for (i = 0; i < nrOfChunksAllocd; i++)
  {
    printf("Chunk %x: prev = %p; next = %p" EOL, i, listPtr->prevChunk, listPtr->nextChunk);
    printf("Start address: %#.8x; size %#x" EOL, listPtr->chunk.startAddress, listPtr->chunk.size);
    printf("-----------------------------------" EOL);
    listPtr = listPtr->nextChunk;
  }
}


void *mallocBytes(u32int size)
{
  DEBUG(MALLOC, "mallocBytes: size %x" EOL, size);

  if ((size & 0x3) != 0)
  {
    DIE_NOW(NULL, "mallocBytes not word aligned.");
  }

  if ((freePtr + size) >= heapEnd)
  {
    DIE_NOW(NULL, "malloc out of heap space.");
  }

  chunkList->nextChunk = (memchunkListElem*)(((u32int)chunkList) + sizeof(memchunkListElem));

  memchunkListElem * tmp = chunkList;
  chunkList = chunkList->nextChunk;

  chunkList->prevChunk = tmp;

  chunkList->chunk.startAddress = freePtr;
  chunkList->chunk.size = size;
  nrOfChunksAllocd++;

  freePtr = freePtr + size;
  return (void *)chunkList->chunk.startAddress;
}


void* mallocBytesWithAlign(u32int size, u32int alignBits)
{
  DEBUG(MALLOC, "mallocBytesWithAlign: size %x alingn bits %x" EOL, size, alignBits);

  if ((freePtr + size) >= heapEnd)
  {
    DIE_NOW(0, "malloc out of heap space.");
  }

  chunkList->nextChunk = (memchunkListElem*)(((u32int)chunkList) + sizeof(memchunkListElem));

  DEBUG(MALLOC, "mallocBytesWithAlign: freePtr %08x" EOL, freePtr);

  // need X bytes aligned at N bits
  u32int alignMask = ~((1 << alignBits) - 1);
  // only adjust pointer if not aligned as requested!
  if (freePtr != (freePtr & alignMask))
  {
    freePtr = (freePtr & alignMask) + (1 << alignBits);
  } 
  
  DEBUG(MALLOC, "mallocBytesWithAlign: freePtr now %08x, alignMask %08x" EOL, freePtr, alignMask);

  memchunkListElem * tmp = chunkList;
  chunkList = chunkList->nextChunk;

  chunkList->prevChunk = tmp;

  chunkList->chunk.startAddress = freePtr;
  chunkList->chunk.size = size;
  nrOfChunksAllocd++;
 
  freePtr = freePtr + size;
  return (void*)chunkList->chunk.startAddress;
}
