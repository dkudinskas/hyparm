#include "common/assert.h"
#include "common/bit.h"
#include "common/debug.h"
#include "common/linker.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "cpuArch/armv7.h"
#include "cpuArch/constants.h"

#include "guestManager/guestContext.h"

#include "memoryManager/addressing.h"
#include "memoryManager/memoryConstants.h"
#include "memoryManager/mmu.h"
#include "memoryManager/pageTable.h"
#include "memoryManager/shadowMap.h"


/**
 * allocate space for a new level 1 base page table
 **/
simpleEntry *newLevelOnePageTable()
{
  simpleEntry *pageTable = memalign(1 << PT1_ALIGN_BITS, PT1_SIZE);
  if (pageTable == NULL)
  {
    DIE_NOW(NULL, "failed to allocate L1 page table");
  }
  ASSERT(isAlignedToMask(pageTable, PT1_ALIGN_MASK), "new L1 page table is not correctly aligned");
  memset(pageTable, 0, PT1_SIZE);
  DEBUG(MM_PAGE_TABLES, "newLevelOnePageTable: Page Table base addr: %p" EOL, pageTable);
  return pageTable;
}

/**
 * allocate space for a new level 2 base page table
 **/
u32int *newLevelTwoPageTable()
{
  u32int *pageTable = memalign(1 << PT2_ALIGN_BITS, PT2_SIZE);
  if (pageTable == NULL)
  {
    DIE_NOW(NULL, "failed to allocate L2 page table");
  }
  ASSERT(isAlignedToMask(pageTable, PT2_ALIGN_MASK), "new L2 page table is not correctly aligned");
  memset(pageTable, 0, PT2_SIZE);
  DEBUG(MM_PAGE_TABLES, "newLevelTwoPageTable: Page Table base addr: %p" EOL, pageTable);
  return pageTable;
}


/**
 * frees an allocated level two page table and removes the corresponding
 * meta data information from the list. only called for 2nd level SHADOW page
 * tables. we can't delete guest page tables, not our memory!
 **/
void deleteLevelTwoPageTable(pageTableEntry *pageTable)
{
  GCONTXT *context = getGuestContext();
  DEBUG(MM_PAGE_TABLES, "deleteLevelTwoPageTable: page table entry %#.8x @ %p" EOL,
        *(u32int *)pageTable, pageTable);
  // this can only be called on shadow second level page tables
  ptInfo *head = context->pageTables->sptInfo;
  ptInfo *prev = 0;
  while (head != NULL)
  {
    if ((head->firstLevelEntry == pageTable) && (*(u32int *)head->firstLevelEntry == *(u32int *)pageTable))
    {
      head->firstLevelEntry = NULL;
      head->physAddr = 0;
      free((void *)head->virtAddr);
      head->virtAddr = 0;
      
      // remember a pointer to this doomed entry
      ptInfo *tempPtr = head;
      head = head->nextEntry;
      free(tempPtr);
      
      // all thats left is to link previous entry to next entry (if there was a previous entry!)
      if (prev != NULL)
      {
        prev->nextEntry = head;
      }
      else
      {
        // removing first entry from the list
        context->pageTables->sptInfo = head;
      }
      return;
    }
    else
    {
      prev = head;
      head = head->nextEntry;
    }
  } // while ends
}

u32int mapRange(simpleEntry *pageTable, u32int virtualStartAddress, u32int physicalStartAddress,
                u32int physicalEndAddress, u8int domain, u8int accessBits, bool cacheable,
                bool bufferable, u8int regionAttributes, bool executeNever)
{
  ASSERT(isAlignedToMaskN(physicalStartAddress, SMALL_PAGE_MASK), "bad alignment");
  /*
   * Construct a conservative, linear mapping for the specified address range.
   */
  while (physicalStartAddress < physicalEndAddress)
  {
    if (isAlignedToMaskN(physicalStartAddress, SECTION_MASK)
        && (physicalEndAddress - physicalStartAddress) >= SECTION_SIZE)
    {
      /* 1 MB, map a section */
      mapSection(pageTable, virtualStartAddress, physicalStartAddress, domain, accessBits,
                 cacheable, bufferable, regionAttributes, executeNever);
      physicalStartAddress += SECTION_SIZE;
      virtualStartAddress += SECTION_SIZE;
    }
    else
    {
      /* 4 kB, map a small page -- may involve creation of an L2 table */
      mapSmallPage(pageTable, virtualStartAddress, physicalStartAddress, domain, accessBits,
                   cacheable, bufferable, regionAttributes, executeNever);
      physicalStartAddress += SMALL_PAGE_SIZE;
      virtualStartAddress += SMALL_PAGE_SIZE;
    }
  }
  /*
   * Return the last mapped *physical* address (useful to check for overlaps).
   */
  return physicalStartAddress;
}

/**
 * Map hypervisor into given base page table in sections, 1-2-1 VA to PA
 **/
void mapHypervisorMemory(simpleEntry *pageTable)
{
  DEBUG(MM_PAGE_TABLES, "mapHypervisorMemory in page table %p" EOL, pageTable);

  u32int startAddr = HYPERVISOR_BEGIN_ADDRESS;
  u32int endAddr = MEMORY_END_ADDR;

  for (; startAddr < endAddr; startAddr += SECTION_SIZE)
  {
    // @ hypervisor domain, all acces bits, cachable, not bufferable, tex 0
    mapSection(pageTable, startAddr, startAddr, HYPERVISOR_ACCESS_DOMAIN,
               HYPERVISOR_ACCESS_BITS, TRUE, FALSE, 0, FALSE);
  }
}


/**
 * Add a section mapping of given virtual to physical address
 * into the given base page table 
 **/
