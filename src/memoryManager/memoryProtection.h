#ifndef __MEMORY_PROTECTION_H__
#define __MEMORY_PROTECTION_H__
#include "types.h"
#include "pageTable.h" // for ACCESS_TYPE enum

// uncomment me for memory protection debug: #define MEM_PROT_DBG

//function ptr definition
typedef void (*memProtPtr)(u32int, u32int);

/* List of pages in a protected address range */
struct pageList
{
  u32int pteAddr; //any addr in the pteAddr
  struct pageList* next;
  ACCESS_TYPE prevAccessBits;
};
typedef struct pageList PLIST;

/* linked list entry, holding address range we want to protect */
struct memProtectionEntry
{
  //Address range we want to protect
  u32int protStartAddr;
  u32int protEndAddr;
  //Address of page table entry boundaries
  u32int pteStartAddr;
  u32int pteEndAddr;
  memProtPtr ptr;
  struct memProtectionEntry* next;
  PLIST* plist;
};
typedef struct memProtectionEntry MPE;

struct memProtection
{
  MPE* first;
  u32int maxEntries;
};
typedef struct memProtection MEMPROT;

MEMPROT* initialiseMemoryProtection(void);
memProtPtr checkProtectionArray(u32int address);
u32int addProtection(u32int startAddr, u32int endAddr, memProtPtr ptr, ACCESS_TYPE protection);
u32int removeProtection(u32int startAddr);

// returns true if data abort to be delivered to guest
bool shouldDataAbort(bool privAccess, bool isWrite, u32int address);

// returns true if prefetch abort to be delivered to guest
bool shouldPrefetchAbort(u32int address);

#endif
