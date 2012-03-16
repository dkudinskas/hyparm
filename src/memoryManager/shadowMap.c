#include "memoryManager/shadowMap.h"
#include "memoryManager/memoryConstants.h"
#include "memoryManager/mmu.h"

#include "common/debug.h"
#include "common/assert.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "guestManager/guestContext.h"


extern u32int __START_MARKER__;
extern u32int __END_MARKER__;

/**
 * take a VA that caused a memory abort and try to add a shadow mapping
 * from the guest page table to both shadow page tables 
 **/
bool shadowMap(u32int virtAddr)
{
  GCONTXT* context = getGuestContext();
#ifdef SHADOWING_DEBUG
  printf("shadowMap: virtual address %08x\n", virtAddr);
#endif
  u32int backupEntry = 0;
  u32int tempEntry = 0;
  bool backedUp = FALSE;
  bool success = FALSE;
  simpleEntry* tempFirst = 0;
  simpleEntry* spt = context->pageTables->shadowActive;

  simpleEntry* gpt = 0;
  if (context->pageTables->guestVirtual != 0)
  {
    // cool. we have the VA already! use it. 
#ifdef SHADOWING_DEBUG
    printf("shadowMap: guestVirtual PT set, %p\n", context->pageTables->guestVirtual);
#endif
    gpt = context->pageTables->guestVirtual;
  }
  else
  {
#ifdef SHADOWING_DEBUG
    printf("shadowMap: guestVirtual PT not set. hack a 1-2-1 of %p\n",
                                  context->pageTables->guestPhysical);
#endif
    // crap, we haven't shadow mapped the gPT VA->PA mapping yet.
    // hack a 1-2-1 mapping for now.
    tempFirst = getEntryFirst(spt, (u32int)context->pageTables->guestPhysical);
    backupEntry = *(u32int*)tempFirst;
#ifdef SHADOWING_DEBUG
    printf("shadowMap: VA %08x backed up entry %08x @ %p\n", virtAddr, backupEntry, tempFirst);
#endif
    mapSection(context->pageTables->shadowActive, (u32int)context->pageTables->guestPhysical, 
              (u32int)context->pageTables->guestPhysical, HYPERVISOR_ACCESS_DOMAIN,
              HYPERVISOR_ACCESS_BITS, TRUE, FALSE, 0);
    mmuClearTLBbyMVA((u32int)context->pageTables->guestPhysical);
    gpt = context->pageTables->guestPhysical;
    tempEntry = *(u32int*)tempFirst;
#ifdef SHADOWING_DEBUG
    printf("shadowMap: gpt now set to %p\n", gpt);
#endif
    backedUp = TRUE;
  }


  simpleEntry* guestFirst = getEntryFirst(gpt, virtAddr);
#ifdef SHADOWING_DEBUG
  printf("shadowMap: first entry %08x @ %p\n", *(u32int*)guestFirst, guestFirst);
#endif
  
  switch(guestFirst->type)
  {
    case SECTION:
    {
#ifdef SHADOWING_DEBUG
      printf("shadowMap: guest first level entry section\n");
#endif
      u32int virtual = ((u32int)guestFirst - (u32int)context->pageTables->guestVirtual) << 18;
      sectionEntry* guestSection = (sectionEntry*)guestFirst;
      sectionEntry* shadowEntry  = (sectionEntry*)getEntryFirst(spt, virtAddr);
      shadowMapSection(guestSection, shadowEntry, virtual);
      if (shadowEntry->addr == 0x804)
      {
        printf("shadowMap section VA %08x\n", virtual);
        printf("shadowMap value now: %08x\n", *(u32int*)((virtual & 0xFFF00000) | 0x2a870));
      }
      success = TRUE;
      break;
    }
    case PAGE_TABLE:
    {
#ifdef SHADOWING_DEBUG
      printf("shadowMap: guest first level entry page table\n");
#endif
      // must check if PT2 entry was shadow mapped already. if not, shadow map PT2 
      simpleEntry* shadowFirst = getEntryFirst(spt, virtAddr);
      switch(shadowFirst->type)
      {
        case FAULT:
        {
#ifdef SHADOWING_DEBUG
          printf("shadowMap: shadow 1st lvl entry fault. need to shadowmap PT2\n");
#endif
          pageTableEntry* shadowPageTable  = (pageTableEntry*)shadowFirst;
          pageTableEntry* guestPageTable = (pageTableEntry*)guestFirst;
          shadowMapPageTable(guestPageTable, guestPageTable, shadowPageTable);
          break;
        }
        case PAGE_TABLE:
        {
#ifdef SHADOWING_DEBUG
          printf("shadowMap: shadow 1st lvl entry PT2. OK.\n");
#endif
          // already mapped as PT2 in shadow page tables. however, the guest PT2 metadata might not
          // be saved, in case the hypervisor used that mapping for its own purposes.
          // make sure gPT2 metadata is really entered!
          ptInfo* metadata = getPageTableInfo((pageTableEntry*)guestFirst);
          if (metadata == 0)
          {
            addPageTableInfo((pageTableEntry*)guestFirst, 0, ((pageTableEntry*)guestFirst)->addr << 10, FALSE);
          }
          break;
        }
        case SECTION:
        {
          // if shadow entry is section, something's wrong.
          DIE_NOW(context, "shadowMap: guest PT2 but shadow SECTION.\n");
          break;
        }
        case RESERVED:
        {
          // if shadow entry is section, something's wrong.
          DIE_NOW(context, "shadowMap: guest PT2 but shadow RESERVED.\n");
          break;
        }
      }

      // now we are sure the PT2 is shadow mapped
      // we must try to shadow map the small page
      // hack a 1-2-1 mapping of the gPT2 to shadow-copy an entry
      // will remove the temporary mapping afterwards
      u32int gptPhysAddr = getPhysicalAddress(context->pageTables->hypervisor, 
                                 (((pageTableEntry*)guestFirst)->addr << 10));
      simpleEntry* shadow = getEntryFirst(context->pageTables->shadowActive, gptPhysAddr);
      u32int backup = *(u32int*)shadow;
#ifdef SHADOWING_DEBUG
      printf("shadowMap: backed up PT2 entry %08x @ %p\n", backup, shadow);
#endif
      mapSection(context->pageTables->shadowActive, gptPhysAddr, gptPhysAddr,
           HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, TRUE, FALSE, 0);
      mmuClearTLBbyMVA(gptPhysAddr);

      u32int index = (virtAddr & 0x000FF000) >> 10;
      u32int guestSecondPtr = gptPhysAddr | index;
      simpleEntry* guestSecond = (simpleEntry*)guestSecondPtr;
#ifdef SHADOWING_DEBUG
      printf("shadowMap: guest 2nd lvl entry %08x @ %p\n", *(u32int*)guestSecond, guestSecond);
#endif
      switch (guestSecond->type)
      {
        case SMALL_PAGE:
        case SMALL_PAGE_3:
        {
          smallPageEntry* guestSmallPage = (smallPageEntry*)guestSecond;
          smallPageEntry* shadowSmallPage = (smallPageEntry*)getEntrySecond((pageTableEntry*)shadowFirst, virtAddr);
          shadowMapSmallPage(guestSmallPage, shadowSmallPage, ((pageTableEntry*)guestFirst)->domain);
          success = TRUE;
          if (shadowSmallPage->addr == 0x8042a)
          {
            printf("shadowMap small page VA %08x\n", virtAddr);
            printf("shadowMap value now: %08x\n", *(u32int*)((virtAddr & 0xFFFFF000) | 0x870));
          }
          break;
        }
        case LARGE_PAGE:
        {
          DIE_NOW(context, "found large page entry. investigate.\n");
          success = TRUE;
          break;
        }
        case FAULT:
        {
          // guest doesnt have this virtual address mapped.
          success = FALSE;
          break;
        }
      }
      // restore backed up entry
      *(u32int*)shadow = backup;
#ifdef SHADOWING_DEBUG
      printf("shadowMap: restored 2nd lvl backed up entry %08x @ %p\n", backup, shadow);
#endif
      mmuClearTLBbyMVA(gptPhysAddr);
      mmuDataBarrier();
      break;
    }
    case RESERVED:
    case FAULT:
    {
      // guest doesn't have a valid entry, can't shadow map
      success = FALSE;
      break;
    }
    default:
    {
      DIE_NOW(context, "shadowMap: invalid 1st lvl page table entry\n");
    }
  }

  // exception case: we backed up the entry, but shadowed that exact entry. 
  // dont want to restore old entry then!!
  if (backedUp)
  {
    if (tempEntry == *(u32int*)tempFirst)
    {
      // if we dont have gPT1 VA we must have backed up the lvl1 entry. restore now
      *(u32int*)tempFirst = backupEntry;
      mmuClearTLBbyMVA((u32int)context->pageTables->guestPhysical);
#ifdef SHADOWING_DEBUG
      printf("shadowMap: restore backed up entry %08x @ %p\n", backupEntry, tempFirst);
#endif
    }
#ifdef SHADOWING_DEBUG
    else
    {
      printf("shadowMap: tempEntry %08x, guestFirst %08x. DONT RESTORE\n", tempEntry, *(u32int*)tempFirst);
    }
#endif
  }
  return success;
}