void mapSection(simpleEntry *pageTable, u32int virtAddr, u32int physical, u8int domain,
                u8int accessBits, bool c, bool b, u8int tex, bool executeNever)
{
  DEBUG(MM_PAGE_TABLES, "mapSection: Virtual Addr: %#.8x, physical addr: %#.8x" EOL, virtAddr,
        physical);

  // check what is in the page table at the required index
  simpleEntry *firstLevelEntry = getEntryFirst(pageTable, virtAddr);
  switch(firstLevelEntry->type)
  {
    case SECTION:
    case FAULT:
    {
      addSectionEntry((sectionEntry *)firstLevelEntry,
                      physical, domain, accessBits, c, b, tex, executeNever);
      break;
    }
    case PAGE_TABLE:
    {
      printf("mapSection error: VA: %#.8x, PA: %#.8x, oldEntry %#.8x @ %p" EOL, virtAddr, physical,
             *(u32int *)firstLevelEntry, firstLevelEntry);
      DIE_NOW(NULL, "adding section entry on top of existing page table entry");
      break;
    }
    case RESERVED:
    default:
    {
      printf("mapSection error: Virtual Addr: %#.8x, physical addr: %#.8x" EOL, virtAddr, physical);
      DIE_NOW(NULL, "adding section entry on top of existing reserved entry");
    }
  }//switch
}


/**
 * Add a small page mapping of given virtual to physical address
 * into a second page table 
 **/
void mapSmallPage(simpleEntry *pageTable, u32int virtAddr, u32int physical,
                u8int domain, u8int accessBits, u8int c, u8int b, u8int tex, u8int xn)
{
  DEBUG(MM_PAGE_TABLES, "mapSmallPage: Virtual %#.8x, physical %#.8x, dom: %x, AP: %x, c: %x, "
        "b: %x, tex: %x, xn: %x" EOL, virtAddr, physical, domain, accessBits, c, b, tex, xn);

  // First check 1st Level page table entry
  simpleEntry* first = getEntryFirst(pageTable, virtAddr);
  DEBUG(MM_PAGE_TABLES, "mapSmallPage: first entry %#.8x @ %p" EOL, *(u32int *)first, first);
  switch(first->type)
  {
    case FAULT:
    {
      DEBUG(MM_PAGE_TABLES, "mapSmallPage: entry for given VA is FAULT. Creating new" EOL);
      // we need a new second level page table. This gives a virtual address allocated
      u32int *vAddr = newLevelTwoPageTable();
      // need to get the physical address (assume 1:1 mapping will follow in case MMU is disabled)
      u32int pAddr = isMmuEnabled() ? getPhysicalAddress(pageTable, (u32int)vAddr) : (u32int)vAddr;
      DEBUG(MM_PAGE_TABLES, "mapSmallPage: PT VA %p PA %#.8x" EOL, vAddr, pAddr);
      // store metadata
      addPageTableEntry((pageTableEntry*)first, (u32int)pAddr, domain);
      u32int mapped = ((u32int)first - (u32int)pageTable) << 18;
      addPageTableInfo((pageTableEntry*)first, (u32int)vAddr, pAddr, mapped, TRUE);
      break;
    }
    case PAGE_TABLE:
    {
      // if existing mapping is pageTable type, we're ok already
      DEBUG(MM_PAGE_TABLES, "mapSmallPage: entry for given VA is pageTable. Correct, first time" EOL);
      break;
    }
    case SECTION:
    {
      printf("mapSmallPage: Virtual %#.8x, physical %#.8x, dom: %x, AP: %x, c: %x, b: %x, tex: %x, xn: %x" EOL,
             virtAddr, physical, domain, accessBits, c, b, tex, xn);
      printf("mapSmallPage: first entry %#.8x @ %p" EOL, *(u32int*)first, first);
      DIE_NOW(NULL, "adding page table entry on top of existing section entry");
      break;
    }
    case RESERVED:
    default:
    {
      DIE_NOW(NULL, "adding page table entry on top of existing reserved entry");
    }
  }//switch

  // At this point we know its a 2nd level page table entry 
  simpleEntry *second = getEntrySecond((pageTableEntry *)first, virtAddr);
  DEBUG(MM_PAGE_TABLES, "mapSmallPage: 2nd lvl entry @ %p = %#.8x" EOL, second, *(u32int *)second);
  // Again, we need to check the existing entry
  if (second->type == FAULT)
  {
    DEBUG(MM_PAGE_TABLES, "mapSmallPage: 2nd Level FAULT. Creating new small entry" EOL);
    addSmallPageEntry((smallPageEntry*)second, physical, accessBits, c, b, tex, xn);
  }
  else //Existing small or large page
  {
    printf("mapSmallPage: Virtual %#.8x, physical %#.8x, dom: %x, AP: %x, c: %x, b: %x, tex: %x, xn: %x" EOL,
           virtAddr, physical, domain, accessBits, c, b, tex, xn);
    printf("mapSmallPage: first entry %#.8x @ %p" EOL, *(u32int*)first, first);
    printf("mapSmallPage: 2nd lvl entry @ %#.8x = %#.8x" EOL, (u32int)second, *(u32int*)second);
    DIE_NOW(NULL, "adding over existing entry");
  }
}


/**
 * adds a section entry at a given place in the first level page table
 **/
