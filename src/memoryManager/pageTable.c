#include "common/assert.h"
#include "common/bit.h"
#include "common/debug.h"
#include "common/linker.h"
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
#include "memoryManager/stack.h"


extern void v7_flush_dcache_all(u32int dev);


/**
 * allocate space for a new level 1 base page table
 **/
u32int* newLevelOnePageTable()
{
  // alloc some some space for the 1stLevel table
  u32int pageTableAddress = (u32int)memalign(1 << PT1_ALIGN_BITS, PT1_SIZE);
  if (pageTableAddress == 0)
  {
    DIE_NOW(0, "newLevelOnePageTable: Failed to allocate page table");
  }
  memset((void*)pageTableAddress, 0x0, PT1_SIZE);
  
  // test if page table is correctly aligned
  if((pageTableAddress & PT1_ALIGN_MASK) != pageTableAddress)
  {
    DIE_NOW(0, "newLevelOnePageTable: New 1st level page table is not correctly aligned.");
  }
#ifdef PAGE_TABLE_DBG
  printf("newLevelOnePageTable: Page Table base addr: %08x\n", pageTableAddress);
#endif
  return (u32int*)pageTableAddress;
}


/**
 * allocate space for a new level 2 base page table
 **/
u32int* newLevelTwoPageTable(void)
{
  //alloc some some space for the 1stLevel table
  u32int pageTableAddress = (u32int)memalign(1 << PT2_ALIGN_BITS, PT2_SIZE);
  if (pageTableAddress == 0)
  {
    DIE_NOW(0, "newLevelTwoPageTable: Failed to allocate page table");
  }
  memset((void*)pageTableAddress, 0x0, PT2_SIZE);
  
  // test if page table is correctly aligned
  if((pageTableAddress & PT2_ALIGN_MASK) != pageTableAddress)
  {
    DIE_NOW(0, "newLevelTwoPageTable: New 1st level page table is not correctly aligned.");
  }
#ifdef PAGE_TABLE_DBG
  printf("newLevelTwoPageTable: Page Table base addr: %08x\n", pageTableAddress);
#endif
  return (u32int*)pageTableAddress;
}


/**
 * frees an allocated level two page table and removes the corresponding
 * meta data information from the list. only called for 2nd level SHADOW page
 * tables. we can't delete guest page tables, not our memory!
 **/