/**
 * takes pointers to guest and shadow page table section entries
 * maps/copies data from guest to shadow entry
 **/
void shadowMapSection(sectionEntry* guest, sectionEntry* shadow, u32int virtual)
{
#ifdef SHADOWING_DEBUG
  printf("shadowMapSection: guest entry %08x @ %08x\n", *(u32int*)guest, (u32int)guest);
  printf("shadowMapSection: shadow entry %08x @ %08x\n",*(u32int*)shadow, (u32int)shadow);
  printf("shadowMapSection: virtual %08x\n", virtual);
#endif
  bool peripheral = FALSE;
  GCONTXT* context = getGuestContext();

  if(guest->superSection)
  {
    DIE_NOW(context, "shadowMapSection: copy supersection unimplemented\n");
  }

  // Address mapping
  u32int guestPhysAddr = guest->addr << 20;
  if ((guestPhysAddr < MEMORY_START_ADDR) || (guestPhysAddr >= MEMORY_END_ADDR))
  {
#ifdef SHADOWING_DEBUG
    printf("shadowMapSection: guestPhysAddr %08x\n", guestPhysAddr);
#endif
    peripheral = TRUE;
    if (shadow->type != FAULT)
    {
      DIE_NOW(context, "shadowMapSection: peripheral mapping already exists!\n");
    }
  }

  sectionEntry* host = (sectionEntry*)getEntryFirst(context->pageTables->hypervisor, guestPhysAddr);

  if (host->type != SECTION)
  {
    // if guest enabled virtual memory we can remove the physical
    // address protection, remap as section
    // if guest virtual addressing is NOT enabled, we can only get here
    // when its turning vmem ON, since function copies guest PT entry
    // so both cases this 'split granularity' protection can be removed

    // must invalidate any entry that is there
    u32int sectionAddr = guestPhysAddr & SECTION_MASK;
    host->type = FAULT;
    // .. then add a new section 1-2-1 mapping
    if (peripheral)
    {
      addSectionEntry((sectionEntry*)host, sectionAddr,
                    GUEST_ACCESS_DOMAIN, PRIV_RW_USR_NO, 0, 0, 0b000);
    }
    else
    {
      addSectionEntry((sectionEntry*)host, sectionAddr,
                    GUEST_ACCESS_DOMAIN, GUEST_ACCESS_BITS, 1, 0, 0b000);
    }
    host = (sectionEntry*)getEntryFirst(context->pageTables->hypervisor, guestPhysAddr);
  }
#ifdef SHADOWING_DEBUG
  printf("shadowMapSection: guest address mapped to host %08x\n", host->addr << 20);
#endif

  shadow->type = SECTION;
  shadow->addr = host->addr;
  shadow->c = peripheral ? 0 : guest->c;
//  shadow->c = guest->c;
  shadow->b = 0;
  shadow->s = 0;//guest->s;
  shadow->tex = 0b100;
  shadow->nG = guest->nG;
  shadow->ns = guest->ns;
  shadow->domain = GUEST_ACCESS_DOMAIN;
#ifdef SHADOWING_DEBUG
  printf("shadowMapSection: Shadow entry now @ %p = %08x\n", shadow, *(u32int*)shadow);
#endif

  if (guest->xn)
  {
    // execute never bit depends on guest entry XN and DACR
    shadow->xn = mapExecuteNeverBit(guest->domain, guest->xn); 
#ifdef SHADOWING_DEBUG
    printf("shadowMapSection: shadow xn bit %x\n", shadow->xn);
#endif
  }

  if(guest->imp)
  {
#ifdef SHADOWING_DEBUG
    printf("shadowMapSection: guest OS PT using the imp bit, investigate\n");
#endif
  }

  // maps AP bits to shadow entry, write-protects if necessary
  if (peripheral)
  {
    shadow->ap10 = PRIV_RW_USR_NO & 0x3;
    shadow->ap2  = PRIV_RW_USR_NO >> 2;
  }
  else
  {
    mapAPBitsSection(guest, (simpleEntry*)shadow, virtual);
  }
#ifdef SHADOWING_DEBUG
  printf("shadowMapSection: Shadow entry after AP map @ %08x = %08x\n", (u32int)shadow, *(u32int*)shadow);
#endif
}


