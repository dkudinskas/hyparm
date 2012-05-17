#ifndef __MEMORY_MANAGER__PAGE_TABLE_INFO_H__
#define __MEMORY_MANAGER__PAGE_TABLE_INFO_H__

#include "common/compiler.h"
#include "common/types.h"

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


void addPageTableInfo(pageTableEntry* entry, u32int virtual, u32int physical, u32int mapped, bool host);
ptInfo* getPageTableInfo(pageTableEntry* firstLevelEntry);
void removePageTableInfo(pageTableEntry* firstLevelEntry, bool host);
void dumpPageTableInfo(void) __cold__;
void invalidatePageTableInfo(void);

#endif
