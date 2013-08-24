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
#include "memoryManager/pageTableInfo.h"
#include "memoryManager/shadowMap.h"
#include "memoryManager/stack.h"


/**
 * allocate space for a new level 1 base page table
 **/
simpleEntry *newLevelOnePageTable()
{
  simpleEntry *pageTable = (simpleEntry *)memalign(1 << PT1_ALIGN_BITS, PT1_SIZE);
  if (pageTable == NULL)
  {
    DIE_NOW(NULL, "failed to allocate L1 page table");
  }
  ASSERT(isAlignedToMask(pageTable, PT1_ALIGN_MASK), "new L1 page table is not correctly aligned");
  // STARFIX: remove memsets!
  memset(pageTable, 0, PT1_SIZE);
  DEBUG(MM_PAGE_TABLES, "newLevelOnePageTable: Page Table base addr: %p" EOL, pageTable);
  return pageTable;
}


/**
 * allocate space for a new level 2 base page table
 **/
u32int *newLevelTwoPageTable()
{
  u32int *pageTable = (u32int *)memalign(1 << PT2_ALIGN_BITS, PT2_SIZE);
  if (pageTable == NULL)
  {
    DIE_NOW(NULL, "failed to allocate L2 page table");
  }
  ASSERT(isAlignedToMask(pageTable, PT2_ALIGN_MASK), "new L2 page table is not correctly aligned");
  // STARFIX; remove memsets
  memset(pageTable, 0, PT2_SIZE);
  DEBUG(MM_PAGE_TABLES, "newLevelTwoPageTable: Page Table base addr: %p" EOL, pageTable);
  return pageTable;
}


/**
 * frees an allocated level two page table and removes the corresponding
 * meta data information from the list. only called for 2nd level SHADOW page
 * tables. we can't delete guest page tables, not our memory!
 **/