void addSectionEntry(sectionEntry *sectionEntryPtr, u32int physAddr, u8int domain,
                     u8int accessBits, bool cacheable, bool bufferable, u8int tex, bool executeNever)
{
  sectionEntryPtr->addr = (physAddr >> 20);
  sectionEntryPtr->type = SECTION;
  sectionEntryPtr->c = cacheable  ? 1:0;
  sectionEntryPtr->b = bufferable ? 1:0;
  sectionEntryPtr->xn = executeNever ? 1 : 0;
  sectionEntryPtr->domain = domain;
  sectionEntryPtr->imp = 0; //currently unused
  sectionEntryPtr->ap10 = accessBits & 0x3;
  sectionEntryPtr->tex = (tex & 0x7);
  sectionEntryPtr->ap2 = accessBits >> 2;
  sectionEntryPtr->s = 0;
  sectionEntryPtr->nG = 0;
  sectionEntryPtr->superSection = 0; // 0 for normal section (1 for supersection)
  sectionEntryPtr->ns = 1; // non secure memory?
  DEBUG(MM_PAGE_TABLES, "addSectionEntry: Section entry written: %#.8x @ %p" EOL,
        *(u32int *)sectionEntryPtr, sectionEntryPtr);
}


/**
 * adds a small page entry at a given place in the second level page table
 **/
void addSmallPageEntry(smallPageEntry* smallPageEntryPtr, u32int physical,
        u8int accessBits, u8int cacheable, u8int bufferable, u8int tex, u8int xn)
{
  smallPageEntryPtr->addr = (physical >> 12);
  smallPageEntryPtr->nG = 0;
  smallPageEntryPtr->s = 0;
  smallPageEntryPtr->ap2 = accessBits >> 2;
  smallPageEntryPtr->tex = (tex & 0x7);
  smallPageEntryPtr->ap10 = accessBits & 0x3;
  smallPageEntryPtr->c = cacheable  ? 1:0;
  smallPageEntryPtr->b = bufferable ? 1:0;
  smallPageEntryPtr->type = 1;
  smallPageEntryPtr->xn = xn;
  DEBUG(MM_PAGE_TABLES, "addSmallPageEntry: Small descriptor written: %#.8x @ %p" EOL,
        *(u32int *)smallPageEntryPtr, smallPageEntryPtr);
}


/**
 * Allocates memory for a new second level page table
 * adds a new page table entry to a given place in a base page table
 */
void addPageTableEntry(pageTableEntry* pageTableEntryPtr, u32int physical, u8int domain)
{
  pageTableEntryPtr->type = PAGE_TABLE;
  pageTableEntryPtr->sbz2 = 0;
  pageTableEntryPtr->ns = 0;
  pageTableEntryPtr->sbz = 0;
  pageTableEntryPtr->domain = domain;
  pageTableEntryPtr->imp = 0;
  pageTableEntryPtr->addr = (physical >> 10);
  DEBUG(MM_PAGE_TABLES, "addPageTableEntry: Written entry at %p value %#.8x" EOL,
        pageTableEntryPtr, *(u32int *)pageTableEntryPtr);
}


/**
 * Given a virtual address, retrieves the underlying physical address
 **/
u32int getPhysicalAddress(simpleEntry* pageTable, u32int virtAddr)
{
  DEBUG(MM_PAGE_TABLES, "getPhysicalAddress for VA %#.8x in PT @ %p" EOL, virtAddr, pageTable);
  simpleEntry* entryFirst = getEntryFirst(pageTable, virtAddr);
  if (entryFirst->type == FAULT)
  {
    // may not be shadow mapped? try it.
    if (!shadowMap(virtAddr))
    {
      printf("getPhysicalAddress for VA %#.8x in PT @ %#.8x" EOL, virtAddr, (u32int)pageTable);
      DIE_NOW(NULL, "failed to shadow map");
    }
  }
  switch(entryFirst->type)
  {
    case FAULT: //fall through, no break!
    {
      DIE_NOW(NULL, "fault entry in page table");
      break;
    }
    case SECTION:
    {
      sectionEntry* section = (sectionEntry*)entryFirst;
      if(section->superSection)
      {
        printf("getPhysicalAddress for VA %#.8x in PT @ %#.8x" EOL, virtAddr, (u32int)pageTable);
        printf("getPhysicalAddress: PT1 entry %#.8x @ %p" EOL, *(u32int*)section, section);
        DIE_NOW(NULL, "supersection case unimplemented");
      }
      else
      {
        return (section->addr << 20) | (virtAddr & ~SECTION_MASK);
      }
      break;
    }
    case PAGE_TABLE:
    {
      simpleEntry* entrySecond = getEntrySecond((pageTableEntry*)entryFirst, virtAddr);
      
      switch(entrySecond->type)
      {
        case LARGE_PAGE:
        {
          DIE_NOW(NULL, "Large page case unimplemented");
        }
        // may not be shadow mapped? try it.
        case FAULT:
        {
          if (!shadowMap(virtAddr))
          {
            printf("getPhysicalAddress for VA %#.8x in PT @ %#.8x" EOL, virtAddr, (u32int)pageTable);
            DIE_NOW(NULL, "failed to shadow map, lvl2");
          }
        }
        case SMALL_PAGE: //fall through
        case SMALL_PAGE_3:
        {
          smallPageEntry* smallPage = (smallPageEntry*)entrySecond;
          return (smallPage->addr << 12) | (virtAddr & ~SMALL_PAGE_MASK);
        }
        default:
        {
          printf("getPhysicalAddress for VA %#.8x in PT @ %#.8x" EOL, virtAddr, (u32int)pageTable);
          DIE_NOW(NULL, "fault entry in page table");
        }
      }
      break;
    }
    case RESERVED: //fall through
    {
      printf("getPhysicalAddress for VA %#.8x in PT @ %#.8x" EOL, virtAddr, (u32int)pageTable);
      DIE_NOW(NULL, "reserved entry in page table");
    }
  }
  // compiler happy
  return 0;
}