/**
 * takes pointers to guest and shadow page table section entries
 * removes the shadow mapping for the corresponding guest entry
 **/
void shadowUnmapSection(simpleEntry* shadow, sectionEntry* guest, u32int virtual)
{
#ifdef SHADOWING_DEBUG
  printf("shadowUnmapSection: guest entry %08x @ %08x\n", *(u32int*)guest, (u32int)guest);
  printf("shadowUnmapSection: shadow entry %08x @ %08x\n",*(u32int*)shadow, (u32int)shadow);
  printf("shadowUnmapSection: virtual address %08x\n", virtual);
#endif
  GCONTXT* context = getGuestContext();

  if (shadow->type == FAULT)
  {
    // quick exit if entry hasn't been shadow mapped yet.
    return;
  }

  // is the guest trying to unmap a non-RAM address? must check not to screw up
  u32int physAddr = guest->addr << 20;
  if ((physAddr < MEMORY_START_ADDR) || (physAddr >= MEMORY_END_ADDR))
  {
    printf("shadowUnmapSection: physAddr %08x\n", physAddr);
    DIE_NOW(context, "shadowUnmapSection: not a RAM address, doublecheck\n");
  }

  // would gladly remove this entry, but must check if the guest didnt decide
  // to remove pte that maps the for the hypervisor
  u32int startAddr = (u32int)&__START_MARKER__;
  u32int endAddr = MEMORY_END_ADDR;
#ifdef SHADOWING_DEBUG
  printf("shadowUnmapSection: VA %08x PA %08x\n", virtual, physAddr);
#endif
  if ((startAddr <= virtual) && (virtual <= endAddr))
  {
    DIE_NOW(context, "shadowUnmapSection: Guest trying to unmap an address the hypervisor lives in\n");
  }

  // Need to flush block cache at these addresses first
  // but which addresses to flush? shadow entries might have been fragmented to pages from a section...
  // so flush address range that the guest mapped originally.
  validateCacheMultiPreChange(context->blockCache, virtual, (virtual+SECTION_SIZE-1));
  
  if (context->pageTables->guestVirtual != 0)
  {
    // if the current gPT1 base VA is in this section, lets 'forget' it in GC
    if ((virtual <= (u32int)context->pageTables->guestVirtual) && 
       ((u32int)context->pageTables->guestVirtual < (virtual + SECTION_SIZE)))
    {
      DIE_NOW(context, "shadowUnmapSection: unmapping entry that is used for gPT VA.");
    }
  }
  
  // if it's been split to small pages, must properly remove PT2
  if (shadow->type == PAGE_TABLE)
  {
    deleteLevelTwoPageTable((pageTableEntry*)shadow);
  }

  // and finally, remove the shadow entry!
  *(u32int*)shadow = 0;
  mmuClearTLBbyMVA(virtual);
  mmuDataBarrier();
}



