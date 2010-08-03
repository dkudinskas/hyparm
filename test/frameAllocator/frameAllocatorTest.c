#include "frameAllocatorTest.h"
#include "serial.h"
#include "frameAllocator.h"
#include "memoryConstants.h"
#include <stdlib.h>
#include <stdio.h>

//Pass as gcc flags -D FRAME_TABLE_ALLOC_TEST

#define FRAME_TABLE_ENTRIES (TOTAL_MACHINE_RAM / FRAME_TABLE_CHUNK_SIZE)

u32int* frameTableAddr;
void testFrameTable(void);
bool testsPassed;

const u32int startPhyAddr = MEMORY_START_ADDR;
const u32int endPhyAddr = HYPERVISOR_START_ADDR + (TOTAL_MACHINE_RAM/4);

int main(void)
{
  printf("Frame Allocator Tests\n");
  serial_putstring("Serial putstring method working");
  serial_newline();


  /* An ifdef present in frameAllocator.c allows us to set the frameTable address
   * So we can test it properly
  */

  u32int* memPtr = (u32int*)malloc(FRAME_TABLE_ENTRIES + 4096);

  //put frameTable on 4KB boundary
  frameTableAddr = (u32int*)(((u32int)memPtr + 4095) & ~0xFFF);
  printf("Frame table 4KB aligned at 0x%08x\n", (u32int)frameTableAddr);

  initialiseFrameTable();

  testsPassed = TRUE; //assume passed to begin with

  testFrameTable();

  if(testsPassed)
  {
    printf("ALL TESTS PASSED\n");
  }
  else
  {
    printf("ONE OR MORE TESTS FAILED\n");
  }

  free(memPtr);
  return 0;
}

u32int* getFrameAllocatorTestAddress(void)
{
  return frameTableAddr;
}

void dumpOccupiedMem()
{
  u32int addr = startPhyAddr;
  while(addr <= endPhyAddr)
  {
    if(isFrameFree(addr))
    {
      printf("4KB chunk starting Addr: 0x%08x is occupied\n", addr);
    }
    addr += FRAME_TABLE_CHUNK_SIZE;
  }
  printf("\n");
}

u32int* newFrame(u8int domain, bool testShouldPass)
{
  u32int* frameAllocated;

  frameAllocated = allocFrame(domain);

  if(0 == frameAllocated)
  {
    //not allocated a frame
    if(testShouldPass)
    {
      //The test was designed to fail
      testsPassed=FALSE;
      printf("\tFAIL: frame not allocated. return status: 0x%08x\n", (u32int)frameAllocated);
    }
    else
    {
      printf("\tPASS: frame correctly not allocated, return status %d\n", (u32int)frameAllocated);
    }
  }
  else
  {
    //valid address returned
    if(testShouldPass)
    {
      printf("\tPASS: Frame allocated, addr: 0x%08x\n", (u32int)frameAllocated);
    }
    else
    {
      testsPassed=FALSE;
      printf("\tFAIL: Frame incorrectly allocated, addr: 0x%08x\n", (u32int)frameAllocated);
    }
  }

  return frameAllocated;
}

