#include "common/debug.h"
#include "common/linker.h"
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
}


void mallocInit()
{
  u32int startAddr = RAM_XN_POOL_BEGIN;
  u32int size = RAM_XN_POOL_END - RAM_XN_POOL_BEGIN;

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

  // Adjust size to be a multiple of 4 bytes
  size = (size + 3) & ~0b11;

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