/**
 * takes pointers to guest and shadow page table PAGE_TABLE type entries
 * creates a new shadow 2nd level page table and copies the guest mapping
 **/
void shadowMapPageTable(pageTableEntry* guest, pageTableEntry* guestOld, pageTableEntry* shadow)
{
#ifdef SHADOWING_DEBUG
  printf("shadowMapPageTable guest %08x @ %08x, shadow %08x @ %08x\n",
     *(u32int*)guest, (u32int)guest, *(u32int*)shadow, (u32int)shadow); 
  printf("shadowMapPageTable old guest %08x @ %08x\n", *(u32int*)guestOld, (u32int)guestOld);
#endif
  GCONTXT* context = getGuestContext();
  u32int sptVirtAddr = 0;
  u32int sptPhysAddr = 0;

  switch(shadow->type)
  {
    case FAULT:
    {
      // need to allocate a new shadow page table!
      if ((sptVirtAddr = (u32int)memalign(1 << PT2_ALIGN_BITS, PT2_SIZE)) == 0)
      {
        DIE_NOW(0, "shadowMapPageTable: Failed to allocate 2lvl shadow page table");
      }
      memset((void*)sptVirtAddr, 0x0, PT2_SIZE);
#ifdef SHADOWING_DEBUG
      printf("shadowMapPageTable: newPT2 @ %08x\n", sptVirtAddr);
#endif
  
      sptPhysAddr = getPhysicalAddress(context->pageTables->shadowActive, sptVirtAddr);
    
      // fill in the given 1st level entry with correct data 
      shadow->addr = sptPhysAddr >> 10;
      //This is just a copy of the high level descriptor
      shadow->type = PAGE_TABLE;
      shadow->domain = GUEST_ACCESS_DOMAIN;
      shadow->ns = guest->ns;
  
      addPageTableInfo(shadow, sptVirtAddr, sptPhysAddr, TRUE);
      break;
    }
    case PAGE_TABLE:
    {
      ptInfo* metadata = getPageTableInfo(shadow);
      if (metadata == 0)
      {
        DIE_NOW(context, "shadowMapPageTable: sPT2 metadata not found\n");
      }
      sptVirtAddr = metadata->virtAddr;
      break;
    }
    case SECTION:
    {
      DIE_NOW(context, "shadowMaPageTable: shadow entry already written, section.\n");
    }
    case RESERVED:
    {
      DIE_NOW(context, "shadowMaPageTable: shadow entry already written, reserved.\n");
    }
  } // shadow switch

  addPageTableInfo((pageTableEntry*)guestOld, 0, guest->addr << 10, FALSE);

  // ok. we must scan the shadow PT looking for previously shadow mapped entries
  // that point to this new guest 2nd lvl page table. if found, write-protect
  u32int gptPhysical = guest->addr << 10;
  simpleEntry* shadowBase = context->pageTables->shadowActive;
  u32int i = 0;
  for (i=0; i < PT1_ENTRIES; i++)
  {
    if (shadowBase[i].type == SECTION)
    {
      sectionEntry* sectionPtr = (sectionEntry*)&shadowBase[i]; 
      u32int section = sectionPtr->addr << 20;
      if ((section <= gptPhysical) && ((section + SECTION_SIZE - 1) >= gptPhysical))
      {
#ifdef SHADOWING_DEBUG
        printf("shadowMapPageTable: section %08x maps guest PT2 %08x\n", section, gptPhysical);
#endif
        // guest protect PT2
        u32int virtualAddress = i << 20;
        virtualAddress |= (gptPhysical & ~SECTION_MASK);
#ifdef SHADOWING_DEBUG
        printf("shadowMapPageTable: virtualAddress of gPT2 %08x\n", virtualAddress);
#endif
        guestWriteProtect(virtualAddress, virtualAddress + PT2_SIZE - 1);
      }
    }
    else if (shadowBase[i].type == PAGE_TABLE)
    {
      pageTableEntry* pageTablePtr = (pageTableEntry*)&shadowBase[i];
      ptInfo* metadata = getPageTableInfo(pageTablePtr);
      if (metadata == 0)
      {
        DIE_NOW(0, "shadowMapPageTable: sPT2 metadata not found while checking AP\n");
      }
      simpleEntry* tempPageTable = (simpleEntry*)(metadata->virtAddr);
      u32int y = 0;
      for (y = 0; y < PT2_ENTRIES; y++)
      {
        if (tempPageTable[y].type != FAULT)
        {
          if(tempPageTable[y].type == LARGE_PAGE)
          {
            DIE_NOW(0, "shadowMapPageTable: found guest LARGE_PAGE entry, investigate.\n");
          }
          else
          {
            // must calculate what VA guest is trying to map with this small page
            smallPageEntry* smallPage = (smallPageEntry*)&tempPageTable[y]; 
            u32int phys = smallPage->addr << 12;
            u32int physPageTableMasked = gptPhysical & SMALL_PAGE_MASK;
            if (phys == physPageTableMasked)
            {
              u32int virtualAddress = i << 20;
              virtualAddress |= (y << 12);
              guestWriteProtect(virtualAddress, virtualAddress + PT2_SIZE - 1);
            }
          }
        } // !fault
      } // for loop
    } // host PAGE_TABLE
  } // for loop

}