/**
 * extract first level page table entry for a given virtual address
 **/
simpleEntry* getEntryFirst(simpleEntry* pageTable, u32int virtAddr)
{
  DEBUG(MM_PAGE_TABLES, "getEntryFirst: virtual address %#.8x" EOL, virtAddr);
  // 1st level page table index is the top 12 bits of the virtual address
  u32int tableIndex = (virtAddr & 0xFFF00000) >> 18;
  // 1st level page table must be aligned to 16kb (bottom 14 bits)
  u32int baseAddress = ((u32int)pageTable & 0xFFFFC000);
  return (simpleEntry*)(tableIndex | baseAddress);
}


/**
 * extract second level page table entry for a given virtual address
 * given a first level page table entry: this entry contains PA of 2nd lvl PT
 **/
simpleEntry* getEntrySecond(pageTableEntry* firstLevelEntry, u32int virtAddr)
{
  DEBUG(MM_PAGE_TABLES, "getEntrySecond: 1st lvl PTE %#.8x @ %p; VA %#.8x" EOL,
        *(u32int *)firstLevelEntry, firstLevelEntry, virtAddr);

  u32int index = (virtAddr & 0x000FF000) >> 10;
  // to look through the 2nd lvl page table, we need to access it using VA
  // but we only have the physical address! find VA in pt metadata cache
  ptInfo* metadata = getPageTableInfo(firstLevelEntry);
  if (metadata == 0)
  {
    printf("getEntrySecond: metadata not found. 1st lvl PTE %#.8x @ %p; VA %#.8x" EOL,
                            *(u32int*)firstLevelEntry, firstLevelEntry, virtAddr);
    dumpPageTableInfo();
    DIE_NOW(NULL, "could not find PT2 metadta");
  }
  // however if this entry is a guest PT2 info, then virtAddr will not be set!
  if (!metadata->host)
  {
    DIE_NOW(NULL, "virtAddr not set in metadata on GPT2!");
  }
  u32int entryAddress = metadata->virtAddr | index;
  DEBUG(MM_PAGE_TABLES, "getEntrySecond: found! base = %#.8x index %#.8x; -> %#.8x" EOL,
        metadata->virtAddr, index, entryAddress);
  return (simpleEntry *)entryAddress;
}


/**
 * function to take a first level page table entry - section 
 * and remap it as second level 4KB small pages
 **/
void splitSectionToSmallPages(simpleEntry* pageTable, u32int virtAddr)
{
  DEBUG(MM_PAGE_TABLES, "splitSectionToSmallPages: 1st level PT @ %p vAddr %#.8x" EOL,
        pageTable, virtAddr);

  // 1. get section entry
  sectionEntry *sectionEntryPtr = (sectionEntry *)getEntryFirst(pageTable, virtAddr);
  DEBUG(MM_PAGE_TABLES, "splitSectionToSmallPages: section entry @ %p = %#.8x" EOL,
        sectionEntryPtr, *(u32int *)sectionEntryPtr);
  mmuInvalidateUTLBbyMVA(virtAddr);

  // 2. invalidate entry.
  sectionEntryPtr->type = 0;
  u16int domain = sectionEntryPtr->domain;
  u16int c = sectionEntryPtr->c;
  u16int b = sectionEntryPtr->b;
  u16int tex = sectionEntryPtr->tex;
  u16int xn = sectionEntryPtr->xn;

  // 3. map memory in small pages
  virtAddr = virtAddr & 0xFFF00000;
  u32int physAddr = (sectionEntryPtr->addr) << 20;
  
  DEBUG(MM_PAGE_TABLES, "splitSectionToSmallPages: vaddr %#.8x, pAddr %#.8x" EOL, virtAddr, physAddr);

  u32int protectionBits = sectionEntryPtr->ap10 | (sectionEntryPtr->ap2 << 2);
  u32int index = 0;
  for (index = 0; index < PT2_ENTRIES; index++)
  {
    mapSmallPage(pageTable, virtAddr + (SMALL_PAGE_SIZE * index),
       physAddr + (SMALL_PAGE_SIZE * index), domain, protectionBits, c, b, tex, xn);
  }
}


/**
 * check if a given virtual address is in any of the guest page tables.
 * a.k.a. the given address points to a page table entry
 **/
bool isAddrInPageTable(simpleEntry* pageTablePhys, u32int physAddr)
{
  GCONTXT *context = getGuestContext();
  DEBUG(MM_PAGE_TABLES, "isAddrInPageTable: is physAddr %#.8x in PT %p" EOL, physAddr, pageTablePhys);
  if (pageTablePhys == NULL)
  {
    // no guest page table yet.
    return FALSE;
  }

  // is the address in guests first level page table?
  if((physAddr >= (u32int)pageTablePhys) && 
     (physAddr <= ((u32int)pageTablePhys + PT1_SIZE - 1)) )
  {
    DEBUG(MM_PAGE_TABLES, "isAddrInPageTable: phys address points to a 1st lvl page table entry" EOL);
    return TRUE;
  }

  // maybe second level page tables live in this section?
  ptInfo *head = context->pageTables->gptInfo;
  while (head != NULL)
  {
    if ( (head->physAddr <= physAddr ) && ((head->physAddr+PT2_SIZE-1) >= physAddr) )
    {
      DEBUG(MM_PAGE_TABLES, "isAddrInPageTable: phys addr points to a 2nd lvl page table entry" EOL);
      return TRUE;
    }
    head = head->nextEntry;
  }
  // otherwise, not found - return false

  return FALSE; 
}


