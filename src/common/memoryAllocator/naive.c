#include "common/stddef.h"
#include "common/stdlib.h"

#include "common/memoryAllocator/allocator.h"


static void *poolStart;
static void *poolEnd;


void initialiseAllocator(u32int startAddress, u32int bytes)
{
  poolStart = (void *)startAddress;
  poolEnd = poolStart + bytes;
}

void uncheckedFree(void *ptr)
{
}

void *uncheckedMalloc(u32int size)
{
  size = (size + 3) & ~3;
  void *ptr = poolStart;
  if ((ptr + size) < poolEnd)
  {
    poolStart += size;
    return ptr;
  }
  return NULL;
}

void *uncheckedMemalign(u32int alignment, u32int size)
{
  size = (size + 3) & ~3;
  void *ptr = (void *)(((u32int)poolStart + (alignment - 1)) & ~(alignment - 1));
  void *next = ptr + size;
  if (next < poolEnd)
  {
    poolStart = next;
    return ptr;
  }
  return NULL;
}

void *uncheckedRealloc(void *ptr, u32int size)
{
  size = (size + 3) & ~3;
  void *newPtr = uncheckedMalloc(size);
  size = (u32int)newPtr - (u32int)ptr < size ? ((u32int)newPtr - (u32int)ptr) : size;
  memcpy(newPtr, ptr, size);
  return newPtr;
}