void shadowUnmapPageTable(pageTableEntry* shadow, pageTableEntry* guest, u32int virtual)
{
  printf("shadowUnmapPageTable: shadow %08x @ %p, guest %08x @ %p, VA %08x\n",
                   *(u32int*)shadow, shadow, *(u32int*)guest, guest, virtual);

  GCONTXT* context = getGuestContext();

  if (shadow->type == FAULT)
  {
    // entry has not been shadow mapped. quick return.
    return;
  }

  // would gladly remove this entry, but must check if the guest didnt decide
  // to remove pte that maps the for the hypervisor
  u32int startAddr = (u32int)&__START_MARKER__;
  u32int endAddr = MEMORY_END_ADDR;
  if ((startAddr <= virtual) && (virtual <= endAddr))
  {
    DIE_NOW(context, "shadowUnmapPageTable: Guest trying to unmap an address the hypervisor lives in\n");
  }

  // validate block cache...
  validateCacheMultiPreChange(context->blockCache, virtual, (virtual+SECTION_SIZE-1));

  if (context->pageTables->guestVirtual != 0)
  {
    // if the current gPT1 base VA is in this section, lets 'forget' it in GC
    if ((virtual <= (u32int)context->pageTables->guestVirtual) && 
       ((u32int)context->pageTables->guestVirtual < (virtual + SECTION_SIZE)))
    {
      DIE_NOW(context, "shadowUnmapPageTable: unmapping entry that is used for gPT VA.");
    }
  }

  deleteLevelTwoPageTable((pageTableEntry*)shadow);

  // and finally, remove the shadow entry!
  *(u32int*)shadow = 0;
  mmuClearTLBbyMVA(virtual);
  mmuDataBarrier();
}


/**
 * takes pointers to guest and shadow small page entries
 * shadow maps the given guest small page entry.
 **/
void shadowMapSmallPage(smallPageEntry* guest, smallPageEntry* shadow, u32int dom)
{
#ifdef SHADOWING_DEBUG
  printf("shadowMapSmallPage: guest %08x @ %08x; shadow %08x @ %08x\n",
     *(u32int*)guest, (u32int)guest, *(u32int*)shadow, (u32int)shadow);
  printf("shadowMapSmallPage: dom %x\n", dom);
#endif
  bool peripheral = FALSE;
  GCONTXT* context = getGuestContext();

  // Address mapping: guest entry points to guest physical. Translate to host physical
  u32int guestPhysical = (guest->addr << 12) & SMALL_PAGE_MASK;

  // Address mapping
  if ((guestPhysical < MEMORY_START_ADDR) || (guestPhysical >= MEMORY_END_ADDR))
  {
#ifdef SHADOWING_DEBUG
    printf("shadowMapSmallPage: guestPhysical %08x\n", guestPhysical);
#endif
    peripheral = TRUE;
    if (shadow->type != FAULT)
    {
      DIE_NOW(context, "shadowMapSmallPage: peripheral mapping already exists!\n");
    }
  }

  simpleEntry* hostEntry = (simpleEntry*)getEntryFirst(context->pageTables->hypervisor, guestPhysical);

  u32int hostPhysical = 0;
  switch (hostEntry->type)
  {
    case SECTION:
    {
      sectionEntry* hostSection = (sectionEntry*)hostEntry;
      hostPhysical = ((hostSection->addr << 20) & SECTION_MASK) |
                      (guestPhysical & (~(SECTION_MASK)));
      break;
    }
    case PAGE_TABLE:
    {
      simpleEntry* hostPage = getEntrySecond((pageTableEntry*)hostEntry, guestPhysical);
      switch (hostPage->type)
      {
        case SMALL_PAGE:
        case SMALL_PAGE_3:
        {
          smallPageEntry* hostSmallPage = (smallPageEntry*)hostPage;
          hostPhysical = (hostSmallPage->addr << 12) & SMALL_PAGE_MASK;
          break;
        }
        case LARGE_PAGE:
        case FAULT:
        default:
          DIE_NOW(context, "shadowMapSmallPage: host physical 2nd lvl unimplemented.");
      }
      break;
    }
    case FAULT:
    case RESERVED:
    default:
      DIE_NOW(context, "shadowMapSmallPage: invalid entry found translating guestPA to hostPA.");
  }
#ifdef SHADOWING_DEBUG
  printf("shadowMapSmallPage: guest physical %08x maps to host physical %08x\n",
         guestPhysical, hostPhysical);
#endif
  shadow->type = 1; // small page entry.
  shadow->addr = (hostPhysical >> 12);

  // maps AP bits to shadow entry, write-protects if necessary
  mapAPBitsSmallPage(dom, guest, shadow);
#ifdef SHADOWING_DEBUG
  printf("shadowMapSmallPage: Shadow entry after AP map @ %08x = %08x\n", (u32int)shadow, *(u32int*)shadow);
#endif

  if (guest->xn)
  {
    // execute never bit depends on guest entry XN and DACR
    shadow->xn = mapExecuteNeverBit(dom, guest->xn); 
#ifdef SHADOWING_DEBUG
    printf("shadowMapSmallPage: shadow xn bit %x\n", shadow->xn);
#endif
  }

  shadow->c = peripheral ? 0 : guest->c;
//  shadow->c = guest->c;
  shadow->b = 0; //guest->b;
  shadow->tex = 0b100;
  shadow->s = 0; //guest->s;
  shadow->nG = guest->nG;
#ifdef SHADOWING_DEBUG
  printf("shadowMapSmallPage: Shadow at the end @ %08x = %08x\n", (u32int)shadow, *(u32int*)shadow);
#endif
}


