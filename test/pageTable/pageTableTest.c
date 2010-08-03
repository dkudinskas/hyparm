#include <stdlib.h>
#include <stdio.h>
#include "pageTableTest.h"
#include "serial.h"
#include "pageTable.h"
#include "frameAllocator.h"

extern void freeAllMemory(void);

int main()
{
  createHypervisorPageTable();

  //tidy up the frameAllocator memory structures
  freeAllMemory();
  return 0;
}
