#include "frameAllocator.h"
#include "memoryConstants.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void addEntry(u32int* newEntry);
void freeAllMemory(void);

u32int arrayEntries=20;
u32int currentEntry=-1;
u32int* allocatedMemoryArray = 0;

//Allocate a single frame, returns 0 if failed, otherwise the address of the allocated frame
u32int* allocFrame(u8int domain)
{
  return allocMultipleFrames(1, domain);
}

//Attempt to allocate multiple contiguous frames
u32int* allocMultipleFrames(u32int numFrames, u8int domain)
{
  //we are going to align this, even if the hypervisor doesn't need it aligned
  u32int* addr;

  u32int chunkSize = FRAME_TABLE_CHUNK_SIZE * numFrames;

  //Worst case is the 4KB memory chunk is 1byte over the boundary => need 2xcunk size -1
  addr = (u32int*) malloc(chunkSize + (chunkSize -1));
  addEntry(addr);

  //get a 4KB aligned chunk of memory
  u32int* chunk;
  chunk = (u32int*)(((u32int)addr + chunkSize -1) & (u32int)~(chunkSize -1));

  return chunk;
}

void addEntry(u32int* newEntry)
{
  //If we don't have an array of entrys create one.
  if(0 == allocatedMemoryArray)
  {
    allocatedMemoryArray = malloc(sizeof(u32int) * arrayEntries);

    if(NULL == allocatedMemoryArray)
    {
      printf("Malloc Error. Exiting...\n");
      exit(1);
    }

    //zero array
    memset(allocatedMemoryArray,0,(arrayEntries * sizeof(u32int)));

  }

  currentEntry++;
  allocatedMemoryArray[currentEntry] = (u32int)newEntry;
}

/* Code to ensure no memory leaks */
void freeAllMemory()
{
  u32int* entry;
  int i;
  for(i=0; i <= currentEntry; i++)
  {
    //get the address at array entry i, cast it into  ptr
    entry = (u32int*)(allocatedMemoryArray[i]);

    if(entry !=0)
    {
      free(entry);
    }
  }
  free(allocatedMemoryArray);
}