/**
 * removes a shadow-mapped small page entry. does all necessary checks
 **/
void shadowUnmapSmallPage(smallPageEntry* shadow, smallPageEntry* guest, u32int virtual)
{
//#ifdef SHADOWING_DEBUG
  printf("shadowUnmapSmallPage: guest entry %08x @ %08x\n", *(u32int*)guest, (u32int)guest);
  printf("shadowUnmapSmallPage: shadow entry %08x @ %08x\n",*(u32int*)shadow, (u32int)shadow);
  printf("shadowUnmapSmallPage: virtual address %08x\n", virtual);
//#endif
  GCONTXT* context = getGuestContext();

  if (shadow->type == FAULT)
  {
    // quick exit if entry hasn't been shadow mapped yet.
    return;
  }

  // is the guest trying to unmap a non-RAM address? must check not to screw up
  u32int physAddr = guest->addr << 12;
  if ((physAddr < MEMORY_START_ADDR) || (physAddr >= MEMORY_END_ADDR))
  {
    printf("shadowUnmapSmallPage: physAddr %08x\n", physAddr);
    DIE_NOW(context, "shadowUnmapSmallPage: not a RAM address, doublecheck\n");
  }
//#ifdef SHADOWING_DEBUG
  printf("shadowUnmapSmallPage: VA %08x PA %08x\n", virtual, physAddr);
//#endif
  // would gladly remove this entry, but must check if the guest didnt decide
  // to remove pte that maps the for the hypervisor
  u32int startAddr = (u32int)&__START_MARKER__;
  u32int endAddr = MEMORY_END_ADDR;
  if ((startAddr <= virtual) && (virtual <= endAddr))
  {
    DIE_NOW(context, "shadowUnmapSmallPage: Guest trying to unmap an address the hypervisor lives in\n");
  }

  // Need to flush block cache at these addresses first
  // but which addresses to flush? shadow entries might have been fragmented to pages from a section...
  // so flush address range that the guest mapped originally.
  validateCacheMultiPreChange(context->blockCache, virtual, (virtual+SMALL_PAGE_SIZE-1));

  if (context->pageTables->guestVirtual != 0)
  {
    // if the current gPT1 base VA is in this section, lets 'forget' it in GC
    if ((virtual & PT1_ALIGN_MASK) == (u32int)context->pageTables->guestVirtual)
    {
      DIE_NOW(context, "shadowUnmapSection: unmapping entry that is used for gPT VA.");
    }
  }
  
  // and finally, remove the shadow entry!
  *(u32int*)shadow = 0;
  mmuClearTLBbyMVA(virtual);
  mmuDataBarrier();
}


/**
 * maps access permission bits from guest entry to current active shadow
 * takes into account the settings in guest domain access control register
 **/
u32int mapAccessPermissionBits(u32int guestAP, u32int domain)
{
  GCONTXT* context = getGuestContext();
  
  u32int dacr = getCregVal(3, 0, 0, 0, &context->coprocRegBank[0]);
  u32int domBits = (dacr >> (domain*2)) & 0x3;
  u32int shadowAP = 0;
  bool guestPriv = isGuestInPrivMode(context);
  switch (domBits)
  {
    case DACR_NO_ACCESS:
    {
      // no access. AP bits priv R/W user N/A
      shadowAP = PRIV_RW_USR_NO;
      break;
    }
    case DACR_CLIENT:
    {
      // client access. actually map guest to shadow AP bits
      switch(guestAP)
      {
        case PRIV_NO_USR_NO:
          shadowAP = PRIV_RW_USR_NO;
          break;
        case PRIV_RW_USR_NO:
          shadowAP = guestPriv ? PRIV_RW_USR_RW : PRIV_RW_USR_NO;
          break;
        case PRIV_RW_USR_RO:
          shadowAP = guestPriv ? PRIV_RW_USR_RW : PRIV_RW_USR_RO;
          break;
        case PRIV_RW_USR_RW:
          shadowAP = PRIV_RW_USR_RW;
          break;
        case PRIV_RO_USR_NO:
          shadowAP = guestPriv ? PRIV_RW_USR_RO : PRIV_RW_USR_NO;
          break;
        case DEPRECATED:
          shadowAP = guestPriv ? PRIV_RW_USR_RW : PRIV_RW_USR_RO;
          break;
        case PRIV_RO_USR_RO:
          shadowAP = PRIV_RW_USR_RO;
          break;
        case AP_RESERVED:
        default:
          DIE_NOW(context, "mapAccessPermissionBits: Invalid access permission bits.");
      }
      break;
    }
    case DACR_RESERVED:
    {
      DIE_NOW(context, "mapAccessPermissionBits: domain - reserved.");
    }
    case DACR_MANAGER:
    {
      // manager access. allow all guest access
      shadowAP = PRIV_RW_USR_RW;
      break;
    }
  } // domain switch ends
  return shadowAP;
}


/**
 * Access control bit mapping from guest to shadow priv and shadow user
 * Checks if guest is not editing an entry that contains guest page tables
 **/
