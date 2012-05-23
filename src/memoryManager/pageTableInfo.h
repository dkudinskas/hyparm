#ifndef __MEMORY_MANAGER__PAGE_TABLE_INFO_H__
#define __MEMORY_MANAGER__PAGE_TABLE_INFO_H__

#include "common/compiler.h"
#include "common/types.h"

#include "guestManager/types.h"

#include "memoryManager/pageTable.h"


struct PageTableMetaData
{
  pageTableEntry *firstLevelEntry;
  u32int virtAddr;
  u32int physAddr;
  u32int mappedMegabyte;
  bool host;
  struct PageTableMetaData *nextEntry;
};
typedef struct PageTableMetaData ptInfo;


void addPageTableInfo(GCONTXT *context, pageTableEntry* entry, u32int virtual, u32int physical, u32int mapped, bool host);
ptInfo* getPageTableInfo(GCONTXT *context, pageTableEntry* firstLevelEntry);
void removePageTableInfo(GCONTXT *context, pageTableEntry* firstLevelEntry, bool host);
void dumpPageTableInfo(GCONTXT *context) __cold__;
void invalidatePageTableInfo(GCONTXT *context);

#endif
