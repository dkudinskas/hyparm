#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "common/memoryAllocator/allocator.h"
#include "common/memoryAllocator/legacy.h"


u32int heapStart;
u32int heapSize;
u32int heapEnd;

u32int freePtr;

u32int nrOfChunksAllocd;
memchunkListElem * chunkList;
memchunkListElem * chunkListRoot;


void dumpAllocatorInternals()
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

void initialiseAllocator(u32int startAddress, u32int bytes)
{
  DEBUG(MEMORY_ALLOCATOR, "mallocInit(%#.8x, %x);" EOL, startAddress, bytes);

  heapStart = startAddress;
  heapSize = bytes;
  heapEnd = startAddress + bytes;

  freePtr = heapStart;

  // root element...
  chunkListRoot = (memchunkListElem*)startAddress;
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
}

void uncheckedFree(void *ptr)
{
}

void *uncheckedMalloc(u32int size)
{
  DEBUG(MEMORY_ALLOCATOR, "mallocBytes: size %x" EOL, size);

  // Adjust size to be a multiple of 4 bytes
  size = (size + 3) & ~3;

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

void *uncheckedMemalign(u32int alignment, u32int size)
{
  DEBUG(MEMORY_ALLOCATOR, "uncheckedMemalign: size %#.8x alignment %#.8x" EOL, size, alignment);

  if ((freePtr + size) >= heapEnd)
  {
    // FIXME: this does not take alignment into account!
    DIE_NOW(0, "malloc out of heap space.");
  }

  chunkList->nextChunk = (memchunkListElem*)(((u32int)chunkList) + sizeof(memchunkListElem));

  DEBUG(MEMORY_ALLOCATOR, "uncheckedMemalign: freePtr %08x" EOL, freePtr);

  // need X bytes aligned at N bits
  u32int alignMask = ~(alignment - 1);
  // only adjust pointer if not aligned as requested!
  if (freePtr != (freePtr & alignMask))
  {
    freePtr = (freePtr & alignMask) + alignment;
  } 
  
  DEBUG(MEMORY_ALLOCATOR, "uncheckedMemalign: freePtr now %#.8x, alignMask %#.8x" EOL, freePtr, alignMask);

  memchunkListElem * tmp = chunkList;
  chunkList = chunkList->nextChunk;

  chunkList->prevChunk = tmp;

  chunkList->chunk.startAddress = freePtr;
  chunkList->chunk.size = size;
  nrOfChunksAllocd++;
 
  freePtr = freePtr + size;
  return (void*)chunkList->chunk.startAddress;
}

void *uncheckedRealloc(void *ptr, u32int size)
{
  size = (size + 3) & ~3;
  void *newPtr = uncheckedMalloc(size);
  size = (u32int)newPtr - (u32int)ptr < size ? ((u32int)newPtr - (u32int)ptr) : size;
  memcpy(newPtr, ptr, size);
  return newPtr;
}