void mapAPBitsSection(sectionEntry* guest, simpleEntry* shadow, u32int virtual)
{
#ifdef SHADOWING_DEBUG
  printf("mapAPBitsSection: guest entry %08x @ %08x\n", *(u32int*)guest, (u32int)guest);
  printf("mapAPBitsSection: shadow entry %08x @ %08x\n", *(u32int*)shadow, (u32int)shadow);
  printf("mapAPBitsSection: virtual %08x\n", virtual);
#endif
  GCONTXT* context = getGuestContext();

  // get new access permission bits, that take into account guest DACR
  u32int sysCtrlReg = getCregVal(1, 0, 0, 0, &context->coprocRegBank[0]);
  if (sysCtrlReg & SYS_CTRL_ACCESS_FLAG)
  {
    DIE_NOW(context, "mapAPBitsSection: access flag enabled set, unimplemented.\n");
  }
  u32int guestAP = (guest->ap2 << 2) | guest->ap10;
  u32int shadowAP = mapAccessPermissionBits(guestAP, guest->domain);
#ifdef SHADOWING_DEBUG
  printf("mapAPBitsSection: guestAP %x, shadowAP %x\n", guestAP, shadowAP);
#endif

  switch(shadow->type)
  {
    case SECTION:
    {
      sectionEntry* section = (sectionEntry*)shadow;
      section->ap2  = (shadowAP >> 2) & 0x1;
      section->ap10 =  shadowAP & 0x3;
      break;
    }
    case PAGE_TABLE:
    {
      // loop through all second level entries adjusting mapping
      pageTableEntry* shadowPT = (pageTableEntry*)shadow;
      ptInfo* metadata = getPageTableInfo(shadowPT);
      if (metadata == 0)
      {
        DIE_NOW(context, "mapAPBitsSection: could not find required metadata for PT2.\n");
      }

      u32int i = 0;
      for (i = 0; i < PT2_ENTRIES; i++)
      {
        smallPageEntry* shadowSmallPage = (smallPageEntry*)(metadata->virtAddr + i*PT_ENTRY_SIZE);
        shadowSmallPage->ap2  = (shadowAP >> 2) & 0x1;
        shadowSmallPage->ap10 =  shadowAP & 0x3;
      }
      break;
    }
    case RESERVED:
    case FAULT:
    default:
      DIE_NOW(context, "mapAPBitsSection: shadow entry - fault/reserved.");
  } // switch ends


  // check if guest physical maps guest first level page table
  if ((u32int)(guest->addr << 20) == ((u32int)context->pageTables->guestPhysical & SECTION_MASK))
  {
    // 1st level page table lives in this section!
#ifdef SHADOWING_DEBUG
    printf("mapAPBitsSection: vAddr maps to memory that contains a 1st lvl gPT\n");
#endif
    u32int virtualAddress =  virtual | (((u32int)context->pageTables->guestPhysical) & (~SECTION_MASK));
#ifdef SHADOWING_DEBUG
    printf("mapAPBitsSection: guest PT virtual address is %08x\n", virtualAddress);
#endif
    guestWriteProtect(virtualAddress, virtualAddress+PT1_SIZE-1);
    // found a guest page table entry that maps its base page table!
    // lets update our GC pointer immediatelly
    context->pageTables->guestVirtual = (simpleEntry*)virtualAddress;
  }

  // maybe second level page tables live in this section?
  u32int guestPhysical = (guest->addr << 20);
  ptInfo* head = context->pageTables->gptInfo;
  while (head != 0)
  {
    if ((head->physAddr >= guestPhysical) &&
       ((head->physAddr+PT2_SIZE -1) <= (guestPhysical+SECTION_SIZE-1)) )
    {
#ifdef SHADOWING_DEBUG
      printf("mapAPBitsSection: vAddr maps to memory that contains a 2nd lvl gPT\n");
#endif
      u32int virtualAddress = virtual | (((u32int)head->physAddr) & (~SECTION_MASK));
      guestWriteProtect(virtualAddress, virtualAddress+PT2_SIZE-1);
    }
    head = head->nextEntry;
  } // while ends
}


/**
 * Access control bit mapping from guest to shadow priv and shadow user
 * Checks if guest is not editing an entry that contains guest page tables
 * updates all pages found in 2nd level page table: this op is often done upon
 * guest changing its DACR
 **/
