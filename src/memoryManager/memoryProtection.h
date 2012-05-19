#ifndef __MEMORY_MANAGER__MEMORY_PROTECTION_H__
#define __MEMORY_MANAGER__MEMORY_PROTECTION_H__

#include "common/types.h"

#include "guestManager/guestContext.h"

#include "memoryManager/pageTable.h" // for AccessType enum


// uncomment me for memory protection debug: #define MEM_PROT_DBG

#define DACR_NO_ACCESS   0
#define DACR_CLIENT      1
#define DACR_RESERVED    2
#define DACR_MANAGER     3

void guestWriteProtect(GCONTXT *gc, u32int startAddress, u32int endAddress);

void writeProtectRange(GCONTXT *gc, simpleEntry* pageTable, u32int start, u32int end);

// returns true if data abort to be delivered to guest
bool shouldDataAbort(GCONTXT *context, bool privAccess, bool isWrite, u32int address);

// returns true if prefetch abort to be delivered to guest
bool shouldPrefetchAbort(GCONTXT *context, bool privAccess, u32int address);

#endif