void deleteLevelTwoPageTable(pageTableEntry* pageTable)
{
  GCONTXT* context = getGuestContext();
#ifdef PAGE_TABLE_DBG
  printf("deleteLevelTwoPageTable: page table entry %08x @ %08x\n", *(u32int*)pageTable, (u32int)pageTable);
#endif
  // this can only be called on shadow second level page tables
  ptInfo* head = context->pageTables->sptInfo;
  ptInfo* prev = 0;
  while (head != 0)
  {
    if ((head->firstLevelEntry == pageTable) && (*(u32int*)head->firstLevelEntry == *(u32int*)pageTable))
    {
      head->firstLevelEntry = 0;
      head->physAddr = 0;
      free((void*)head->virtAddr);
      head->virtAddr = 0;
      
      // remember a pointer to this doomed entry
      ptInfo* tempPtr = head;
      head = head->nextEntry;
      free((void*)tempPtr);
      
      // all thats left is to link previous entry to next entry (if there was a previous entry!)
      if (prev != 0)
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
  /*
   * Make use of MMU protection mechanisms to protect the hypervisor against guests and itself.
   * Make as few memory as possible executable: only the code of the hypervisor and instruction
   * caches, if we have them (only with block copy -- TODO).
   *
   * First of all, make sure the stuff we get from the linker makes sense (it usually either works
   * or dies, but sometimes silently messes up):
   */
  ASSERT(MEMORY_START_ADDR <= HYPERVISOR_BEGIN_ADDRESS, "bad linker symbols");
  ASSERT(HYPERVISOR_BEGIN_ADDRESS < HYPERVISOR_END_ADDRESS, "bad linker symbols");
  ASSERT(HYPERVISOR_END_ADDRESS <= MEMORY_END_ADDR, "bad linker symbols");
  ASSERT(HYPERVISOR_BEGIN_ADDRESS == HYPERVISOR_TEXT_BEGIN_ADDRESS, "bad linker symbols");
  ASSERT(HYPERVISOR_TEXT_BEGIN_ADDRESS < HYPERVISOR_TEXT_END_ADDRESS, "bad linker symbols");
  ASSERT(HYPERVISOR_TEXT_END_ADDRESS <= HYPERVISOR_RO_XN_BEGIN_ADDRESS, "bad linker symbols");
  ASSERT(HYPERVISOR_RO_XN_BEGIN_ADDRESS < HYPERVISOR_RO_XN_END_ADDRESS, "bad linker symbols");
  ASSERT(HYPERVISOR_RO_XN_BEGIN_ADDRESS <= HYPERVISOR_RW_XN_BEGIN_ADDRESS, "bad linker symbols");
  ASSERT(HYPERVISOR_RW_XN_BEGIN_ADDRESS < HYPERVISOR_RW_XN_END_ADDRESS, "bad linker symbols");
  ASSERT(HYPERVISOR_RW_XN_END_ADDRESS == HYPERVISOR_END_ADDRESS, "bad linker symbols");
  ASSERT(HYPERVISOR_RW_XN_END_ADDRESS <= RAM_XN_POOL_BEGIN, "bad linker symbols");
  ASSERT(RAM_XN_POOL_BEGIN < RAM_XN_POOL_END, "bad linker symbols");
  ASSERT(RAM_XN_POOL_END <= MEMORY_END_ADDR, "bad linker symbols");
  /*
   * Now make sure the linker followed our alignment constraints:
   */
  ASSERT(isAlignedToMaskN(HYPERVISOR_TEXT_BEGIN_ADDRESS, SMALL_PAGE_MASK),
         "executable section not aligned on small page boundary");
  ASSERT(isAlignedToMaskN(HYPERVISOR_RO_XN_BEGIN_ADDRESS, SMALL_PAGE_MASK),
         "non-executable read-only section not aligned on small page boundary");
  ASSERT(isAlignedToMaskN(HYPERVISOR_RW_XN_BEGIN_ADDRESS, SMALL_PAGE_MASK),
           "non-executable read-write sections not aligned on small page boundary");
  ASSERT(isAlignedToMaskN(RAM_XN_POOL_BEGIN, SMALL_PAGE_MASK),
         "non-executable RAM pool not aligned on small page boundary");
  ASSERT(isAlignedToMaskN(RAM_XN_POOL_END, SMALL_PAGE_MASK),
         "non-executable RAM pool not aligned on small page boundary");
  /*
   * Stacks must be non-executable, and should be protected against overflow by leaving gaps (fault
   * entries) in the translation table. Stacks and gaps must reside in one of the data sections and
   * must be aligned on small page boundaries! Gaps must directly precede their associated stack.
   * This implies stack size must be a multiple of small page size.
   */
  ASSERT(HYPERVISOR_RW_XN_BEGIN_ADDRESS <= ABT_STACK_GAP, "bad stack layout");
  ASSERT(HYPERVISOR_RW_XN_BEGIN_ADDRESS <= FIQ_STACK_GAP, "bad stack layout");
  ASSERT(HYPERVISOR_RW_XN_BEGIN_ADDRESS <= IRQ_STACK_GAP, "bad stack layout");
  ASSERT(HYPERVISOR_RW_XN_BEGIN_ADDRESS <= SVC_STACK_GAP, "bad stack layout");
  ASSERT(HYPERVISOR_RW_XN_BEGIN_ADDRESS <= UND_STACK_GAP, "bad stack layout");
  ASSERT(HYPERVISOR_RW_XN_BEGIN_ADDRESS <= TOP_STACK_GAP, "bad stack layout");
  ASSERT(isAlignedToMaskN(ABT_STACK_GAP, SMALL_PAGE_MASK), "bad stack gap alignment");
  ASSERT(isAlignedToMaskN(FIQ_STACK_GAP, SMALL_PAGE_MASK), "bad stack gap alignment");
  ASSERT(isAlignedToMaskN(IRQ_STACK_GAP, SMALL_PAGE_MASK), "bad stack gap alignment");
  ASSERT(isAlignedToMaskN(SVC_STACK_GAP, SMALL_PAGE_MASK), "bad stack gap alignment");
  ASSERT(isAlignedToMaskN(UND_STACK_GAP, SMALL_PAGE_MASK), "bad stack gap alignment");
  ASSERT(isAlignedToMaskN(TOP_STACK_GAP, SMALL_PAGE_MASK), "bad stack gap alignment");
  ASSERT((ABT_STACK_GAP + SMALL_PAGE_SIZE) == ABT_STACK_LB, "bad stack gap position");
  ASSERT((FIQ_STACK_GAP + SMALL_PAGE_SIZE) == FIQ_STACK_LB, "bad stack gap position");
  ASSERT((IRQ_STACK_GAP + SMALL_PAGE_SIZE) == IRQ_STACK_LB, "bad stack gap position");
  ASSERT((SVC_STACK_GAP + SMALL_PAGE_SIZE) == SVC_STACK_LB, "bad stack gap position");
  ASSERT((UND_STACK_GAP + SMALL_PAGE_SIZE) == UND_STACK_LB, "bad stack gap position");
  ASSERT(isAlignedToMaskN(ABT_STACK_UB, SMALL_PAGE_MASK), "bad stack alignment");
  ASSERT(isAlignedToMaskN(FIQ_STACK_UB, SMALL_PAGE_MASK), "bad stack alignment");
  ASSERT(isAlignedToMaskN(IRQ_STACK_UB, SMALL_PAGE_MASK), "bad stack alignment");
  ASSERT(isAlignedToMaskN(SVC_STACK_UB, SMALL_PAGE_MASK), "bad stack alignment");
  ASSERT(isAlignedToMaskN(UND_STACK_UB, SMALL_PAGE_MASK), "bad stack alignment");
  ASSERT(ABT_STACK_UB <= HYPERVISOR_RW_XN_END_ADDRESS, "bad stack layout");
  ASSERT(FIQ_STACK_UB <= HYPERVISOR_RW_XN_END_ADDRESS, "bad stack layout");
  ASSERT(IRQ_STACK_UB <= HYPERVISOR_RW_XN_END_ADDRESS, "bad stack layout");
  ASSERT(SVC_STACK_UB <= HYPERVISOR_RW_XN_END_ADDRESS, "bad stack layout");
  ASSERT(UND_STACK_UB <= HYPERVISOR_RW_XN_END_ADDRESS, "bad stack layout");
  ASSERT((TOP_STACK_GAP + SMALL_PAGE_SIZE) <= HYPERVISOR_RW_XN_END_ADDRESS, "bad stack layout");
  /*
   * Hardcoded stack order... have some ASSERTs ready for when someone decides to change startup.S.
   */
  ASSERT(SVC_STACK_UB == ABT_STACK_GAP, "hardcoded stack layout was modified; update required");
  ASSERT(ABT_STACK_UB == UND_STACK_GAP, "hardcoded stack layout was modified; update required");
  ASSERT(UND_STACK_UB == IRQ_STACK_GAP, "hardcoded stack layout was modified; update required");
  ASSERT(IRQ_STACK_UB == FIQ_STACK_GAP, "hardcoded stack layout was modified; update required");
  ASSERT(FIQ_STACK_UB == TOP_STACK_GAP, "hardcoded stack layout was modified; update required");
  /*
   * Finally... map hypervisor memory.
   */
#ifndef CONFIG_DISABLE_HYPERVISOR_MEMORY_PROTECTION
  mapRange(pageTable, HYPERVISOR_TEXT_BEGIN_ADDRESS, HYPERVISOR_TEXT_BEGIN_ADDRESS,
           HYPERVISOR_TEXT_END_ADDRESS, HYPERVISOR_ACCESS_DOMAIN, PRIV_RO_USR_NO, TRUE,
           FALSE, 0, FALSE);
  mapRange(pageTable, HYPERVISOR_RO_XN_BEGIN_ADDRESS, HYPERVISOR_RO_XN_BEGIN_ADDRESS,
           HYPERVISOR_RO_XN_END_ADDRESS, HYPERVISOR_ACCESS_DOMAIN, PRIV_RO_USR_NO, TRUE, FALSE, 0,
           TRUE);
  mapRange(pageTable, HYPERVISOR_RW_XN_BEGIN_ADDRESS, HYPERVISOR_RW_XN_BEGIN_ADDRESS,
           SVC_STACK_GAP, HYPERVISOR_ACCESS_DOMAIN, PRIV_RW_USR_NO, TRUE, FALSE, 0, TRUE);
  mapRange(pageTable, SVC_STACK_LB, SVC_STACK_LB, SVC_STACK_UB, HYPERVISOR_ACCESS_DOMAIN,
           PRIV_RW_USR_NO, TRUE, FALSE, 0, TRUE);
  mapRange(pageTable, ABT_STACK_LB, ABT_STACK_LB, ABT_STACK_UB, HYPERVISOR_ACCESS_DOMAIN,
           PRIV_RW_USR_NO, TRUE, FALSE, 0, TRUE);
  mapRange(pageTable, UND_STACK_LB, UND_STACK_LB, UND_STACK_UB, HYPERVISOR_ACCESS_DOMAIN,
           PRIV_RW_USR_NO, TRUE, FALSE, 0, TRUE);
  mapRange(pageTable, IRQ_STACK_LB, IRQ_STACK_LB, IRQ_STACK_UB, HYPERVISOR_ACCESS_DOMAIN,
           PRIV_RW_USR_NO, TRUE, FALSE, 0, TRUE);
  mapRange(pageTable, FIQ_STACK_LB, FIQ_STACK_LB, FIQ_STACK_UB, HYPERVISOR_ACCESS_DOMAIN,
           PRIV_RW_USR_NO, TRUE, FALSE, 0, TRUE);
  mapRange(pageTable, TOP_STACK_GAP + SMALL_PAGE_SIZE, TOP_STACK_GAP + SMALL_PAGE_SIZE,
           HYPERVISOR_RW_XN_END_ADDRESS, HYPERVISOR_ACCESS_DOMAIN, PRIV_RW_USR_NO, TRUE, FALSE, 0,
           TRUE);
  mapRange(pageTable, RAM_XN_POOL_BEGIN, RAM_XN_POOL_BEGIN, RAM_XN_POOL_END,
           HYPERVISOR_ACCESS_DOMAIN, PRIV_RW_USR_NO, TRUE, FALSE, 0, TRUE);
#else
  mapRange(pageTable, HYPERVISOR_BEGIN_ADDRESS, HYPERVISOR_BEGIN_ADDRESS, MEMORY_END_ADDR,
           HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, TRUE, FALSE, 0, FALSE);
#endif
}

/**
 * Add a section mapping of given virtual to physical address
 * into the given base page table 
 **/
void mapSection(simpleEntry* pageTable, u32int virtAddr, u32int physical,
                u8int domain, u8int accessBits, bool c, bool b, u8int tex, u8int xn)
{
#ifdef PAGE_TABLE_DBG
  printf("mapSection: Virtual Addr: %08x, physical addr: %08x\n", virtAddr, physical);
#endif

  // check what is in the page table at the required index
  simpleEntry* firstLevelEntry = getEntryFirst(pageTable, virtAddr);
  switch(firstLevelEntry->type)
  {
    case SECTION:
    case FAULT:
    {
      addSectionEntry((sectionEntry*)firstLevelEntry, 
                      physical, domain, accessBits, c, b, tex, xn);
      break;
    }
    case PAGE_TABLE:
    {
      printf("mapSection error: VA: %08x, PA: %08x, oldEntry %08x @ %p\n", virtAddr, physical, *(u32int*)firstLevelEntry, firstLevelEntry);
      DIE_NOW(0, "mapSection: adding section entry on top of existing page table entry");
      break;
    }
    case RESERVED:
    default:
    {
      printf("mapSection error: Virtual Addr: %08x, physical addr: %08x\n", virtAddr, physical);
      DIE_NOW(0, "mapSection: adding section entry on top of existing reserved entry");
    }
  }//switch
}


/**
 * Add a small page mapping of given virtual to physical address
 * into a second page table 
 **/
void mapSmallPage(simpleEntry* pageTable, u32int virtAddr, u32int physical,
                u8int domain, u8int accessBits, u8int c, u8int b, u8int tex, u8int xn)
{
#ifdef PAGE_TABLE_DBG
  printf("mapSmallPage: Virtual %08x, physical %08x, dom: %x, AP: %x, c: %x, b: %x, tex: %x, xn: %x\n",
         virtAddr, physical, domain, accessBits, c, b, tex, xn);
#endif

  // First check 1st Level page table entry
  simpleEntry* first = getEntryFirst(pageTable, virtAddr);
#ifdef PAGE_TABLE_DBG
  printf("mapSmallPage: first entry %08x @ %p\n", *(u32int*)first, first);
#endif
  switch(first->type)
  {
    case FAULT:
    {
#ifdef PAGE_TABLE_DBG
      printf("mapSmallPage: entry for given VA is FAULT. Creating new.\n");
#endif
      // we need a new second level page table. This gives a virtual address allocated
      u32int* vAddr = newLevelTwoPageTable();
      // need to get the physical address
#ifdef CONFIG_DISABLE_HYPERVISOR_MEMORY_PROTECTION
      u32int pAddr = getPhysicalAddress(pageTable, (u32int)vAddr);
#else
      u32int pAddr = getGuestContext()->virtAddrEnabled
                   ? getPhysicalAddress(pageTable, (u32int)vAddr) : (u32int)vAddr;
#endif
#ifdef PAGE_TABLE_DBG
      printf("mapSmallPage: PT VA %08x PA %08x\n", (u32int)vAddr, pAddr);
#endif
      // store metadata
      addPageTableEntry((pageTableEntry*)first, (u32int)pAddr, domain);
      u32int mapped = ((u32int)first - (u32int)pageTable) << 18;
      addPageTableInfo((pageTableEntry*)first, (u32int)vAddr, pAddr, mapped, TRUE);
      break;
    }
    case PAGE_TABLE:
    {
      // if existing mapping is pageTable type, we're ok already
#ifdef PAGE_TABLE_DBG
      printf("mapSmallPage: entry for given VA is pageTable. Correct, first time.\n");
#endif
      break;
    }
    case SECTION:
    {
      printf("mapSmallPage: Virtual %08x, physical %08x, dom: %x, AP: %x, c: %x, b: %x, tex: %x, xn: %x\n",
             virtAddr, physical, domain, accessBits, c, b, tex, xn);
      printf("mapSmallPage: first entry %08x @ %p\n", *(u32int*)first, first);
      DIE_NOW(0, "mapSmallPage: adding page table entry on top of existing section entry");
      break;
    }
    case RESERVED:
    default:
    {
      DIE_NOW(0, "mapSmallPage: adding page table entry on top of existing reserved entry");
    }
  }//switch

  // At this point we know its a 2nd level page table entry 
  simpleEntry* second = getEntrySecond((pageTableEntry*)first, virtAddr);
#ifdef PAGE_TABLE_DBG
  printf("mapSmallPage: 2nd lvl entry @ %08x = %08x\n", (u32int)second, *(u32int*)second);
#endif
  // Again, we need to check the existing entry
  if(second->type == FAULT)
  {
#ifdef PAGE_TABLE_DBG
    printf("mapSmallPage: 2nd Level FAULT. Creating new small entry\n");
#endif
    addSmallPageEntry((smallPageEntry*)second, physical, accessBits, c, b, tex, xn);
  }
  else //Existing small or large page
  {
    printf("mapSmallPage: Virtual %08x, physical %08x, dom: %x, AP: %x, c: %x, b: %x, tex: %x, xn: %x\n",
           virtAddr, physical, domain, accessBits, c, b, tex, xn);
    printf("mapSmallPage: first entry %08x @ %p\n", *(u32int*)first, first);
    printf("mapSmallPage: 2nd lvl entry @ %08x = %08x\n", (u32int)second, *(u32int*)second);
    DIE_NOW(0, "mapSmallPage: adding over existing entry\n");
  }
}


/**
 * adds a section entry at a given place in the first level page table
 **/
void addSectionEntry(sectionEntry* sectionEntryPtr, u32int physAddr, 
     u8int domain, u8int accessBits, bool cacheable, bool bufferable, u8int tex, u8int xn)
{
  sectionEntryPtr->addr = (physAddr >> 20);
  sectionEntryPtr->type = SECTION;
  sectionEntryPtr->c = cacheable  ? 1:0;
  sectionEntryPtr->b = bufferable ? 1:0;
  sectionEntryPtr->xn = xn;
  sectionEntryPtr->domain = domain;
  sectionEntryPtr->imp = 0; //currently unused
  sectionEntryPtr->ap10 = accessBits & 0x3;
  sectionEntryPtr->tex = (tex & 0x7);
  sectionEntryPtr->ap2 = accessBits >> 2;
  sectionEntryPtr->s = 0;
  sectionEntryPtr->nG = 0;
  sectionEntryPtr->superSection = 0; // 0 for normal section (1 for supersection)
  sectionEntryPtr->ns = 1; // non secure memory?
#ifdef PAGE_TABLE_DBG
  printf("addSectionEntry: Section entry written: %08x @ %08x\n", *(u32int*)sectionEntryPtr, (u32int)sectionEntryPtr);
#endif
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
#ifdef PAGE_TABLE_DBG
  printf("addSmallPageEntry: Small descriptor written: %08x @ %08X\n", 
        *(u32int*)smallPageEntryPtr, (u32int)smallPageEntryPtr);
#endif
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
#ifdef PAGE_TABLE_DBG
  printf("addPageTableEntry: Written entry at %08x value %08x\n",
         (u32int)pageTableEntryPtr, *(u32int*)pageTableEntryPtr);
#endif
}


/**
 * Given a virtual address, retrieves the underlying physical address
 **/
u32int getPhysicalAddress(simpleEntry* pageTable, u32int virtAddr)
{
#ifdef PAGE_TABLE_DBG
  printf("getPhysicalAddress for VA %08x in PT @ %08x\n", virtAddr, (u32int)pageTable);
#endif
  simpleEntry* entryFirst = getEntryFirst(pageTable, virtAddr);
  if (entryFirst->type == FAULT)
  {
    // may not be shadow mapped? try it.
    if (!shadowMap(virtAddr))
    {
      printf("getPhysicalAddress for VA %08x in PT @ %08x\n", virtAddr, (u32int)pageTable);
      DIE_NOW(0, "getPhysicalAddress: failed to shadow map\n");
    }
  }
  switch(entryFirst->type)
  {
    case FAULT: //fall through, no break!
    {
      DIE_NOW(0, "getPhysicalAddress: fault entry in page table\n");
      break;
    }
    case SECTION:
    {
      sectionEntry* section = (sectionEntry*)entryFirst;
      if(section->superSection)
      {
        printf("getPhysicalAddress for VA %08x in PT @ %08x\n", virtAddr, (u32int)pageTable);
        printf("getPhysicalAddress: PT1 entry %08x @ %p\n", *(u32int*)section, section);
        DIE_NOW(0, "getPhysicalAddress: supersection case unimplemented\n");
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
          DIE_NOW(0, "getPhysicalAddress: Large page case unimplemented\n");
        }
        // may not be shadow mapped? try it.
        case FAULT:
        {
          if (!shadowMap(virtAddr))
          {
            printf("getPhysicalAddress for VA %08x in PT @ %08x\n", virtAddr, (u32int)pageTable);
            DIE_NOW(0, "getPhysicalAddress: failed to shadow map, lvl2\n");
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
          printf("getPhysicalAddress for VA %08x in PT @ %08x\n", virtAddr, (u32int)pageTable);
          DIE_NOW(0, "getPhysicalAddress: fault entry in page table\n");
        }
      }
      break;
    }
    case RESERVED: //fall through
    {
      printf("getPhysicalAddress for VA %08x in PT @ %08x\n", virtAddr, (u32int)pageTable);
      DIE_NOW(0, "getPhysicalAddress: reserved entry in page table\n");
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
#ifdef PAGE_TABLE_DBG
  printf("getEntryFirst: virtual address %08x\n", virtAddr);
#endif
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
#ifdef PAGE_TABLE_DBG
  printf("getEntrySecond: 1st lvl PTE %08x @ %p; VA %08x\n",
        *(u32int*)firstLevelEntry, firstLevelEntry, virtAddr);
#endif

  u32int index = (virtAddr & 0x000FF000) >> 10;
  // to look through the 2nd lvl page table, we need to access it using VA
  // but we only have the physical address! find VA in pt metadata cache
  ptInfo* metadata = getPageTableInfo(firstLevelEntry);
  if (metadata == 0)
  {
    printf("getEntrySecond: metadata not found. 1st lvl PTE %08x @ %p; VA %08x\n",
                            *(u32int*)firstLevelEntry, firstLevelEntry, virtAddr);
    dumpPageTableInfo();
    DIE_NOW(0, "getEntrySecond: could not find PT2 metadta.\n");
  }
  // however if this entry is a guest PT2 info, then virtAddr will not be set!
  if (!metadata->host)
  {
    DIE_NOW(0, "getEntrySecond: virtAddr not set in metadata on GPT2!\n");
  }
  u32int entryAddress = metadata->virtAddr | index;
#ifdef PAGE_TABLE_DBG
  printf("getEntrySecond: found! base = %08x index %08x; -> %08x\n",
          metadata->virtAddr, index, entryAddress);
#endif
  return (simpleEntry*)entryAddress;
}


/**
 * function to take a first level page table entry - section 
 * and remap it as second level 4KB small pages
 **/
void splitSectionToSmallPages(simpleEntry* pageTable, u32int virtAddr)
{
#ifdef PAGE_TABLE_DBG
  printf("splitSectionToSmallPages: 1st level PT @ %08x vAddr %08x\n", (u32int)pageTable, virtAddr);
#endif

  // 1. get section entry
  sectionEntry* sectionEntryPtr = (sectionEntry*)getEntryFirst(pageTable, virtAddr);
#ifdef PAGE_TABLE_DBG
  printf("splitSectionToSmallPages: section entry @ %08x = %08x\n",
        (u32int)sectionEntryPtr, *(u32int*)sectionEntryPtr);
#endif
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
  
#ifdef PAGE_TABLE_DBG
  printf("splitSectionToSmallPages: vaddr %08x, pAddr %08x\n", virtAddr, physAddr);
#endif

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
  GCONTXT* context = getGuestContext();
#ifdef PAGE_TABLE_DBG
  printf("isAddrInPageTable: is physAddr %08x in PT %08x\n", physAddr, (u32int)pageTablePhys);
#endif
  if (pageTablePhys == 0)
  {
    // no guest page table yet.
    return FALSE;
  }

  // is the address in guests first level page table?
  if((physAddr >= (u32int)pageTablePhys) && 
     (physAddr <= ((u32int)pageTablePhys + PT1_SIZE - 1)) )
  {
#ifdef PAGE_TABLE_DBG
    printf("isAddrInPageTable: phys address points to a 1nd lvl page table entry\n");
#endif
    return TRUE;
  }

  // maybe second level page tables live in this section?
  ptInfo* head = context->pageTables->gptInfo;
  while (head != 0)
  {
    if ( (head->physAddr <= physAddr ) && ((head->physAddr+PT2_SIZE-1) >= physAddr) )
    {
#ifdef PAGE_TABLE_DBG
      printf("isAddrInPageTable: phys addr points to a 2nd lvl page table entry\n");
#endif
      return TRUE;
    }
    head = head->nextEntry;
  }
  // otherwise, not found - return false

  return FALSE; 
}


void addPageTableInfo(pageTableEntry* entry, u32int virtual, u32int physical, u32int mapped, bool host)
{
#ifdef PAGE_TABLE_DBG
  printf("addPageTableInfo: entry %08x @ %p, PA %08x VA %08x, mapped %08x host %x\n",
        *(u32int*)entry, entry, physical, virtual, mapped, host);
#endif
  GCONTXT* context = getGuestContext();
  
  ptInfo* newEntry = (ptInfo*)malloc(sizeof(ptInfo));
#ifdef PAGE_TABLE_DBG
  printf("addPageTableInfo: new entry @ %p\n", newEntry);
#endif
  newEntry->firstLevelEntry = entry;
  newEntry->physAddr = physical;
  newEntry->virtAddr = virtual;
  newEntry->host = host;
  newEntry->mappedMegabyte = mapped;
  newEntry->nextEntry = 0;

  ptInfo* head = (host) ? context->pageTables->sptInfo : context->pageTables->gptInfo;
  if (head == 0)
  {
    // first entry
    if (host)
    {
#ifdef PAGE_TABLE_DBG
      printf("addPageTableInfo: first entry, sptInfo now %p\n", newEntry);
#endif
      context->pageTables->sptInfo = newEntry;
    }
    else
    {
#ifdef PAGE_TABLE_DBG
      printf("addPageTableInfo: first entry, gptInfo now %p\n", newEntry);
#endif
      context->pageTables->gptInfo = newEntry;
    }
  }
  else
  {
    // loop to the end of the list
#ifdef PAGE_TABLE_DBG
    printf("addPageTableInfo: not the first entry. head %p\n", head);
#endif
    while (head->nextEntry != 0)
    {
      head = head->nextEntry;
    }
    head->nextEntry = newEntry;
  }
  dumpPageTableInfo();
  // done.
}


ptInfo* getPageTableInfo(pageTableEntry* firstLevelEntry)
{
#ifdef PAGE_TABLE_DBG
  printf("getPageTableInfo: first level entry ptr %p = %08x\n", firstLevelEntry, *(u32int*)firstLevelEntry);
#endif
  GCONTXT* context = getGuestContext();

  // spt first
  ptInfo* head = context->pageTables->sptInfo;
  while (head != 0)
  {
    if (head->firstLevelEntry == firstLevelEntry)
    {
#ifdef PAGE_TABLE_DBG
      printf("getPageTableInfo: found spt; entry %p = %08x\n", head->firstLevelEntry, *(u32int*)head->firstLevelEntry);
#endif
      return head;
    }
    head = head->nextEntry;
  }

  // its not spt, gpt then? 
  head = context->pageTables->gptInfo;
  while (head != 0)
  {
    if (head->firstLevelEntry == firstLevelEntry)
    {
#ifdef PAGE_TABLE_DBG
      printf("getPageTableInfo: found gpt; entry %p = %08x\n", head->firstLevelEntry, *(u32int*)head->firstLevelEntry);
#endif
      return head;
    }
    head = head->nextEntry;
  }

  // not found, return 0
  return 0;
}


void removePageTableInfo(pageTableEntry* firstLevelEntry, bool host)
{
#ifdef PAGE_TABLE_DBG
  printf("removePageTableInfo: first level entry ptr %p = %08x\n", firstLevelEntry, *(u32int*)firstLevelEntry);
#endif
  GCONTXT* context = getGuestContext();

  ptInfo* head = 0;
  ptInfo* prev = 0;
  if (host)
  {
    head = context->pageTables->sptInfo;
  }
  else
  {
    head = context->pageTables->gptInfo;
  }

  while (head != 0)
  {
    if (head->firstLevelEntry == firstLevelEntry)
    {
#ifdef PAGE_TABLE_DBG
      printf("removePageTableInfo: found entry %p = %08x\n", head->firstLevelEntry, *(u32int*)head->firstLevelEntry);
#endif
      ptInfo* tmp = head;
      head = head->nextEntry;
      if (host)
      {
        free((void*)tmp->virtAddr);
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
      free((void*)tmp);
      return;
    }
    prev = head; 
    head = head->nextEntry;
  }
}


void dumpPageTableInfo()
{
#ifdef PAGE_TABLE_DBG
  printf("dumpPageTableInfo:\n");
  
  GCONTXT* context = getGuestContext();
  // spt first
  ptInfo* head = context->pageTables->sptInfo;
  printf("sptInfo:\n");
  while (head != 0)
  {
    printf("%p: ptEntry %p; PA %08x VA %08x host %x\n", head, head->firstLevelEntry, head->physAddr, head->virtAddr, head->host);
    head = head->nextEntry;
  }

  // gpt then 
  head = context->pageTables->gptInfo;
  printf("gptInfo:\n");
  while (head != 0)
  {
    printf("%p: ptEntry %p; PA %08x VA %08x host %x\n", head, head->firstLevelEntry, head->physAddr, head->virtAddr, head->host);
    head = head->nextEntry;
  }
#endif
}


void invalidatePageTableInfo()
{
#ifdef PAGE_TABLE_DBG
  printf("invalidatePageTableInfo:\n");
#endif  
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
#ifdef PAGE_TABLE_DBG
  printf("invalidatePageTableInfo: ...done\n");
#endif
}


/**
 * Called from the instruction emulator when we have a permission abort
 * and the guest is writing to its own page table
 **/
void pageTableEdit(u32int address, u32int newVal)
{
  GCONTXT* context = getGuestContext();

  u32int virtualAddress;
  pageTableEntry* pageTableEntryGuest = 0;
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
#ifdef PAGE_TABLE_DBG
  printf("PageTableEdit: address %08x newval: %08x\n", address, newVal);
  printf("PageTableEdit: physical address of edit %08x, phys gPT %p\n", 
        physicalAddress, context->pageTables->guestPhysical);
#endif

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
#ifdef PAGE_TABLE_DBG
    printf("pageTableEdit: PT2 case: editAddr masked to PT2 size %08x\n", editAddr);
#endif
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
      DIE_NOW(context, "pageTableEdit: guest editing second lvl PT, couldn't find metadata.\n");
    }
#ifdef PAGE_TABLE_DBG
    printf("pageTableEdit: FOUND virt %08x phys %08x entry %08x @ %08x\n", 
          head->virtAddr, head->physAddr, *(u32int*)head->firstLevelEntry, (u32int)head->firstLevelEntry);
#endif
    if (context->pageTables->guestVirtual == 0)
    {
      DIE_NOW(context, "pageTableEdit: guestVirtual not set\n");
    }
    pageTableEntryGuest = head->firstLevelEntry;
    virtualAddress = ((u32int)head->firstLevelEntry - (u32int)context->pageTables->guestVirtual) << 18;
    virtualAddress |= ((address & 0x3FC) << 10);
#ifdef PAGE_TABLE_DBG
    printf("pageTableEdit: PT2 case: pt edit corresponds to VA %08x\n", virtualAddress);
#endif
  }


#ifdef PAGE_TABLE_DBG
  printf("pageTableEdit: virtualAddr %08x oldguestEntry %08x newGuestEntry %08x\n",
          virtualAddress, *(u32int*)oldGuestEntry, *(u32int*)newGuestEntry);
#endif

  if (firstLevelEntry && (oldGuestEntry->type == RESERVED))
  {
    printf("pageTableEdit: addr %08x newVal %08x vAddr %08x\n", address, newVal, virtualAddress);
    printf("pageTableEdit: oldGuestEntry %08x newGuestEntry %08x\n",
           *(u32int*)oldGuestEntry, *(u32int*)newGuestEntry);
    DIE_NOW(context, "pageTableEdit: old entry RESERVED type");
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
          printf("pageTableEdit: addr %08x newVal %08x\n", address, newVal);
          printf("pageTableEdit: virtualAddr %08x oldguestEntry %08x newGuestEntry %08x\n",
                          virtualAddress, *(u32int*)oldGuestEntry, *(u32int*)newGuestEntry);
          printf("pageTableEdit: shadowUser %08x @ %p shadowPriv %08x @ %p\n",
            *(u32int*)shadowUser, shadowUser, *(u32int*)shadowPriv, shadowPriv);
          shadowUser = getEntrySecond((pageTableEntry*)shadowUser, virtualAddress);
          shadowPriv = getEntrySecond((pageTableEntry*)shadowPriv, virtualAddress);
          printf("pageTableEdit: 2nd lvl shadowUser %08x @ %p shadowPriv %08x @ %p\n",
            *(u32int*)shadowUser, shadowUser, *(u32int*)shadowPriv, shadowPriv);
          DIE_NOW(context, "pageTableEdit: remove/change type 2nd lvl entry unimplemented.\n");
        }
      }
    }

    if ((oldGuestEntry->type == FAULT) && (newGuestEntry->type != FAULT))
    {
      // old entry fault, new !fault. add entry case ignore.
#ifdef PAGE_TABLE_DBG
      printf("pageTableEdit: old entry fault, new entry !fault. ignore ADD.\n");
#endif 
      return;
    }

    if ((oldGuestEntry->type != FAULT) && (newGuestEntry->type != FAULT))
    {
      // old entry fault, new !fault. changing page table entry type. mustn't ignore
      DIE_NOW(0, "pageTableEdit: old entry !fault, new entry !fault. change type unimplemented.\n");
    }
  }
  else
  {
    // type of old entry is the same as new one. just changing attributes!
#ifdef PAGE_TABLE_DBG
    printf("pageTableEdit: attributes @ %08x, oldValue %08x, newValue %08x\n", address, *(u32int*)address, newVal);
    printf("pageTableEdit: shadowPriv %08x shadowUser %08x\n", *(u32int*)shadowPriv, *(u32int*)shadowUser); 
#endif
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
          DIE_NOW(context, "pageTableEdit: editing attributes LARGE_PAGE: unimplemented.\n");
        }
        case FAULT:
        {
          DIE_NOW(context, "pageTableEdit: editing attributes FAULT type?.\n");
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
#ifdef PAGE_TABLE_DBG
  printf("editAttributesSection: oldSection %08x, newSection %08x shadow %08x\n",
         *(u32int*)oldSection, *(u32int*)newSection, *(u32int*)shadow);
#endif
  GCONTXT* context = getGuestContext();

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
      DIE_NOW(context, "editAttributesSection: shadow entry SPLIT.\n");
    }
    else
    {
      DIE_NOW(context, "editAttributesSection: remap address or supersection\n");
      // must shadow unmap and shadow re-map section.
    }
  }

  if (oldSection->b != newSection->b)
  {
    DIE_NOW(context, "editAttributesSection: edit bufferable bit\n");
  }

  if (oldSection->c != newSection->c)
  {
    DIE_NOW(context, "editAttributesSection: edit cacheable bit\n");
  }

  if (oldSection->xn != newSection->xn)
  {
    if (shadow->type != SECTION)
    {
      DIE_NOW(context, "editAttributesSection: map XN, shadow entry SPLIT.");
    }
    else
    {
      DIE_NOW(context, "editAttributesSection: edit XN bit\n");
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
    DIE_NOW(context, "editAttributesSection: imp-dep bit set");
  }

  if (oldSection->tex != newSection->tex)
  {
    DIE_NOW(context, "editAttributesSection:, edit TEX bits\n");
  }

  if ((oldSection->ap10 != newSection->ap10) || (oldSection->ap2 != newSection->ap2))
  {
    mapAPBitsSection(newSection, (simpleEntry*)shadow, virtual);
  }

  if (oldSection->s != newSection->s)
  {
    DIE_NOW(context, "editAttributesSection: edit details, edit Shareable bit\n");
  }

  if (oldSection->nG != newSection->nG)
  {
    DIE_NOW(context, "editAttributesSection: edit details, edit non-Global bit\n");
  }

  if (oldSection->ns != newSection->ns)
  {
    DIE_NOW(context, "pageTableEdit: edit details, edit non-secure bit\n");
  }
}