void testFrameTable()
{
  u32int domain, numFrames, result;
  u32int* frameAllocated;

  printf("INFO: Check Hypervisor space has been mapped\n");
  dumpOccupiedMem();

  printf("TEST: Clear whole table\n");
  result = freeMultipleFrames(startPhyAddr, endPhyAddr);
  if(0 == result)
  {
    printf("\tPASS\n");
  }
  else
  {
    testsPassed=FALSE;
    printf("\tFAIL\n");
  }

  printf("INFO: Dump all entries, should display nothing\n");
  dumpOccupiedMem();

  domain = 0;
  printf("TEST: Lets allocate a frame for domain %d\n", domain);
  frameAllocated = newFrame(domain, TRUE);

  printf("INFO: Check allocated frames, should be 1\n");
  dumpOccupiedMem();

  domain = 4;
  printf("TEST: Attempt to allocate a frame to an out of range domain (%d)\n", domain);
  frameAllocated = newFrame(domain, FALSE);

  domain =1;
  printf("TEST: Allocate a frame to domain %d\n", domain);
  frameAllocated = newFrame(domain, TRUE);

  domain =2;
  printf("TEST: Allocate a frame to domain %d\n", domain);
  frameAllocated = newFrame(domain, TRUE);

  domain =3;
  printf("TEST: Allocate a frame to domain %d\n", domain);
  frameAllocated = newFrame(domain, TRUE);

  domain =3;
  printf("TEST: Allocate a frame to domain %d\n", domain);
  frameAllocated = newFrame(domain, TRUE);
  domain =3;
  printf("TEST: Allocate a frame to domain %d\n", domain);
  frameAllocated = newFrame(domain, TRUE);
  domain =3;
  printf("TEST: Allocate a frame to domain %d\n", domain);
  frameAllocated = newFrame(domain, TRUE);
  domain =3;
  printf("TEST: Allocate a frame to domain %d\n", domain);
  frameAllocated = newFrame(domain, TRUE);

  printf("TEST: Check allocated frames.\n");
  dumpOccupiedMem();

  printf("Attempt to allocate the frame we have just been given again\n");
  frameAllocated = allocFrameAddr((u32int)frameAllocated);

  if(!frameAllocated)
  {
    printf("\tPASS: Frame correctly not re-allocated\n");
  }
  else
  {
    testsPassed=FALSE;
    printf("\tFAIL: Frame re-allocated");
  }

  printf("INFO: clearing table\n");
  freeMultipleFrames(startPhyAddr, endPhyAddr);

  u32int address = 0x87654321;
  printf("TEST: Attempt to allocate frame addr, not on 4KB boundary addr: 0x%08x\n", address);
  frameAllocated = allocFrameAddr(address);
  if(0 == frameAllocated)
  {
    testsPassed=FALSE;
    printf("\tFAIL: Frame not allocated.");
  }
  else
  {
    printf("\tPASS: Frame allocated");
  }

  printf("INFO: Check allocated frame.\n");
  dumpOccupiedMem();

  printf("INFO: clearing all allocated frames.\n");
  freeMultipleFrames(startPhyAddr, endPhyAddr);

  domain = 0;
  numFrames = 1;
  printf("TEST: Attempt to allocate %d contiguous frames from domain %d\n", numFrames, domain);
  frameAllocated = allocMultipleFrames(numFrames,domain);
  if(0 == frameAllocated)
  {
    testsPassed = FALSE;
    printf("\tFAIL\n");
  }
  else
  {
     printf("\tPASS\n");
  }

  printf("INFO: Check for %d contiguous allocated frames.\n", numFrames);
  dumpOccupiedMem();

  domain = 0;
  numFrames = 2;
  printf("TEST: Attempt to allocate %d contiguous frames from domain %d\n", numFrames, domain);
  frameAllocated = allocMultipleFrames(numFrames,domain);
  if(0 == frameAllocated)
  {
    testsPassed = FALSE;
    printf("\tFAIL\n");
  }
  else
  {
    printf("\tPASS\n");
  }

  printf("INFO: Check for %d new contiguous allocated frames.\n", numFrames);
  dumpOccupiedMem();

  domain = 0;
  numFrames = 16;
  printf("TEST: %d frames for domain %d r\n", numFrames, domain);
  frameAllocated = allocMultipleFrames(numFrames ,domain);
  if(0 == frameAllocated)
  {
    testsPassed = FALSE;
    printf("\tFAIL\n");
  }
  else
  {
    printf("\tPASS\n");
  }

  printf("INFO: Check for %d new allocated frames.\n", numFrames);
  dumpOccupiedMem();

  domain = 0;
  numFrames = 32;
  printf("TEST %d frames for the domain %d\n", numFrames, domain);
  frameAllocated = allocMultipleFrames(numFrames, domain);
  if(0 == frameAllocated)
  {
    testsPassed = FALSE;
    printf("\tFAIL\n");
  }
  else
  {
    printf("\tPASS\n");
  }

  printf("INFO: Check for %d allocated frames.\n", numFrames);
  dumpOccupiedMem();

  /*
  * Current Implementation of Frame Allocator can't alloc more than 32 (128KB) of memory


  printf("TEST: %d frames for the hypervisor\n", numFrames, domain);
  frameAllocated = allocMultipleFrames(33,0);
  if(0 == frameAllocated)
  {
    testsPassed = FALSE;
    printf("\tFAIL\n");
  }
  else
  {
    printf("\tPASS\n");
  }

  printf("Check for %d new allocated frames.\n", numFrames);
  dumpOccupiedMem();
  */
}
