#include "memFunctions.h"
#include "serial.h"

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
  serial_putstring("mallocInit(");
  serial_putint_nozeros(startAddr);
  serial_putstring(", ");
  serial_putint_nozeros(size);
  serial_putstring(");");
  serial_newline();
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
  serial_putstring("mallocBytes(");
  serial_putint_nozeros(size);
  serial_putstring(");");
  serial_newline();
#endif

  if ((size & 0x3) != 0)
  {
    serial_ERROR("mallocBytes not word aligned.");
  }

  if ((freePtr + size) >= heapEnd)
  {
    serial_ERROR("malloc out of heap space.");
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
  serial_putstring("Dumping malloc internal structures:");
  serial_putstring("***********************************");
  serial_newline();
  for (i = 0; i < nrOfChunksAllocd; i++)
  {
    serial_putstring("Chunk ");
    serial_putint_nozeros(i);
    serial_putstring(": prev = ");
    serial_putint((u32int)listPtr->prevChunk);
    serial_putstring("; next = ");
    serial_putint((u32int)listPtr->nextChunk);
    serial_newline();
    serial_putstring("Start address: ");
    serial_putint(listPtr->chunk.startAddress);
    serial_putstring("; Size: ");
    serial_putint(listPtr->chunk.size);
    serial_newline();
    serial_putstring("-----------------------------------");
    serial_newline();
    listPtr = listPtr->nextChunk;
  }
  serial_ERROR("done");
}