void editAttributesPageTable(pageTableEntry* oldTable, pageTableEntry* newTable, pageTableEntry* shadowTable, u32int virtual)
{
  printf("editAttributesPageTable: for virtual address %08x; implement\n", virtual);
  DIE_NOW(0, "editAttributesPageTable: unimplemented\n");
}


void editAttributesSmallPage(smallPageEntry* oldPage, smallPageEntry* newPage, smallPageEntry* shadowPage, u32int virtual)
{
#ifdef PAGE_TABLE_DBG
  printf("editAttributesSmallPage: oldPage %08x, newPage %08x shadowPage %08x\n",
         *(u32int*)oldPage, *(u32int*)newPage, *(u32int*)shadowPage);
#endif
  GCONTXT* context = getGuestContext();

  if (shadowPage->type == FAULT)
  {
    // quick exit if not shadow mapped yet
    return;
  }
  printf("editAttributesSmallPage: oldPage %08x, newPage %08x shadowPage %08x\n",
         *(u32int*)oldPage, *(u32int*)newPage, *(u32int*)shadowPage);

  if (oldPage->addr != newPage->addr)
  {
    // changing base address of entry, remove then add operation.
    // must shadow unmap and shadow re-map section.
    DIE_NOW(context, "editAttributesSmallPage: remap address\n");
  }

  if (oldPage->b != newPage->b)
  {
    DIE_NOW(context, "editAttributesSmallPage: edit bufferable bit\n");
  }

  if (oldPage->c != newPage->c)
  {
    DIE_NOW(context, "editAttributesSmallPage: edit cacheable bit\n");
  }

  if (oldPage->xn != newPage->xn)
  {
    DIE_NOW(context, "editAttributesSmallPage: edit XN bit\n");
  }

  if ((oldPage->ap10 != newPage->ap10) || (oldPage->ap2 != newPage->ap2))
  {
    DIE_NOW(context, "editAttributesSmallPage: edit AP bits\n");
//    mapAPBitsSection(newSection, (simpleEntry*)shadow, virtual);
  }

  if (oldPage->tex != newPage->tex)
  {
    DIE_NOW(context, "editAttributesSmallPage:, edit TEX bits\n");
  }

  if (oldPage->s != newPage->s)
  {
    DIE_NOW(context, "editAttributesSmallPage: edit details, edit Shareable bit\n");
  }

  if (oldPage->nG != newPage->nG)
  {
    DIE_NOW(context, "editAttributesSmallPage: edit details, edit non-Global bit\n");
  }
}