void addPageTableInfo(pageTableEntry* entry, u32int virtual, u32int physical, u32int mapped, bool host)
{
  DEBUG(MM_PAGE_TABLES, "addPageTableInfo: entry %#.8x @ %p, PA %#.8x VA %#.8x, mapped %#.8x host %x" EOL,
        *(u32int *)entry, entry, physical, virtual, mapped, host);
  GCONTXT *context = getGuestContext();
  
  ptInfo *newEntry = (ptInfo *)malloc(sizeof(ptInfo));
  DEBUG(MM_PAGE_TABLES, "addPageTableInfo: new entry @ %p" EOL, newEntry);
  newEntry->firstLevelEntry = entry;
  newEntry->physAddr = physical;
  newEntry->virtAddr = virtual;
  newEntry->host = host;
  newEntry->mappedMegabyte = mapped;
  newEntry->nextEntry = 0;

  ptInfo *head = (host) ? context->pageTables->sptInfo : context->pageTables->gptInfo;
  if (head == NULL)
  {
    // first entry
    if (host)
    {
      DEBUG(MM_PAGE_TABLES, "addPageTableInfo: first entry, sptInfo now %p" EOL, newEntry);
      context->pageTables->sptInfo = newEntry;
    }
    else
    {
      DEBUG(MM_PAGE_TABLES, "addPageTableInfo: first entry, gptInfo now %p" EOL, newEntry);
      context->pageTables->gptInfo = newEntry;
    }
  }
  else
  {
    // loop to the end of the list
    DEBUG(MM_PAGE_TABLES, "addPageTableInfo: not the first entry. head %p" EOL, head);
    while (head->nextEntry != NULL)
    {
      head = head->nextEntry;
    }
    head->nextEntry = newEntry;
  }
  dumpPageTableInfo();
  // done.
}


ptInfo *getPageTableInfo(pageTableEntry *firstLevelEntry)
{
  DEBUG(MM_PAGE_TABLES, "getPageTableInfo: first level entry ptr %p = %#.8x" EOL, firstLevelEntry,
        *(u32int *)firstLevelEntry);
  GCONTXT* context = getGuestContext();

  // spt first
  ptInfo *head = context->pageTables->sptInfo;
  while (head != NULL)
  {
    if (head->firstLevelEntry == firstLevelEntry)
    {
      DEBUG(MM_PAGE_TABLES, "getPageTableInfo: found spt; entry %p = %#.8x" EOL,
            head->firstLevelEntry, *(u32int *)head->firstLevelEntry);
      return head;
    }
    head = head->nextEntry;
  }

  // its not spt, gpt then? 
  head = context->pageTables->gptInfo;
  while (head != NULL)
  {
    if (head->firstLevelEntry == firstLevelEntry)
    {
      DEBUG(MM_PAGE_TABLES, "getPageTableInfo: found gpt; entry %p = %#.8x" EOL,
            head->firstLevelEntry, *(u32int *)head->firstLevelEntry);
      return head;
    }
    head = head->nextEntry;
  }

  // not found, return 0
  return 0;
}


void removePageTableInfo(pageTableEntry* firstLevelEntry, bool host)
{
  DEBUG(MM_PAGE_TABLES, "removePageTableInfo: first level entry ptr %p = %#.8x" EOL,
        firstLevelEntry, *(u32int *)firstLevelEntry);
  GCONTXT *context = getGuestContext();

  ptInfo *head = (host) ? context->pageTables->sptInfo : context->pageTables->gptInfo;
  ptInfo *prev = NULL;

  while (head != NULL)
  {
    if (head->firstLevelEntry == firstLevelEntry)
    {
      DEBUG(MM_PAGE_TABLES, "removePageTableInfo: found entry %p = %#.8x" EOL,
            head->firstLevelEntry, *(u32int *)head->firstLevelEntry);
      ptInfo* tmp = head;
      head = head->nextEntry;
      if (host)
      {
        free((void *)tmp->virtAddr);
      }

      if (prev == 0)
      {
        // first entry in list.
        if (host)
        {
          context->pageTables->sptInfo = head;
        }
        else
        {
          context->pageTables->gptInfo = head;
        }
      }
      else
      {
        // not the first entry in list
        prev->nextEntry = head;
      }
      free((void *)tmp);
      return;
    }
    prev = head; 
    head = head->nextEntry;
  }
}


void dumpPageTableInfo()
{
#if CONFIG_DEBUG_MM_PAGE_TABLES
  printf("dumpPageTableInfo:" EOL);
  
  GCONTXT* context = getGuestContext();
  // spt first
  ptInfo* head = context->pageTables->sptInfo;
  printf("sptInfo:" EOL);
  while (head != 0)
  {
    printf("%p: ptEntry %p; PA %#.8x VA %#.8x host %x" EOL, head, head->firstLevelEntry, head->physAddr, head->virtAddr, head->host);
    head = head->nextEntry;
  }

  // gpt then 
  head = context->pageTables->gptInfo;
  printf("gptInfo:" EOL);
  while (head != 0)
  {
    printf("%p: ptEntry %p; PA %#.8x VA %#.8x host %x" EOL, head, head->firstLevelEntry, head->physAddr, head->virtAddr, head->host);
    head = head->nextEntry;
  }
#endif
}