void deleteLevelTwoPageTable(GCONTXT *context, pageTableEntry *pageTable)
{
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


u32int mapRegion(simpleEntry *pageTable, u32int virtualStartAddress, u32int physicalStartAddress,
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
    else if (isAlignedToMaskN(physicalStartAddress, LARGE_PAGE_MASK)
             && (physicalEndAddress - physicalStartAddress) >= LARGE_PAGE_SIZE)
    {
      // 64 kB, map a large page -- may involve creation of an L2 table 
      mapLargePage(pageTable, virtualStartAddress, physicalStartAddress, domain, accessBits,
                   cacheable, bufferable, regionAttributes, executeNever);
      physicalStartAddress += LARGE_PAGE_SIZE;
      virtualStartAddress += LARGE_PAGE_SIZE;
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
#ifndef CONFIG_DISABLE_HYPERVISOR_MEMORY_PROTECTION
  /*
   * Make use of MMU protection mechanisms to protect the hypervisor against guests and itself.
   * Make as few memory as possible executable: only the code of the hypervisor and instruction
   * caches, if we have them (only with block copy). See linker.h for information on how the
   * hypervisor memory image is split and how different parts should be mapped.
   *
   * WARNING: the implementation below relies on things being done in a particular order. Do not
   *          randomly swap things around and read the comments.
   *
   * 1   We need to make sure that the stuff we get from the linker makes sense (the linker usually
   *     either works or dies, but sometimes silently messes up). Because the information we need
   *     is only determined at link-time, we cannot use static assertions (and the point is to
   *     check from the C code... which is where we actually use the information from the linker).
   *     This also protects against potentially harmful modifications of the linker script.
   *
   * 1.1 Make sure that all the ELF sections and dynamic allocation pools are in the order we
   *     expect them to be.
   */
  ASSERT(MEMORY_START_ADDR <= HYPERVISOR_BEGIN_ADDRESS, ERROR_NOT_IMPLEMENTED);
  ASSERT(HYPERVISOR_BEGIN_ADDRESS < HYPERVISOR_END_ADDRESS, ERROR_NOT_IMPLEMENTED);
  ASSERT(HYPERVISOR_END_ADDRESS <= MEMORY_END_ADDR, ERROR_NOT_IMPLEMENTED);
  ASSERT(HYPERVISOR_BEGIN_ADDRESS == HYPERVISOR_TEXT_BEGIN_ADDRESS, ERROR_NOT_IMPLEMENTED);
  ASSERT(HYPERVISOR_TEXT_BEGIN_ADDRESS < HYPERVISOR_TEXT_END_ADDRESS, ERROR_NOT_IMPLEMENTED);
  ASSERT(HYPERVISOR_TEXT_END_ADDRESS <= HYPERVISOR_RO_XN_BEGIN_ADDRESS, ERROR_NOT_IMPLEMENTED);
  ASSERT(HYPERVISOR_RO_XN_BEGIN_ADDRESS < HYPERVISOR_RO_XN_END_ADDRESS, ERROR_NOT_IMPLEMENTED);
  ASSERT(HYPERVISOR_RO_XN_BEGIN_ADDRESS <= HYPERVISOR_RW_XN_BEGIN_ADDRESS, ERROR_NOT_IMPLEMENTED);
  ASSERT(HYPERVISOR_RW_XN_BEGIN_ADDRESS < HYPERVISOR_RW_XN_END_ADDRESS, ERROR_NOT_IMPLEMENTED);
  ASSERT(HYPERVISOR_RW_XN_END_ADDRESS == HYPERVISOR_END_ADDRESS, ERROR_NOT_IMPLEMENTED);
  ASSERT(HYPERVISOR_RW_XN_END_ADDRESS <= RAM_CODE_CACHE_POOL_BEGIN, ERROR_NOT_IMPLEMENTED);
  ASSERT(RAM_CODE_CACHE_POOL_BEGIN < RAM_CODE_CACHE_POOL_END, ERROR_NOT_IMPLEMENTED);
  ASSERT(RAM_CODE_CACHE_POOL_END <= RAM_XN_POOL_BEGIN, ERROR_NOT_IMPLEMENTED);
  ASSERT(RAM_XN_POOL_BEGIN < RAM_XN_POOL_END, ERROR_NOT_IMPLEMENTED);
  ASSERT(RAM_XN_POOL_END <= MEMORY_END_ADDR, ERROR_NOT_IMPLEMENTED);
  /*
   * 1.2 Now, we have to make sure that the linker followed our alignment constraints. Without
   *     these constraints, regions with a separate purpose cannot be mapped with different
   *     attributes (access bits, XN, ...).
   *
   * NOTE: we also check whether the .text section is aligned on a section boundary (1 MB) because
   *       it is the start of the hypervisor.
   */
  ASSERT(isAlignedToMaskN(HYPERVISOR_TEXT_BEGIN_ADDRESS, SECTION_MASK), ERROR_NOT_IMPLEMENTED);
  ASSERT(isAlignedToMaskN(HYPERVISOR_RO_XN_BEGIN_ADDRESS, SMALL_PAGE_MASK), ERROR_NOT_IMPLEMENTED);
  ASSERT(isAlignedToMaskN(HYPERVISOR_RW_XN_BEGIN_ADDRESS, SMALL_PAGE_MASK), ERROR_NOT_IMPLEMENTED);
  ASSERT(isAlignedToMaskN(RAM_CODE_CACHE_POOL_BEGIN, SMALL_PAGE_MASK), ERROR_NOT_IMPLEMENTED);
  ASSERT(isAlignedToMaskN(RAM_CODE_CACHE_POOL_END, SMALL_PAGE_MASK), ERROR_NOT_IMPLEMENTED);
  ASSERT(isAlignedToMaskN(RAM_XN_POOL_BEGIN, SECTION_MASK), ERROR_NOT_IMPLEMENTED);
  ASSERT(isAlignedToMaskN(RAM_XN_POOL_END, SECTION_MASK), ERROR_NOT_IMPLEMENTED);
  /*
   * 1.3 We also want to protect our stacks in multiple ways:
   *     - they must not be executable;
   *     - we must be able to detect stack overflow and underflow.
   *
   *     Stacks on ARM are fully descending. This means that the beginning of the stack has the
   *     highest address, and the end has the lowest address. A stack overflow occurs when data is
   *     pushed on a stack beyond its end address; an underflow occurs when data is popped beyond
   *     its starting point. We can easily protect against overflow and underflow by not mapping
   *     the surrounding memory. We call these surrounding areas 'stack gaps'.
   *
   *     The smallest memory region we can map is a small page (4 kB). If we want to protect the
   *     stacks as described above, this means that each stack must be a multiple of 4 kB in size,
   *     and must be surrounded by two regions of 4 kB each, which are not mapped. Then, every
   *     stack overflow and underflow will cause a translation fault.
   *
   *     Stacks and stack gaps are 'allocated' in startup.S and reside in '.bss' -- at least, that
   *     is how it should be. For each stack, the preceeding gap is named after that stack. The
   *     last stack gap is called TOP_STACK_GAP.
   *
   *     Once again, the implementation for mapping the stacks is based on a fixed order, so first
   *     of all we verify that:
   *     - the stack gaps are in the expected ELF section / location;
   *     - the stack gaps are aligned to and sized like small pages;
   *     - the stack gaps occur in the expected order (as described above);
   *     - the stacks are aligned to small pages;
   *     - the stacks are in the expected ELF section / location;
   *     - the stacks occur in the expected order (SVC < ABT < UND < IRQ < FIQ).
   */
  ASSERT(HYPERVISOR_RW_XN_BEGIN_ADDRESS <= ABT_STACK_GAP, ERROR_NOT_IMPLEMENTED);
  ASSERT(HYPERVISOR_RW_XN_BEGIN_ADDRESS <= FIQ_STACK_GAP, ERROR_NOT_IMPLEMENTED);
  ASSERT(HYPERVISOR_RW_XN_BEGIN_ADDRESS <= IRQ_STACK_GAP, ERROR_NOT_IMPLEMENTED);
  ASSERT(HYPERVISOR_RW_XN_BEGIN_ADDRESS <= SVC_STACK_GAP, ERROR_NOT_IMPLEMENTED);
  ASSERT(HYPERVISOR_RW_XN_BEGIN_ADDRESS <= UND_STACK_GAP, ERROR_NOT_IMPLEMENTED);
  ASSERT(HYPERVISOR_RW_XN_BEGIN_ADDRESS <= TOP_STACK_GAP, ERROR_NOT_IMPLEMENTED);
  ASSERT(isAlignedToMaskN(ABT_STACK_GAP, SMALL_PAGE_MASK), ERROR_NOT_IMPLEMENTED);
  ASSERT(isAlignedToMaskN(FIQ_STACK_GAP, SMALL_PAGE_MASK), ERROR_NOT_IMPLEMENTED);
  ASSERT(isAlignedToMaskN(IRQ_STACK_GAP, SMALL_PAGE_MASK), ERROR_NOT_IMPLEMENTED);
  ASSERT(isAlignedToMaskN(SVC_STACK_GAP, SMALL_PAGE_MASK), ERROR_NOT_IMPLEMENTED);
  ASSERT(isAlignedToMaskN(UND_STACK_GAP, SMALL_PAGE_MASK), ERROR_NOT_IMPLEMENTED);
  ASSERT(isAlignedToMaskN(TOP_STACK_GAP, SMALL_PAGE_MASK), ERROR_NOT_IMPLEMENTED);
  ASSERT((ABT_STACK_GAP + SMALL_PAGE_SIZE) == ABT_STACK_LB, ERROR_NOT_IMPLEMENTED);
  ASSERT((FIQ_STACK_GAP + SMALL_PAGE_SIZE) == FIQ_STACK_LB, ERROR_NOT_IMPLEMENTED);
  ASSERT((IRQ_STACK_GAP + SMALL_PAGE_SIZE) == IRQ_STACK_LB, ERROR_NOT_IMPLEMENTED);
  ASSERT((SVC_STACK_GAP + SMALL_PAGE_SIZE) == SVC_STACK_LB, ERROR_NOT_IMPLEMENTED);
  ASSERT((UND_STACK_GAP + SMALL_PAGE_SIZE) == UND_STACK_LB, ERROR_NOT_IMPLEMENTED);
  ASSERT(isAlignedToMaskN(ABT_STACK_UB, SMALL_PAGE_MASK), ERROR_NOT_IMPLEMENTED);
  ASSERT(isAlignedToMaskN(FIQ_STACK_UB, SMALL_PAGE_MASK), ERROR_NOT_IMPLEMENTED);
  ASSERT(isAlignedToMaskN(IRQ_STACK_UB, SMALL_PAGE_MASK), ERROR_NOT_IMPLEMENTED);
  ASSERT(isAlignedToMaskN(SVC_STACK_UB, SMALL_PAGE_MASK), ERROR_NOT_IMPLEMENTED);
  ASSERT(isAlignedToMaskN(UND_STACK_UB, SMALL_PAGE_MASK), ERROR_NOT_IMPLEMENTED);
  ASSERT(ABT_STACK_UB <= HYPERVISOR_RW_XN_END_ADDRESS, ERROR_NOT_IMPLEMENTED);
  ASSERT(FIQ_STACK_UB <= HYPERVISOR_RW_XN_END_ADDRESS, ERROR_NOT_IMPLEMENTED);
  ASSERT(IRQ_STACK_UB <= HYPERVISOR_RW_XN_END_ADDRESS, ERROR_NOT_IMPLEMENTED);
  ASSERT(SVC_STACK_UB <= HYPERVISOR_RW_XN_END_ADDRESS, ERROR_NOT_IMPLEMENTED);
  ASSERT(UND_STACK_UB <= HYPERVISOR_RW_XN_END_ADDRESS, ERROR_NOT_IMPLEMENTED);
  ASSERT((TOP_STACK_GAP + SMALL_PAGE_SIZE) <= HYPERVISOR_RW_XN_END_ADDRESS, ERROR_NOT_IMPLEMENTED);
  ASSERT(SVC_STACK_UB == ABT_STACK_GAP, ERROR_NOT_IMPLEMENTED);
  ASSERT(ABT_STACK_UB == UND_STACK_GAP, ERROR_NOT_IMPLEMENTED);
  ASSERT(UND_STACK_UB == IRQ_STACK_GAP, ERROR_NOT_IMPLEMENTED);
  ASSERT(IRQ_STACK_UB == FIQ_STACK_GAP, ERROR_NOT_IMPLEMENTED);
  ASSERT(FIQ_STACK_UB == TOP_STACK_GAP, ERROR_NOT_IMPLEMENTED);
  /*
   * 2   We should be careful about mapping small pages. All hypervisor translation tables are
   *     allocated from the non-executable dynamic memory allocation pool. When mapping small or
   *     large pages, a level 2 translation table (TT) is required. The mapping function expects
   *     the memory in which this level 2 TT resides to be mapped in the active L1 TT. Therefore,
   *     we must first map the pool from which TTs are allocated without using any L2 TTs, i.e.
   *     using sections only. Patching the mapping functions with extra checks to work around this
   *     problem is wasteful as this particular corner case is only hit when setting up new
   *     translation tables.
   *
   * NOTE: this is why RAM_XN_POOL_BEGIN and RAM_XN_POOL_END must be aligned on a section boundary.
   */
  mapRegion(pageTable, RAM_XN_POOL_BEGIN, RAM_XN_POOL_BEGIN, RAM_XN_POOL_END,
            HYPERVISOR_ACCESS_DOMAIN, PRIV_RW_USR_NO, TRUE, FALSE, 0, TRUE);
  /*
   * 3   Last but not least, we can map the rest of the hypervisor. Guests must not be able to
   *     access any of the regions mapped below.
   *
   * 3.1 We start by mapping '.text', the executable code. This region should be executable, and
   *     read-only for the hypervisor.
   */
  mapRegion(pageTable, HYPERVISOR_TEXT_BEGIN_ADDRESS, HYPERVISOR_TEXT_BEGIN_ADDRESS,
            HYPERVISOR_TEXT_END_ADDRESS, HYPERVISOR_ACCESS_DOMAIN, PRIV_RO_USR_NO, TRUE,
            FALSE, 0, FALSE);
  /*
   * 3.2 Next, we map '.rodata', statically allocated read-only data. This region must not be
   *     executable, and must be read-only for the hypervisor.
   */
  mapRegion(pageTable, HYPERVISOR_RO_XN_BEGIN_ADDRESS, HYPERVISOR_RO_XN_BEGIN_ADDRESS,
            HYPERVISOR_RO_XN_END_ADDRESS, HYPERVISOR_ACCESS_DOMAIN, PRIV_RO_USR_NO, TRUE, FALSE, 0,
            TRUE);
  /*
   * 3.3 We map all statically-allocated read/write data. This includes '.data' and '.bss', and
   *     hence the stacks and stack gaps. Since stacks and stack gaps may reside somewhere in the
   *     middle of '.bss', first map the region before the first stack gap. Next, skip all gaps
   *     and map the stacks, and finally, map the region following the last stack gap.
   */
  mapRegion(pageTable, HYPERVISOR_RW_XN_BEGIN_ADDRESS, HYPERVISOR_RW_XN_BEGIN_ADDRESS,
            SVC_STACK_GAP, HYPERVISOR_ACCESS_DOMAIN, PRIV_RW_USR_NO, TRUE, FALSE, 0, TRUE);
  mapRegion(pageTable, SVC_STACK_LB, SVC_STACK_LB, SVC_STACK_UB, HYPERVISOR_ACCESS_DOMAIN,
            PRIV_RW_USR_NO, TRUE, FALSE, 0, TRUE);
  mapRegion(pageTable, ABT_STACK_LB, ABT_STACK_LB, ABT_STACK_UB, HYPERVISOR_ACCESS_DOMAIN,
            PRIV_RW_USR_NO, TRUE, FALSE, 0, TRUE);
  mapRegion(pageTable, UND_STACK_LB, UND_STACK_LB, UND_STACK_UB, HYPERVISOR_ACCESS_DOMAIN,
            PRIV_RW_USR_NO, TRUE, FALSE, 0, TRUE);
  mapRegion(pageTable, IRQ_STACK_LB, IRQ_STACK_LB, IRQ_STACK_UB, HYPERVISOR_ACCESS_DOMAIN,
            PRIV_RW_USR_NO, TRUE, FALSE, 0, TRUE);
  mapRegion(pageTable, FIQ_STACK_LB, FIQ_STACK_LB, FIQ_STACK_UB, HYPERVISOR_ACCESS_DOMAIN,
            PRIV_RW_USR_NO, TRUE, FALSE, 0, TRUE);
  mapRegion(pageTable, TOP_STACK_GAP + SMALL_PAGE_SIZE, TOP_STACK_GAP + SMALL_PAGE_SIZE,
            HYPERVISOR_RW_XN_END_ADDRESS, HYPERVISOR_ACCESS_DOMAIN, PRIV_RW_USR_NO, TRUE, FALSE, 0,
            TRUE);
  /*
   * NOTE: The dynamic allocation pool for executable memory is NOT mapped here. It is meant to
   *       contain per-guest code caches, and must be mapped for each guest independently to ensure
   *       guests cannot read or execute from each other's caches.
   */
#else
  /*
   * Memory protection is disabled -- map the entire hypervisor read/write and executable.
   */
  mapRegion(pageTable, HYPERVISOR_BEGIN_ADDRESS, HYPERVISOR_BEGIN_ADDRESS, MEMORY_END_ADDR,
            HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, TRUE, FALSE, 0, FALSE);
#endif
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
  /*
   * FIXME This looks horribly wrong in a generic "map small page" that is also used for the
   * hypervisor PT !?
   */
  GCONTXT *context = getActiveGuestContext();

  DEBUG(MM_PAGE_TABLES, "mapSmallPage: Virtual %#.8x, physical %#.8x, dom: %x, AP: %x, c: %x, "
        "b: %x, tex: %x, xn: %x" EOL, virtAddr, physical, domain, accessBits, c, b, tex, xn);

  // First check 1st Level page table entry
  simpleEntry* first = getEntryFirst(pageTable, virtAddr);
  DEBUG(MM_PAGE_TABLES, "mapSmallPage: first entry %#.8x @ %p" EOL, *(u32int *)first, first);
  switch (first->type)
  {
    case FAULT:
    {
      DEBUG(MM_PAGE_TABLES, "mapSmallPage: entry for given VA is FAULT. Creating new" EOL);
      // we need a new second level page table. This gives a virtual address allocated
      u32int *vAddr = newLevelTwoPageTable();
      // need to get the physical address
      u32int pAddr = getPhysicalAddress(context, pageTable, (u32int)vAddr);
      DEBUG(MM_PAGE_TABLES, "mapSmallPage: PT VA %p PA %#.8x" EOL, vAddr, pAddr);
      // store metadata
      addPageTableEntry((pageTableEntry*)first, (u32int)pAddr, domain);
      u32int mapped = ((u32int)first - (u32int)pageTable) << 18;
      addPageTableInfo(context, (pageTableEntry*)first, (u32int)vAddr, pAddr, mapped, TRUE);
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
  simpleEntry *second = getEntrySecond(context, (pageTableEntry *)first, virtAddr);
  DEBUG(MM_PAGE_TABLES, "mapSmallPage: 2nd lvl entry @ %p = %#.8x" EOL, second, *(u32int *)second);
  // Again, we need to check the existing entry
  if (second->type == FAULT)
  {
    DEBUG(MM_PAGE_TABLES, "mapSmallPage: 2nd Level FAULT. Creating new small entry" EOL);
    addSmallPageEntry((smallPageEntry*)second, physical, accessBits, c, b, tex, xn);
  }
  else //Existing small or large page
  {
    DEBUG(MM_PAGE_TABLES, "mapSmallPage: 2nd Level exists, adding on top" EOL);
    addSmallPageEntry((smallPageEntry*)second, physical, accessBits, c, b, tex, xn);
  }
}


/**
 * Add a large page mapping of given virtual to physical address
 * into a second page table
 **/
void mapLargePage(simpleEntry *pageTable, u32int virtAddr, u32int physical, u8int domain,
                  u8int accessBits, bool cacheable, bool bufferable, u8int tex, bool executeNever)
{
  /*
   * FIXME This looks horribly wrong in a generic "map small page" that is also used for the
   * hypervisor PT !?
   */
  GCONTXT *context = getActiveGuestContext();

  DEBUG(MM_PAGE_TABLES, "mapLargePage: Virtual %#.8x, physical %#.8x, dom: %x, AP: %x, c: %x, "
        "b: %x, tex: %x, xn: %x" EOL, virtAddr, physical, domain, accessBits, cacheable,
        bufferable, tex, executeNever);

  // First check 1st Level page table entry
  simpleEntry* first = getEntryFirst(pageTable, virtAddr);
  DEBUG(MM_PAGE_TABLES, "mapLargePage: first entry %#.8x @ %p" EOL, *(u32int *)first, first);
  switch (first->type)
  {
    case FAULT:
    {
      DEBUG(MM_PAGE_TABLES, "mapLargePage: entry for given VA is FAULT. Creating new" EOL);
      // we need a new second level page table. This gives a virtual address allocated
      u32int *vAddr = newLevelTwoPageTable();
      // need to get the physical address
      u32int pAddr = getPhysicalAddress(context, pageTable, (u32int)vAddr);
      DEBUG(MM_PAGE_TABLES, "mapLargePage: PT VA %p PA %#.8x" EOL, vAddr, pAddr);
      // store metadata
      addPageTableEntry((pageTableEntry*)first, (u32int)pAddr, domain);
      u32int mapped = ((u32int)first - (u32int)pageTable) << 18;
      addPageTableInfo(context, (pageTableEntry*)first, (u32int)vAddr, pAddr, mapped, TRUE);
      break;
    }
    case PAGE_TABLE:
    {
      // if existing mapping is pageTable type, we're ok already
      DEBUG(MM_PAGE_TABLES, "mapLargePage: entry for given VA is pageTable. Correct, first time" EOL);
      break;
    }
    case SECTION:
    {
      printf("mapLargePage: Virtual %#.8x, physical %#.8x, dom: %x, AP: %x, c: %x, b: %x, tex: %x, xn: %x" EOL,
             virtAddr, physical, domain, accessBits, cacheable, bufferable, tex, executeNever);
      printf("mapLargePage: first entry %#.8x @ %p" EOL, *(u32int*)first, first);
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
  simpleEntry *second = getEntrySecond(context, (pageTableEntry *)first, virtAddr);
  // Again, we need to check the existing entry, but now there's 16 of them
  // Don't bother doing it transaction-style since we crash and burn upon any error anyhow
  for (simpleEntry *const limit = second + 16; second < limit; ++second)
  {
    DEBUG(MM_PAGE_TABLES, "mapLargePage: 2nd lvl entry @ %p = %#.8x" EOL, second, *(u32int *)second);
    if (second->type == FAULT)
    {
      DEBUG(MM_PAGE_TABLES, "mapLargePage: 2nd Level FAULT. Creating new large entry" EOL);
      addLargePageEntry((largePageEntry*)second, physical, accessBits, cacheable, bufferable, tex, executeNever);
    }
    else //Existing small or large page
    {
      printf("mapLargePage: Virtual %#.8x, physical %#.8x, dom: %x, AP: %x, c: %x, b: %x, tex: %x, xn: %x" EOL,
             virtAddr, physical, domain, accessBits, cacheable, bufferable, tex, executeNever);
      printf("mapLargePage: first entry %#.8x @ %p" EOL, *(u32int*)first, first);
      printf("mapLargePage: 2nd lvl entry @ %#.8x = %#.8x" EOL, (u32int)second, *(u32int*)second);
      DIE_NOW(NULL, "adding over existing entry");
    }
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
 * adds a large page entry at a given place in the second level page table
 **/
void addLargePageEntry(largePageEntry *entry, u32int physical, u8int accessBits, bool cacheable,
                       bool bufferable, u8int tex, bool executeNever)
{
  entry->addr = (physical >> 16);
  entry->nG = 0;
  entry->s = 0;
  entry->ap2 = accessBits >> 2;
  entry->tex = (tex & 0x7);
  entry->ap10 = accessBits & 0x3;
  entry->c = cacheable;
  entry->b = bufferable;
  entry->type = 1;
  entry->xn = executeNever;
  DEBUG(MM_PAGE_TABLES, "addLargePageEntry: large descriptor written: %#.8x @ %p" EOL,
        *(u32int *)entry, entry);
}


/**
 * Allocates memory for a new second level page table
 * adds a new page table entry to a given place in a base page table
 */
void addPageTableEntry(pageTableEntry* pageTableEntryPtr, u32int physical, u8int domain)
{
  pageTableEntryPtr->type = PAGE_TABLE;
  pageTableEntryPtr->sbz2 = 0;
  pageTableEntryPtr->ns = 1;
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
u32int getPhysicalAddress(GCONTXT *context, simpleEntry* pageTable, u32int virtAddr)
{
  DEBUG(MM_PAGE_TABLES, "getPhysicalAddress for VA %#.8x in PT @ %p" EOL, virtAddr, pageTable);
  simpleEntry* entryFirst = getEntryFirst(pageTable, virtAddr);
  if (entryFirst->type == FAULT)
  {
    // may not be shadow mapped? try it.
    if (!shadowMap(context, virtAddr))
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
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
      }
      else
      {
        return (section->addr << 20) | (virtAddr & ~SECTION_MASK);
      }
      break;
    }
    case PAGE_TABLE:
    {
      simpleEntry* entrySecond = getEntrySecond(context, (pageTableEntry*)entryFirst, virtAddr);
      
      switch(entrySecond->type)
      {
        // may not be shadow mapped? try it.
        case FAULT:
        {
          if (!shadowMap(context, virtAddr))
          {
            printf("getPhysicalAddress for VA %#.8x in PT @ %#.8x" EOL, virtAddr, (u32int)pageTable);
            DIE_NOW(NULL, "failed to shadow map, lvl2");
          }
        }
        case LARGE_PAGE:
        {
          largePageEntry *largePage = (largePageEntry *)entrySecond;
          return (largePage->addr << 16) | (virtAddr & ~LARGE_PAGE_MASK);
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
simpleEntry* getEntrySecond(GCONTXT *context, pageTableEntry* firstLevelEntry, u32int virtAddr)
{
  DEBUG(MM_PAGE_TABLES, "getEntrySecond: 1st lvl PTE %#.8x @ %p; VA %#.8x" EOL,
        *(u32int *)firstLevelEntry, firstLevelEntry, virtAddr);

  u32int index = (virtAddr & 0x000FF000) >> 10;
  // to look through the 2nd lvl page table, we need to access it using VA
  // but we only have the physical address! find VA in pt metadata cache
  ptInfo* metadata = getPageTableInfo(context, firstLevelEntry);
  if (metadata == 0)
  {
    printf("getEntrySecond: metadata not found. 1st lvl PTE %#.8x @ %p; VA %#.8x" EOL,
                            *(u32int*)firstLevelEntry, firstLevelEntry, virtAddr);
    dumpPageTableInfo(context);
    DIE_NOW(NULL, "could not find PT2 metadata");
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


simpleEntry* getEntry(simpleEntry* pageTable, u32int virtAddr)
{
  simpleEntry* entry = getEntryFirst(pageTable, virtAddr);
  if (entry->type == PAGE_TABLE)
  {
    GCONTXT *context = getActiveGuestContext();
    entry = getEntrySecond(context, (pageTableEntry*)entry, virtAddr);
  }
  return entry;
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
bool isAddrInPageTable(GCONTXT *context, simpleEntry* pageTablePhys, u32int physAddr)
{
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


/**
 * Called from the instruction emulator when we have a permission abort
 * and the guest is writing to its own page table
 **/
void pageTableEdit(GCONTXT *context, u32int address, u32int newVal)
{
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
  u32int physicalAddress = getPhysicalAddress(context, context->pageTables->guestVirtual, address);
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
          removePageTableInfo(context, (pageTableEntry*)oldGuestEntry, FALSE);
          shadowUnmapPageTable(context, (pageTableEntry*)shadowPriv, (pageTableEntry*)oldGuestEntry, virtualAddress);
          shadowUnmapPageTable(context, (pageTableEntry*)shadowUser, (pageTableEntry*)oldGuestEntry, virtualAddress);
        }
        else if (oldGuestEntry->type == SECTION)
        {
          // changing a page table to a section or fault, need to remove PT entry.
          shadowUnmapSection(context, (simpleEntry*)shadowPriv, (sectionEntry*)oldGuestEntry, virtualAddress);
          shadowUnmapSection(context, (simpleEntry*)shadowUser, (sectionEntry*)oldGuestEntry, virtualAddress);
        }
        // nothing to do if old type was reserved.
      }
      else
      {
        if ((oldGuestEntry->type == SMALL_PAGE) || (oldGuestEntry->type == SMALL_PAGE_3))
        {
          if (shadowUser->type != FAULT)
          {
            shadowUser = getEntrySecond(context, (pageTableEntry*)shadowUser, virtualAddress);
            shadowUnmapSmallPage(context, (smallPageEntry*)shadowUser, (smallPageEntry*)oldGuestEntry, virtualAddress);
          }
          if (shadowPriv->type != FAULT)
          {
            shadowPriv = getEntrySecond(context, (pageTableEntry*)shadowPriv, virtualAddress);
            shadowUnmapSmallPage(context, (smallPageEntry*)shadowPriv, (smallPageEntry*)oldGuestEntry, virtualAddress);
          }
        }
        else
        {
          printf("pageTableEdit: addr %#.8x newVal %#.8x" EOL, address, newVal);
          printf("pageTableEdit: virtualAddr %#.8x oldguestEntry %#.8x newGuestEntry %#.8x" EOL,
                          virtualAddress, *(u32int*)oldGuestEntry, *(u32int*)newGuestEntry);
          printf("pageTableEdit: shadowUser %#.8x @ %p shadowPriv %#.8x @ %p" EOL,
            *(u32int*)shadowUser, shadowUser, *(u32int*)shadowPriv, shadowPriv);
          shadowUser = getEntrySecond(context, (pageTableEntry*)shadowUser, virtualAddress);
          shadowPriv = getEntrySecond(context, (pageTableEntry*)shadowPriv, virtualAddress);
          printf("pageTableEdit: 2nd lvl shadowUser %#.8x @ %p shadowPriv %#.8x @ %p" EOL,
            *(u32int*)shadowUser, shadowUser, *(u32int*)shadowPriv, shadowPriv);
          DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
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
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
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
          editAttributesSection(context, (sectionEntry*)oldGuestEntry, (sectionEntry*)newGuestEntry,
                                                               shadowPriv, virtualAddress);
          context->CPSR = cpsrUser; 
          editAttributesSection(context, (sectionEntry*)oldGuestEntry, (sectionEntry*)newGuestEntry,
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
          // editing attributes LARGE_PAGE
          DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
        }
        case FAULT:
        {
          // editing attributes FAULT type?
          DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
        }
      }
    } // else second-level entry ends 
  } // editing entry attributes ends
//  mmuClearTLBbyMVA(address);
  mmuInvalidateUTLB();
  mmuClearDataCache();
  mmuDataMemoryBarrier();
}


void editAttributesSection(GCONTXT *context, sectionEntry* oldSection, sectionEntry* newSection, simpleEntry* shadow, u32int virtual)
{
  // WARNING: shadow descriptor type might not correspond to guest descriptor type!!! 
  DEBUG(MM_PAGE_TABLES, "editAttributesSection: oldSection %#.8x, newSection %#.8x shadow %#.8x"
        EOL, *(u32int *)oldSection, *(u32int *)newSection, *(u32int *)shadow);

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
    ((sectionEntry*)shadow)->domain = newSection->domain;
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
    mapAPBitsSection(context, newSection, (simpleEntry*)shadow, virtual);
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
  DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
}


void editAttributesSmallPage(smallPageEntry* oldPage, smallPageEntry* newPage, smallPageEntry* shadowPage, u32int virtual)
{
  DEBUG(MM_PAGE_TABLES, "editAttributesSmallPage: oldPage %#.8x, newPage %#.8x shadowPage %#.8x"
        EOL, *(u32int *)oldPage, *(u32int *)newPage, *(u32int *)shadowPage);

  if (shadowPage->type == FAULT)
  {
    // quick exit if not shadow mapped yet
    return;
  }

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