void mapAPBitsPageTable(pageTableEntry* guest, pageTableEntry* shadow)
{
#ifdef SHADOWING_DEBUG
  printf("mapAPBitsPageTable: guest %08x @ %08x, shadow %08x @ %08x\n",
        *(u32int*)guest, (u32int)guest, *(u32int*)shadow, (u32int)shadow);
#endif
  GCONTXT* context = getGuestContext();

  // hack a 1-2-1 mapping of the gPT2 to shadow-copy any valid entries
  // then remove the temporary mapping afterwards
  u32int gptPhysAddr = getPhysicalAddress(context->pageTables->hypervisor, (guest->addr << 10));
  simpleEntry* first = getEntryFirst(context->pageTables->shadowActive, gptPhysAddr);
  u32int backupEntry = *(u32int*)first;
#ifdef SHADOWING_DEBUG
  printf("mapAPBitsPageTable: backed up entry %08x @ %p\n", backupEntry, first);
#endif
  mapSection(context->pageTables->shadowActive, gptPhysAddr, gptPhysAddr,
       HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, TRUE, FALSE, 0);
  mmuClearTLBbyMVA(gptPhysAddr);
  mmuDataBarrier();

  u32int guestVA  = gptPhysAddr;
  ptInfo* metadata = getPageTableInfo(shadow);
  if (metadata == 0)
  {
    DIE_NOW(0, "mapAPBitsPageTable: metadata not found.\n");
  } 
  u32int shadowVA = metadata->virtAddr;
#ifdef SHADOWING_DEBUG
  printf("mapAPBitsPageTable: guestVA %08x shadowVA %08x\n", guestVA, shadowVA);
#endif

  // loop through all second level entries
  u32int i = 0;
  for (i = 0; i < PT2_ENTRIES; i++)
  {
    simpleEntry* guestEntry  = (simpleEntry*)(guestVA + i*4);
    simpleEntry* shadowEntry = (simpleEntry*)(shadowVA + i*4);

    switch (guestEntry->type)
    {
      case FAULT:
      {
        break;
      }
      case LARGE_PAGE:
      {
        DIE_NOW(0, "mapAPBitsPageTable hit large page!\n");
        break;
      }
      case SMALL_PAGE:
      case SMALL_PAGE_3:
      {
        // get domain, call mapAPBitsSmallPage
        smallPageEntry* guestSmallPage  = (smallPageEntry*)guestEntry;
        smallPageEntry* shadowSmallPage = (smallPageEntry*)shadowEntry;
        // va now needs to be adjusted for each small page
        mapAPBitsSmallPage(guest->domain, guestSmallPage, shadowSmallPage);
      }
    } // switch ends
  } // for ends
#ifdef SHADOWING_DEBUG
  printf("mapAPBitsPageTable: restored entry %08x @ %p\n", backupEntry, first);
#endif
  *(u32int*)first = backupEntry;
  mmuClearTLBbyMVA(gptPhysAddr);
  mmuDataBarrier();
}


/**
 * Access control bit mapping from guest to shadow priv and shadow user
 * Checks if guest is not editing an entry that contains guest page tables
 **/
void mapAPBitsSmallPage(u32int dom, smallPageEntry* guest, smallPageEntry* shadow)
{
#ifdef SHADOWING_DEBUG
  printf("mapAPBitsSmallPage: dom %08x, guest %08x @ %08x, shadow %08x @ %08x\n",
         dom, *(u32int*)guest, (u32int)guest, *(u32int*)shadow, (u32int)shadow);
#endif

  GCONTXT* context = getGuestContext();

  // get new access permission bits, that take into account guest DACR
  u32int sysCtrlReg = getCregVal(1, 0, 0, 0, &context->coprocRegBank[0]);
  if (sysCtrlReg & SYS_CTRL_ACCESS_FLAG)
  {
    DIE_NOW(context, "mapAPBitsSmallPage: access flag enabled set, unimplemented.\n");
  }
  u32int guestAP = (guest->ap2 << 2) | guest->ap10;
  u32int shadowAP = mapAccessPermissionBits(guestAP, dom);
#ifdef SHADOWING_DEBUG
  printf("mapAPBitsSmallPage: guestAP %x, shadowAP %x\n", guestAP, shadowAP);
#endif
  shadow->ap2  = (shadowAP >> 2) & 0x1;
  shadow->ap10 =  shadowAP & 0x3;

  u32int guestPhysical = guest->addr << 12;
  if ((guestPhysical >= (u32int)context->pageTables->guestPhysical) &&
      (guestPhysical <  (u32int)context->pageTables->guestPhysical + PT1_SIZE))
  {
    // 1st level page table lives in this small page!
    // guestWriteProtect(vAddr, vAddr + SMALL_PAGE_SIZE - 1);
    DIE_NOW(context, "mapAPBitsSmallPage: guest mapped a page to its 1st lvl PT!");
  }

  // maybe second level page tables live in this section?
  ptInfo* head = context->pageTables->gptInfo;
  if (head == 0)
  {
    DIE_NOW(0, "mapAPBitsSmallPage: no PT meta data at all\n");
  }
  do
  {
    if ((guestPhysical >= (u32int)head->physAddr) &&
        (guestPhysical <  (u32int)head->physAddr + PT2_SIZE))
    {
      // guestWriteProtect(vAddr, vAddr + SMALL_PAGE_SIZE - 1);
      DIE_NOW(context, "mapAPBitsSmallPage: guest mapped a small page to its 2nd lvl PT!");
      break;
    }
    head = head->nextEntry;
  }
  while (head != 0);
}


/**
 * checks guest domain access control, and correctly maps execute never bit
 **/
u32int mapExecuteNeverBit(u32int guestDomain, u32int xn)
{
  GCONTXT* context = getGuestContext();
  
  if (!xn)
  {
    // guest doesnt want XN!
    return xn;
  }
  else
  {
    u32int dacr = getCregVal(3, 0, 0, 0, &context->coprocRegBank[0]);
    u32int domBits = (dacr >> (guestDomain*2)) & 0x3;
    if (domBits == DACR_MANAGER)
    {
      // manager access overrides any XN setting
      return 0;
    }
    else
    {
      return xn;
    }
  }
}


/**
 * Needs putting somewhere more appropriate, perhaps its own file
 * once we implement guest domain mapping (held in CP15?/guestContext?)
 **/
u8int mapGuestDomain(u8int guestDomain)
{
  DIE_NOW(0, "mapGuestDomain unimplemented.\n");
  // may do a lot of work here... change AP bits based on DACR
  return 0;
}