void invalidatePageTableInfo()
{
  DEBUG(MM_PAGE_TABLES, "invalidatePageTableInfo:" EOL);
  GCONTXT* context = getGuestContext();
  // spt first
  while (context->pageTables->sptInfo != 0)
  {
    // zero fields
    context->pageTables->sptInfo->firstLevelEntry = 0;
    context->pageTables->sptInfo->physAddr = 0;
    free((void*)context->pageTables->sptInfo->virtAddr);
    context->pageTables->sptInfo->virtAddr = 0;
    
    ptInfo* tempPtr = context->pageTables->sptInfo;
    context->pageTables->sptInfo = context->pageTables->sptInfo->nextEntry;
    free((void*)tempPtr);
  }
  context->pageTables->sptInfo = 0;
  
  // gpt then 
  while (context->pageTables->gptInfo != 0)
  {
    // zero fields
    context->pageTables->gptInfo->firstLevelEntry = 0;
    context->pageTables->gptInfo->physAddr = 0;
    context->pageTables->gptInfo->virtAddr = 0;
    context->pageTables->gptInfo->host = FALSE;
    
    ptInfo* tempPtr = context->pageTables->gptInfo;
    context->pageTables->gptInfo = context->pageTables->gptInfo->nextEntry;
    free((void*)tempPtr);
  }
  context->pageTables->gptInfo = 0;
  DEBUG(MM_PAGE_TABLES, "invalidatePageTableInfo: ...done" EOL);
}


/**
 * Called from the instruction emulator when we have a permission abort
 * and the guest is writing to its own page table
 **/
