#ifndef __MEMORY_MANAGER__MEMORY_PROTECTION_H__
#define __MEMORY_MANAGER__MEMORY_PROTECTION_H__

#include "common/types.h"

#include "memoryManager/pageTable.h" // for AccessType enum

// uncomment me for memory protection debug: #define MEM_PROT_DBG

#define DACR_NO_ACCESS   0
#define DACR_CLIENT      1
#define DACR_RESERVED    2
#define DACR_MANAGER     3

void guestWriteProtect(u32int startAddress, u32int endAddress);

// returns true if data abort to be delivered to guest
bool shouldDataAbort(bool privAccess, bool isWrite, u32int address);

// returns true if prefetch abort to be delivered to guest
bool shouldPrefetchAbort(bool privAccess, u32int address);

#endif