void pageTableEdit(u32int address, u32int newVal)
{
  GCONTXT* context = getGuestContext();

  u32int virtualAddress;
  bool firstLevelEntry;
  simpleEntry* oldGuestEntry = (simpleEntry*)address;
  simpleEntry* newGuestEntry = (simpleEntry*)&newVal;

  //early exit if OS is not actually updating anything
  if (*(u32int*)oldGuestEntry == newVal)
  {
    return;
  }

  if ((u32int)context->pageTables->guestVirtual == 0)
  {
    DIE_NOW(context, "guest virtual not set.");
  }
  u32int physicalAddress = getPhysicalAddress(context->pageTables->guestVirtual, address);
  DEBUG(MM_PAGE_TABLES, "PageTableEdit: address %#.8x newval: %#.8x" EOL, address, newVal);
  DEBUG(MM_PAGE_TABLES, "PageTableEdit: physical address of edit %#.8x, phys gPT %p" EOL,
        physicalAddress, context->pageTables->guestPhysical);

  // Step 1: Get the virtual fault address

  // If we faulted on a write to the 1st level PT then we already have the virtual address
  if( (u32int)context->pageTables->guestPhysical == (physicalAddress & 0xFFFFC000) )
  {
    firstLevelEntry = TRUE;
    // get the virtual address by shifting bits 13:2 of the FA to be bits 31:20 of the VA
    virtualAddress = address << 18;
  }
  else
  {
    // write to an existing 2nd Level table, which we now need to find
    // to get the full virtual address
    u32int editAddr = physicalAddress & PT2_ALIGN_MASK; 
    DEBUG(MM_PAGE_TABLES, "pageTableEdit: PT2 case: editAddr masked to PT2 size %#.8x" EOL, editAddr);
    firstLevelEntry = FALSE;

    ptInfo* head = context->pageTables->gptInfo;

    while (head != 0)
    {
      if (head->physAddr == editAddr)
      {
        break;
      }
      head = head->nextEntry;
    }
    if (head == 0)
    {
      DIE_NOW(context, "pageTableEdit: guest editing second lvl PT, couldn't find metadata." EOL);
    }
    DEBUG(MM_PAGE_TABLES, "pageTableEdit: FOUND virt %#.8x phys %#.8x entry %#.8x @ %p" EOL,
          head->virtAddr, head->physAddr, *(u32int *)head->firstLevelEntry, head->firstLevelEntry);
    if (context->pageTables->guestVirtual == NULL)
    {
      DIE_NOW(context, "pageTableEdit: guestVirtual not set" EOL);
    }
    virtualAddress = ((u32int)head->firstLevelEntry - (u32int)context->pageTables->guestVirtual) << 18;
    virtualAddress |= ((address & 0x3FC) << 10);
    DEBUG(MM_PAGE_TABLES, "pageTableEdit: PT2 case: pt edit corresponds to VA %#.8x" EOL,
          virtualAddress);
  }


  DEBUG(MM_PAGE_TABLES, "pageTableEdit: virtualAddr %#.8x oldguestEntry %#.8x newGuestEntry %#.8x"
        EOL, virtualAddress, *(u32int*)oldGuestEntry, *(u32int*)newGuestEntry);

  if (firstLevelEntry && (oldGuestEntry->type == RESERVED))
  {
    printf("pageTableEdit: addr %#.8x newVal %#.8x vAddr %#.8x" EOL, address, newVal, virtualAddress);
    printf("pageTableEdit: oldGuestEntry %#.8x newGuestEntry %#.8x" EOL,
           *(u32int*)oldGuestEntry, *(u32int*)newGuestEntry);
    DIE_NOW(context, "old entry RESERVED type");
  }

  simpleEntry* shadowUser = getEntryFirst(context->pageTables->shadowUser, virtualAddress);
  simpleEntry* shadowPriv = getEntryFirst(context->pageTables->shadowPriv, virtualAddress);

  // Step 2: Work Out what kind of change we are dealing with
  if (oldGuestEntry->type != newGuestEntry->type)
  {
    // CHANGING ENTRY TYPE
    if ((oldGuestEntry->type != FAULT) && (newGuestEntry->type == FAULT))
    {
      // important, removing entry
      if (firstLevelEntry)
      {
        // first level page table edit
        if (oldGuestEntry->type == PAGE_TABLE)
        {
          // changing a page table to a section or fault, need to remove PT entry.
          removePageTableInfo((pageTableEntry*)oldGuestEntry, FALSE);
          shadowUnmapPageTable((pageTableEntry*)shadowPriv, (pageTableEntry*)oldGuestEntry, virtualAddress);
          shadowUnmapPageTable((pageTableEntry*)shadowUser, (pageTableEntry*)oldGuestEntry, virtualAddress);
        }
        else if (oldGuestEntry->type == SECTION)
        {
          // changing a page table to a section or fault, need to remove PT entry.
          shadowUnmapSection((simpleEntry*)shadowPriv, (sectionEntry*)oldGuestEntry, virtualAddress);
          shadowUnmapSection((simpleEntry*)shadowUser, (sectionEntry*)oldGuestEntry, virtualAddress);
        }
        // nothing to do if old type was reserved.
      }
      else
      {
        if ((oldGuestEntry->type == SMALL_PAGE) || (oldGuestEntry->type == SMALL_PAGE_3))
        {
          if (shadowUser->type != FAULT)
          {
            shadowUser = getEntrySecond((pageTableEntry*)shadowUser, virtualAddress);
            shadowUnmapSmallPage((smallPageEntry*)shadowUser, (smallPageEntry*)oldGuestEntry, virtualAddress);
          }
          if (shadowPriv->type != FAULT)
          {
            shadowPriv = getEntrySecond((pageTableEntry*)shadowPriv, virtualAddress);
            shadowUnmapSmallPage((smallPageEntry*)shadowPriv, (smallPageEntry*)oldGuestEntry, virtualAddress);
          }
        }
        else
        {
          printf("pageTableEdit: addr %#.8x newVal %#.8x" EOL, address, newVal);
          printf("pageTableEdit: virtualAddr %#.8x oldguestEntry %#.8x newGuestEntry %#.8x" EOL,
                          virtualAddress, *(u32int*)oldGuestEntry, *(u32int*)newGuestEntry);
          printf("pageTableEdit: shadowUser %#.8x @ %p shadowPriv %#.8x @ %p" EOL,
            *(u32int*)shadowUser, shadowUser, *(u32int*)shadowPriv, shadowPriv);
          shadowUser = getEntrySecond((pageTableEntry*)shadowUser, virtualAddress);
          shadowPriv = getEntrySecond((pageTableEntry*)shadowPriv, virtualAddress);
          printf("pageTableEdit: 2nd lvl shadowUser %#.8x @ %p shadowPriv %#.8x @ %p" EOL,
            *(u32int*)shadowUser, shadowUser, *(u32int*)shadowPriv, shadowPriv);
          DIE_NOW(context, "remove/change type 2nd lvl entry unimplemented." EOL);
        }
      }
    }

    if ((oldGuestEntry->type == FAULT) && (newGuestEntry->type != FAULT))
    {
      // old entry fault, new !fault. add entry case ignore.
      DEBUG(MM_PAGE_TABLES, "pageTableEdit: old entry fault, new entry !fault. ignore ADD" EOL);
      return;
    }

    if ((oldGuestEntry->type != FAULT) && (newGuestEntry->type != FAULT))
    {
      // old entry fault, new !fault. changing page table entry type. mustn't ignore
      DIE_NOW(NULL, "old entry !fault, new entry !fault. change type unimplemented." EOL);
    }
  }
  else
  {
    // type of old entry is the same as new one. just changing attributes!
    DEBUG(MM_PAGE_TABLES, "pageTableEdit: attributes @ %#.8x, oldValue %#.8x, newValue %#.8x" EOL,
          address, *(u32int *)address, newVal);
    DEBUG(MM_PAGE_TABLES, "pageTableEdit: shadowPriv %#.8x shadowUser %#.8x" EOL,
          *(u32int *)shadowPriv, *(u32int *)shadowUser);
    // guest may be changing access permissions.
    // this requires us to force the guest into an appropriate mode
    u32int cpsrPriv = PSR_SVC_MODE;
    u32int cpsrUser = PSR_USR_MODE;
    u32int cpsrBackup = context->CPSR; 
    if (firstLevelEntry)
    {
      switch(oldGuestEntry->type)
      {
        case SECTION:
        {
          context->CPSR = cpsrPriv; 
          editAttributesSection((sectionEntry*)oldGuestEntry, (sectionEntry*)newGuestEntry,
                                                               shadowPriv, virtualAddress);
          context->CPSR = cpsrUser; 
          editAttributesSection((sectionEntry*)oldGuestEntry, (sectionEntry*)newGuestEntry,
                                                               shadowUser, virtualAddress);
          context->CPSR = cpsrBackup; 
          break;
        }
        case PAGE_TABLE:
        {
          context->CPSR = cpsrPriv; 
          editAttributesPageTable((pageTableEntry*)oldGuestEntry, (pageTableEntry*)newGuestEntry,
                                                    (pageTableEntry*)shadowPriv, virtualAddress);
          context->CPSR = cpsrUser; 
          editAttributesPageTable((pageTableEntry*)oldGuestEntry, (pageTableEntry*)newGuestEntry,
                                                    (pageTableEntry*)shadowUser, virtualAddress);
          context->CPSR = cpsrBackup; 
          break;
        }
        case RESERVED:
        case FAULT:
        {
          DIE_NOW(context, "pageTableEdit: edit fault/reserved details case. wtf.");
          break;
        }
      } // switch type
    }
    else
    {
      switch(oldGuestEntry->type)
      {
        case SMALL_PAGE:
        case SMALL_PAGE_3:
        {
          context->CPSR = cpsrPriv; 
          editAttributesSmallPage((smallPageEntry*)oldGuestEntry, (smallPageEntry*)newGuestEntry,
                                                    (smallPageEntry*)shadowPriv, virtualAddress);
          context->CPSR = cpsrUser; 
          editAttributesSmallPage((smallPageEntry*)oldGuestEntry, (smallPageEntry*)newGuestEntry,
                                                    (smallPageEntry*)shadowUser, virtualAddress);
          context->CPSR = cpsrBackup; 
          break;
        }
        case LARGE_PAGE:
        {
          DIE_NOW(context, "editing attributes LARGE_PAGE: unimplemented");
        }
        case FAULT:
        {
          DIE_NOW(context, "editing attributes FAULT type?");
        }
      }
    } // else second-level entry ends 
  } // editing entry attributes ends
//  mmuClearTLBbyMVA(address);
  mmuInvalidateUTLB();
  mmuClearDataCache();
  mmuDataMemoryBarrier();
}



void editAttributesSection(sectionEntry* oldSection, sectionEntry* newSection, simpleEntry* shadow, u32int virtual)
{
  // WARNING: shadow descriptor type might not correspond to guest descriptor type!!! 
  DEBUG(MM_PAGE_TABLES, "editAttributesSection: oldSection %#.8x, newSection %#.8x shadow %#.8x"
        EOL, *(u32int *)oldSection, *(u32int *)newSection, *(u32int *)shadow);
  GCONTXT *context = getGuestContext();

  if (shadow->type == FAULT)
  {
    // quick exit if not shadow mapped yet
    return;
  }

  if ((oldSection->superSection != newSection->superSection) ||
      (oldSection->addr != newSection->addr))
  {
    // if changing base address or section <-> supersection
    // then it's a remove&add operation
    // check if shadow entry isn't split up to pages...
    if (shadow->type != SECTION)
    {
      DIE_NOW(context, "shadow entry SPLIT");
    }
    else
    {
      DIE_NOW(context, "remap address or supersection");
      // must shadow unmap and shadow re-map section.
    }
  }

  if (oldSection->b != newSection->b)
  {
    DIE_NOW(context, "edit bufferable bit");
  }

  if (oldSection->c != newSection->c)
  {
    DIE_NOW(context, "edit cacheable bit");
  }

  if (oldSection->xn != newSection->xn)
  {
    if (shadow->type != SECTION)
    {
      DIE_NOW(context, "map XN, shadow entry SPLIT");
    }
    else
    {
      DIE_NOW(context, "edit XN bit");
      ((sectionEntry*)shadow)->xn = newSection->xn;
    }
  }

  // even if shadow section was split to small pages, domain bits are the same
  if (oldSection->domain != newSection->domain)
  {
    ((sectionEntry*)shadow)->domain = mapGuestDomain(newSection->domain);
  }

  //Carefull of this one, field is used by the hypervisor
  if (oldSection->imp != newSection->imp)
  {
    DIE_NOW(context, "imp-dep bit set");
  }

  if (oldSection->tex != newSection->tex)
  {
    DIE_NOW(context, "edit TEX bits");
  }

  if ((oldSection->ap10 != newSection->ap10) || (oldSection->ap2 != newSection->ap2))
  {
    mapAPBitsSection(newSection, (simpleEntry*)shadow, virtual);
  }

  if (oldSection->s != newSection->s)
  {
    DIE_NOW(context, "edit details, edit Shareable bit");
  }

  if (oldSection->nG != newSection->nG)
  {
    DIE_NOW(context, "edit details, edit non-Global bit");
  }

  if (oldSection->ns != newSection->ns)
  {
    DIE_NOW(context, "edit details, edit non-secure bit");
  }
}


void editAttributesPageTable(pageTableEntry* oldTable, pageTableEntry* newTable, pageTableEntry* shadowTable, u32int virtual)
{
  printf("editAttributesPageTable: for virtual address %#.8x; implement" EOL, virtual);
  DIE_NOW(NULL, "unimplemented");
}


void editAttributesSmallPage(smallPageEntry* oldPage, smallPageEntry* newPage, smallPageEntry* shadowPage, u32int virtual)
{
  DEBUG(MM_PAGE_TABLES, "editAttributesSmallPage: oldPage %#.8x, newPage %#.8x shadowPage %#.8x"
        EOL, *(u32int *)oldPage, *(u32int *)newPage, *(u32int *)shadowPage);
  GCONTXT *context = getGuestContext();

  if (shadowPage->type == FAULT)
  {
    // quick exit if not shadow mapped yet
    return;
  }
  printf("editAttributesSmallPage: oldPage %#.8x, newPage %#.8x shadowPage %#.8x" EOL,
         *(u32int *)oldPage, *(u32int *)newPage, *(u32int *)shadowPage);

  if (oldPage->addr != newPage->addr)
  {
    // changing base address of entry, remove then add operation.
    // must shadow unmap and shadow re-map section.
    DIE_NOW(context, "remap address");
  }

  if (oldPage->b != newPage->b)
  {
    DIE_NOW(context, "edit bufferable bit");
  }

  if (oldPage->c != newPage->c)
  {
    DIE_NOW(context, "edit cacheable bit");
  }

  if (oldPage->xn != newPage->xn)
  {
    DIE_NOW(context, "edit XN bit");
  }

  if ((oldPage->ap10 != newPage->ap10) || (oldPage->ap2 != newPage->ap2))
  {
    DIE_NOW(context, "edit AP bits");
//    mapAPBitsSection(newSection, (simpleEntry*)shadow, virtual);
  }

  if (oldPage->tex != newPage->tex)
  {
    DIE_NOW(context, "edit TEX bits");
  }

  if (oldPage->s != newPage->s)
  {
    DIE_NOW(context, "edit details, edit Shareable bit");
  }

  if (oldPage->nG != newPage->nG)
  {
    DIE_NOW(context, "edit details, edit non-Global bit");
  }
}
