#include "common/assert.h"
#include "common/debug.h"
#include "common/linker.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "guestManager/guestContext.h"
#include "guestManager/translationStore.h"

#include "memoryManager/memoryConstants.h"
#include "memoryManager/memoryProtection.h"
#include "memoryManager/mmu.h"
#include "memoryManager/shadowMap.h"


/**
 * take a VA that caused a memory abort and try to add a shadow mapping
 * from the guest page table to both shadow page tables 
 **/
bool shadowMap(GCONTXT *context, u32int virtAddr)
{
  DEBUG(MM_SHADOWING, "shadowMap: virtual address %#.8x" EOL, virtAddr);

  bool success = FALSE;
  simpleEntry* spt = context->pageTables->shadowActive;
  simpleEntry* gpt = context->pageTables->guestPhysical;

  simpleEntry* ttbrBackup = mmuGetTTBR0();
  mmuSetTTBR0(context->pageTables->hypervisor, 0x1FF);

  simpleEntry* guestFirst = getEntryFirst(gpt, virtAddr);
  DEBUG(MM_SHADOWING, "shadowMap: VA %08x first entry %08x @ %p" EOL, virtAddr, *(u32int*)guestFirst, guestFirst);

  switch(guestFirst->type)
  {
    case SECTION:
    {
      DEBUG(MM_SHADOWING, "shadowMap: guest first level entry section" EOL);
      u32int virtual = ((u32int)guestFirst - (u32int)gpt) << 18;
      DEBUG(MM_SHADOWING, "shadowMap: virtual address mapped %#.8x" EOL, virtual);
      sectionEntry* guestSection = (sectionEntry*)guestFirst;
      sectionEntry* shadowSection  = (sectionEntry*)getEntryFirst(spt, virtAddr);
      shadowMapSection(context, guestSection, shadowSection, virtual);
      mmuPageTableEdit((u32int)shadowSection, (virtAddr & SECTION_MASK));
      success = TRUE;
      break;
    }
    case PAGE_TABLE:
    {
      DEBUG(MM_SHADOWING, "shadowMap: guest first level entry page table" EOL);
      // must check if PT2 entry was shadow mapped already. if not, shadow map PT2 
      simpleEntry* shadowFirst = getEntryFirst(spt, virtAddr);
      switch(shadowFirst->type)
      {
        case FAULT:
        {
          DEBUG(MM_SHADOWING, "shadowMap: shadow 1st lvl entry fault. need to shadowmap PT2" EOL);
          pageTableEntry* shadowPageTable  = (pageTableEntry*)shadowFirst;
          pageTableEntry* guestPageTable = (pageTableEntry*)guestFirst;
          shadowMapPageTable(context, guestPageTable, shadowPageTable);
          //FIXME: Henri: This TLB flush should not be necessary
          mmuPageTableEdit((u32int)shadowPageTable, (virtAddr & SECTION_MASK));
          break;
        }
        case PAGE_TABLE:
        {
          DEBUG(MM_SHADOWING, "shadowMap: shadow 1st lvl entry PT2. OK." EOL);
          // already mapped as PT2 in shadow page tables. however, the guest PT2 metadata might not
          // be saved, in case the hypervisor used that mapping for its own purposes.
          // make sure gPT2 metadata is really entered!
          ptInfo* metadata = getPageTableInfo(context, (pageTableEntry*)guestFirst);
          if (metadata == 0)
          {
            u32int mappedAddr = ((u32int)guestFirst - (u32int)gpt) << 18;
            addPageTableInfo(context, (pageTableEntry*)guestFirst, 0, ((pageTableEntry*)guestFirst)->addr << 10, mappedAddr, FALSE);
          }
          break;
        }
        case SECTION:
        {
          // if shadow entry is section, something's wrong.
          DIE_NOW(context, "shadowMap: guest PT2 but shadow SECTION");
          break;
        }
        case RESERVED:
        {
          // if shadow entry is section, something's wrong.
          DIE_NOW(context, "shadowMap: guest PT2 but shadow RESERVED");
          break;
        }
      }

      // now we are sure the PT2 is shadow mapped
      // we must try to shadow map the small page
      u32int gptPhysAddr = (((pageTableEntry*)guestFirst)->addr) << 10;
      u32int index = (virtAddr & 0x000FF000) >> 10;
      u32int guestSecondPtr = gptPhysAddr | index;
      simpleEntry* guestSecondEntry = (simpleEntry*)guestSecondPtr;
      DEBUG(MM_SHADOWING, "shadowMap: guest 2nd lvl entry %#.8x @ %p" EOL,
            *(u32int*)guestSecondEntry, guestSecondEntry);
      switch (guestSecondEntry->type)
      {
        case SMALL_PAGE:
        case SMALL_PAGE_3:
        {
          smallPageEntry* guestSmallPage = (smallPageEntry*)guestSecondEntry;
          smallPageEntry* shadowSmallPage = (smallPageEntry*)getEntrySecond(context, (pageTableEntry*)shadowFirst, virtAddr);
          shadowMapSmallPage(context, guestSmallPage, shadowSmallPage, ((pageTableEntry*)guestFirst)->domain);
          mmuPageTableEdit((u32int)shadowSmallPage, (virtAddr & SMALL_PAGE_MASK));
          success = TRUE;
          break;
        }
        case LARGE_PAGE:
        {
          DIE_NOW(context, "found large page entry. investigate");
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
      DIE_NOW(context, "shadowMap: invalid 1st lvl page table entry");
    }
  }

  mmuSetTTBR0(ttbrBackup, 0x100 | context->pageTables->contextID);
  return success;
}


/**
 * takes pointers to guest and shadow page table section entries
 * maps/copies data from guest to shadow entry
 **/
void shadowMapSection(GCONTXT *context, sectionEntry* guest, sectionEntry* shadow, u32int virtual)
{
  DEBUG(MM_SHADOWING, "shadowMapSection: guest entry %#.8x @ %p" EOL, *(u32int *)guest, guest);
  DEBUG(MM_SHADOWING, "shadowMapSection: shadow entry %#.8x @ %p" EOL, *(u32int *)shadow, shadow);
  DEBUG(MM_SHADOWING, "shadowMapSection: virtual %#.8x" EOL, virtual);

  bool peripheral = FALSE;

  if (guest->superSection)
  {
    // copy supersection unimplemented
    DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
  }

  u32int guestPhysAddr = guest->addr << 20;
  if ((guestPhysAddr < MEMORY_START_ADDR) || (guestPhysAddr >= MEMORY_END_ADDR))
  {
    // mapping a peripheral?
    peripheral = TRUE;
    if (shadow->type != FAULT)
    {
      DIE_NOW(context, "shadowMapSection: peripheral mapping already exists!");
    }
  }

  u32int startAddr = HYPERVISOR_BEGIN_ADDRESS;
  u32int endAddr = MEMORY_END_ADDR;
  // if the hypervisor memory starts/ends in this section, panic
  if ( ((guestPhysAddr >= startAddr) && (guestPhysAddr <= endAddr)) ||
      (((guestPhysAddr+SECTION_SIZE-1) >= startAddr) && ((guestPhysAddr+SECTION_SIZE-1) <= endAddr)) )
  {
    DIE_NOW(context, "shadowMapSection: guest mapping physical address the hypervisor lives in\n");
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
                    GUEST_ACCESS_DOMAIN, PRIV_RW_USR_NO, 0, 0, 0b000, FALSE);
    }
    else
    {
      addSectionEntry((sectionEntry*)host, sectionAddr,
                    GUEST_ACCESS_DOMAIN, GUEST_ACCESS_BITS, 1, 0, 0b000, FALSE);
    }
    host = (sectionEntry*)getEntryFirst(context->pageTables->hypervisor, guestPhysAddr);
  }
  DEBUG(MM_SHADOWING, "shadowMapSection: guest address mapped to host %#.8x" EOL, host->addr << 20);

  shadow->type = SECTION;
  shadow->addr = host->addr;
  shadow->c = peripheral ? 0 : guest->c;
  shadow->b = 0;
  shadow->s = 0;
  shadow->tex = 0b100;
  shadow->nG = guest->nG;
  shadow->ns = 0;
  shadow->domain = GUEST_ACCESS_DOMAIN;
  DEBUG(MM_SHADOWING, "shadowMapSection: Shadow entry now @ %p = %#.8x" EOL, shadow,
        *(u32int *)shadow);

  if (guest->xn)
  {
    // execute never bit depends on guest entry XN and DACR
    shadow->xn = mapExecuteNeverBit(context, guest->domain, guest->xn);
    DEBUG(MM_SHADOWING, "shadowMapSection: shadow xn bit %x" EOL, shadow->xn);
  }

  if (guest->imp)
  {
    DEBUG(MM_SHADOWING, "shadowMapSection: guest OS PT using the imp bit, investigate" EOL);
  }

  // maps AP bits to shadow entry, write-protects if necessary
  if (peripheral)
  {
    shadow->ap10 = PRIV_RW_USR_NO & 0x3;
    shadow->ap2  = PRIV_RW_USR_NO >> 2;
  }
  else
  {
    mapAPBitsSection(context, guest, (simpleEntry*)shadow, virtual);
  }
  DEBUG(MM_SHADOWING, "shadowMapSection: Shadow entry after AP map @ %p = %#.x" EOL, shadow,
        *(u32int *)shadow);
}


/**
 * takes pointers to guest and shadow page table section entries
 * removes the shadow mapping for the corresponding guest entry
 **/
void shadowUnmapSection(GCONTXT *context, simpleEntry* shadow, sectionEntry* guest, u32int virtual)
{
  DEBUG(MM_SHADOWING, "shadowUnmapSection: guest entry %#.8x @ %p" EOL, *(u32int *)guest, guest);
  DEBUG(MM_SHADOWING, "shadowUnmapSection: shadow entry %#.8x @ %p" EOL,*(u32int *)shadow, shadow);
  DEBUG(MM_SHADOWING, "shadowUnmapSection: virtual address %#.8x" EOL, virtual);

  if (shadow->type == FAULT)
  {
    // quick exit if entry hasn't been shadow mapped yet.
    return;
  }

  // is the guest trying to unmap a non-RAM address? must check not to screw up
  u32int physAddr = guest->addr << 20;
  if ((physAddr < MEMORY_START_ADDR) || (physAddr >= MEMORY_END_ADDR))
  {
    printf("shadowUnmapSection: physAddr %08x" EOL, physAddr);
    DIE_NOW(context, "shadowUnmapSection: not a RAM address, doublecheck");
  }

  // would gladly remove this entry, but must check if the guest didnt decide
  // to remove pte that maps the for the hypervisor
  u32int startAddr = HYPERVISOR_BEGIN_ADDRESS;
  u32int endAddr = MEMORY_END_ADDR;
  // if the hypervisor memory starts/ends in this section, panic
  if ( ((physAddr >= startAddr) && (physAddr <= endAddr)) ||
      (((physAddr+SECTION_SIZE-1) >= startAddr) && ((physAddr+SECTION_SIZE-1) <= endAddr)) )
  {
    DIE_NOW(context, "shadowUnmapSection: Guest trying to unmap an address the hypervisor lives in");
  }

  // Flush block cache at the address range that the guest mapped originally.
  clearTranslationsByAddressRange(context->translationStore, virtual, (virtual+SECTION_SIZE-1));
  
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
    deleteLevelTwoPageTable(context, (pageTableEntry*)shadow);
  }

  // and finally, remove the shadow entry!
  *(u32int *)shadow = 0;
  mmuInvalidateUTLBbyMVA(virtual);
  mmuDataMemoryBarrier();
}



/**
 * takes pointers to guest and shadow page table PAGE_TABLE type entries
 * creates a new shadow 2nd level page table and copies the guest mapping
 **/
void shadowMapPageTable(GCONTXT *context, pageTableEntry* guest, pageTableEntry* shadow)
{
  DEBUG(MM_SHADOWING, "shadowMapPageTable guest %#.8x @ %p, shadow %#.8x @ %p" EOL,
        *(u32int *)guest, guest, *(u32int *)shadow, shadow);

  u32int sptVirtAddr = 0;
  u32int sptPhysAddr = 0;

  switch(shadow->type)
  {
    case FAULT:
    {
      // need to allocate a new shadow page table!
      sptVirtAddr = (u32int)memalign(1 << PT2_ALIGN_BITS, PT2_SIZE);
      if (sptVirtAddr == 0)
      {
        DIE_NOW(context, "failed to allocate 2lvl shadow page table");
      }
      memset((void *)sptVirtAddr, 0, PT2_SIZE);
      DEBUG(MM_SHADOWING, "shadowMapPageTable: newPT2 @ %#.8x" EOL, sptVirtAddr);

      sptPhysAddr = getPhysicalAddress(context, context->pageTables->shadowActive, sptVirtAddr);
    
      // fill in the given 1st level entry with correct data
      shadow->addr = sptPhysAddr >> 10;
      //This is just a copy of the high level descriptor
      shadow->type = PAGE_TABLE;
      shadow->domain = GUEST_ACCESS_DOMAIN;
      shadow->ns = guest->ns;

      u32int mapped = ((u32int)shadow - (u32int)context->pageTables->shadowActive) << 18;
      addPageTableInfo(context, shadow, sptVirtAddr, sptPhysAddr, mapped, TRUE);
      break;
    }
    case PAGE_TABLE:
    {
      ptInfo* metadata = getPageTableInfo(context, shadow);
      if (metadata == 0)
      {
        DIE_NOW(context, "shadowMapPageTable: sPT2 metadata not found");
      }
      sptVirtAddr = metadata->virtAddr;
      break;
    }
    case SECTION:
    {
      DIE_NOW(context, "shadowMaPageTable: shadow entry already written, section");
    }
    case RESERVED:
    {
      DIE_NOW(context, "shadowMaPageTable: shadow entry already written, reserved");
    }
  } // shadow switch

  u32int mapped = (getPageTableInfo(context, shadow))->mappedMegabyte;
  addPageTableInfo(context, (pageTableEntry*)guest, 0, guest->addr << 10, mapped, FALSE);

  // ok. we must scan the shadow PT looking for previously shadow mapped entries
  // that point to this new guest 2nd lvl page table. if found, write-protect
  u32int gptPhysical = guest->addr << 10;
  simpleEntry* shadowActiveBackup = context->pageTables->shadowActive;
  simpleEntry* shadowUser = context->pageTables->shadowUser;
  simpleEntry* shadowPriv = context->pageTables->shadowPriv;

  u32int i = 0;
  for (i=0; i < PT1_ENTRIES; i++)
  {
    context->pageTables->shadowActive = shadowUser;
    if (shadowUser[i].type == SECTION && shadowUser[i].domain != HYPERVISOR_ACCESS_DOMAIN)
    {
      sectionEntry* sectionPtr = (sectionEntry*)&shadowUser[i];
      u32int section = sectionPtr->addr << 20;
      if ((section <= gptPhysical) && ((section + SECTION_SIZE - 1) >= gptPhysical))
      {
        DEBUG(MM_SHADOWING, "shadowMapPageTable: section %#.8x maps guest PT2 %#.8x" EOL, section,
              gptPhysical);
        // guest protect PT2
        u32int virtualAddress = i << 20;
        virtualAddress |= (gptPhysical & ~SECTION_MASK);
        DEBUG(MM_SHADOWING, "shadowMapPageTable: virtualAddress of gPT2 %#.8x" EOL, virtualAddress);
        guestWriteProtect(context, virtualAddress, virtualAddress + PT2_SIZE - 1);
      }
    }
    else if (shadowUser[i].type == PAGE_TABLE && shadowUser[i].domain != HYPERVISOR_ACCESS_DOMAIN)
    {
      pageTableEntry* pageTablePtr = (pageTableEntry*)&shadowUser[i];
      ptInfo* metadata = getPageTableInfo(context, pageTablePtr);
      if (metadata == 0)
      {
        DIE_NOW(context, "sPT2 metadata not found while checking AP");
      }
      simpleEntry* tempPageTable = (simpleEntry*)(metadata->virtAddr);
      u32int y = 0;
      for (y = 0; y < PT2_ENTRIES; y++)
      {
        if (tempPageTable[y].type != FAULT)
        {
          if (tempPageTable[y].type == LARGE_PAGE)
          {
            largePageEntry* largePage = (largePageEntry*)&tempPageTable[y]; 
            u32int phys = largePage->addr << 16;
            u32int physPageTableMasked = gptPhysical & LARGE_PAGE_MASK;
            if (phys == physPageTableMasked)
            {
              u32int virtualAddress = i << 20;
              virtualAddress |= (y << 12);
              guestWriteProtect(context, virtualAddress, virtualAddress + PT2_SIZE - 1);
            }
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
              guestWriteProtect(context, virtualAddress, virtualAddress + PT2_SIZE - 1);
            }
          }
        } // !fault
      } // for loop
    } // host PAGE_TABLE

    context->pageTables->shadowActive = shadowPriv;
    if ((shadowPriv[i].type == SECTION) && (shadowUser[i].domain != HYPERVISOR_ACCESS_DOMAIN))
    {
      sectionEntry *sectionPtr = (sectionEntry *)&shadowPriv[i];
      u32int section = sectionPtr->addr << 20;
      if ((section <= gptPhysical) && ((section + SECTION_SIZE - 1) >= gptPhysical))
      {
        DEBUG(MM_SHADOWING, "shadowMapPageTable: section %#.8x maps guest PT2 %#.8x" EOL, section,
              gptPhysical);
        // guest protect PT2
        u32int virtualAddress = i << 20;
        virtualAddress |= (gptPhysical & ~SECTION_MASK);
        DEBUG(MM_SHADOWING, "shadowMapPageTable: virtualAddress of gPT2 %#.8x" EOL, virtualAddress);
        guestWriteProtect(context, virtualAddress, virtualAddress + PT2_SIZE - 1);
      }
    }
    else if ((shadowPriv[i].type == PAGE_TABLE) && (shadowUser[i].domain != HYPERVISOR_ACCESS_DOMAIN))
    {
      pageTableEntry* pageTablePtr = (pageTableEntry*)&shadowPriv[i];
      ptInfo* metadata = getPageTableInfo(context, pageTablePtr);
      if (metadata == 0)
      {
        DIE_NOW(context, "shadowMapPageTable: sPT2 metadata not found while checking AP");
      }
      simpleEntry* tempPageTable = (simpleEntry*)(metadata->virtAddr);
      u32int y = 0;

      for (y = 0; y < PT2_ENTRIES; y++)
      {
        if (tempPageTable[y].type != FAULT)
        {
          if(tempPageTable[y].type == LARGE_PAGE)
          {
            largePageEntry* largePage = (largePageEntry*)&tempPageTable[y]; 
            u32int phys = largePage->addr << 16;
            u32int physPageTableMasked = gptPhysical & LARGE_PAGE_MASK;
            if (phys == physPageTableMasked)
            {
              u32int virtualAddress = i << 20;
              virtualAddress |= (y << 12);
              guestWriteProtect(context, virtualAddress, virtualAddress + PT2_SIZE - 1);
            }
          }
          else
          {
            // must calculate what VA guest is trying to map with this small page
            smallPageEntry *smallPage = (smallPageEntry *)&tempPageTable[y];
            u32int phys = smallPage->addr << 12;
            u32int physPageTableMasked = gptPhysical & SMALL_PAGE_MASK;
            if (phys == physPageTableMasked)
            {
              u32int virtualAddress = i << 20;
              virtualAddress |= (y << 12);
              guestWriteProtect(context, virtualAddress, virtualAddress + PT2_SIZE - 1);
            }
          }
        } // !fault
      } // for loop
    } // host PAGE_TABLE
  } // for loop
  context->pageTables->shadowActive = shadowActiveBackup;
}


/**
 * this function removes a shadow 2nd lvl page table
 **/
void shadowUnmapPageTable(GCONTXT *context, pageTableEntry *shadow, pageTableEntry *guest, u32int virtual)
{
  DEBUG(MM_SHADOWING, "shadowUnmapPageTable: shadow %#.8x @ %p, guest %#.8x @ %p, VA %#.8x" EOL,
        *(u32int *)shadow, shadow, *(u32int *)guest, guest, virtual);

  if (shadow->type == FAULT)
  {
    // entry has not been shadow mapped. quick return.
    return;
  }

  // would gladly remove this entry, but must check if the guest didnt decide
  // to remove pte that maps the for the hypervisor
  u32int startAddr = HYPERVISOR_BEGIN_ADDRESS;
  u32int endAddr = MEMORY_END_ADDR;

  ptInfo* metadata = getPageTableInfo(context, shadow);
  if (metadata == 0)
  {
    DIE_NOW(0, "shadowUnMapPageTable: sPT2 metadata not found while checking AP\n");
  }
  simpleEntry* tempPageTable = (simpleEntry*)(metadata->virtAddr);
  u32int y = 0;
  for (y = 0; y < PT2_ENTRIES; y++)
  {
    if (tempPageTable[y].type != FAULT)
    {
      if(tempPageTable[y].type == LARGE_PAGE)
      {
        DIE_NOW(0, "shadowUnMapPageTable: found guest LARGE_PAGE entry, investigate.\n");
      }
      else
      {
        // if the hypervisor memory starts/ends in this page, panic
        u32int guestPhysAddr = ((smallPageEntry*)tempPageTable)[y].addr << 12;
        if ( ((guestPhysAddr >= startAddr) && (guestPhysAddr <= endAddr)) ||
            (((guestPhysAddr+SMALL_PAGE_SIZE-1) >= startAddr) && ((guestPhysAddr+SMALL_PAGE_SIZE-1) <= endAddr)) )
        {
          DIE_NOW(context, "shadowUnmapSection: Guest trying to unmap an address the hypervisor lives in\n");
        }
      }
    } // !fault
  } // for loop

  // Flush block cache at the address range that the guest mapped originally.
  clearTranslationsByAddressRange(context->translationStore, virtual, (virtual+SECTION_SIZE-1));

  if (context->pageTables->guestVirtual != NULL)
  {
    // if the current gPT1 base VA is in this section, lets 'forget' it in GC
    if ((virtual <= (u32int)context->pageTables->guestVirtual) && 
       ((u32int)context->pageTables->guestVirtual < (virtual + SECTION_SIZE)))
    {
      DIE_NOW(context, "shadowUnmapPageTable: unmapping entry that is used for gPT VA.");
    }
  }

  deleteLevelTwoPageTable(context, (pageTableEntry *)shadow);

  // and finally, remove the shadow entry!
  *(u32int *)shadow = 0;
  mmuInvalidateUTLBbyMVA(virtual);
  mmuDataMemoryBarrier();
}


/**
 * takes pointers to guest and shadow small page entries
 * shadow maps the given guest small page entry.
 **/
void shadowMapSmallPage(GCONTXT *context, smallPageEntry* guest, smallPageEntry* shadow, u32int dom)
{
  DEBUG(MM_SHADOWING, "shadowMapSmallPage: guest %#.8x @ %p; shadow %#.8x @ %p" EOL,
        *(u32int *)guest, guest, *(u32int *)shadow, shadow);
  DEBUG(MM_SHADOWING, "shadowMapSmallPage: dom %x" EOL, dom);

  bool peripheral = FALSE;

  u32int guestPhysical = (guest->addr << 12) & SMALL_PAGE_MASK;
  if ((guestPhysical < MEMORY_START_ADDR) || (guestPhysical >= MEMORY_END_ADDR))
  {
    peripheral = TRUE;
    if (shadow->type != FAULT)
    {
      DIE_NOW(NULL, "shadowMapSmallPage: peripheral mapping already exists!");
    }
  }

  u32int startAddr = HYPERVISOR_BEGIN_ADDRESS;
  u32int endAddr = MEMORY_END_ADDR;
  // if the hypervisor memory starts/ends in this section, panic
  if ( ((guestPhysical >= startAddr) && (guestPhysical <= endAddr)) ||
      (((guestPhysical+SMALL_PAGE_SIZE-1) >= startAddr) && ((guestPhysical+SMALL_PAGE_SIZE-1) <= endAddr)) )
  {
    DIE_NOW(context, "shadowMapSmallPage: guest mapping physical address the hypervisor lives in\n");
  }

  simpleEntry* hostEntry = (simpleEntry*)getEntryFirst(context->pageTables->hypervisor, guestPhysical);
  u32int hostPhysical = 0;
  switch (hostEntry->type)
  {
    case SECTION:
    {
      sectionEntry *hostSection = (sectionEntry *)hostEntry;
      hostPhysical = ((hostSection->addr << 20) & SECTION_MASK) |
                      (guestPhysical & (~(SECTION_MASK)));
      break;
    }
    case PAGE_TABLE:
    {
      simpleEntry* hostPage = getEntrySecond(context, (pageTableEntry*)hostEntry, guestPhysical);
      switch (hostPage->type)
      {
        case FAULT:
        {
          addSmallPageEntry((smallPageEntry *)hostPage, guestPhysical,
              GUEST_ACCESS_BITS, FALSE, FALSE, 0, FALSE);
          // Then, fall through and get host physical address
        }
        case SMALL_PAGE:
        case SMALL_PAGE_3:
        {
          smallPageEntry *hostSmallPage = (smallPageEntry *)hostPage;
          hostPhysical = (hostSmallPage->addr << 12) & SMALL_PAGE_MASK;
          break;
        }
        case LARGE_PAGE:
        default:
          // host physical 2nd lvl unimplemented
          DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
      }
      break;
    }
    case FAULT:
    case RESERVED:
    default:
    {
      DIE_NOW(context, "shadowMapSmallPage: invalid entry found translating guestPA to hostPA.");
    }
  }
  DEBUG(MM_SHADOWING, "shadowMapSmallPage: guest physical %#.8x maps to host physical %#.8x" EOL,
        guestPhysical, hostPhysical);
  shadow->type = 1; // small page entry.
  shadow->addr = (hostPhysical >> 12);

  // maps AP bits to shadow entry, write-protects if necessary
  mapAPBitsSmallPage(context, dom, guest, shadow);
  DEBUG(MM_SHADOWING, "shadowMapSmallPage: Shadow entry after AP map @ %p = %#.8x\n", shadow,
        *(u32int *)shadow);

  if (guest->xn)
  {
    // execute never bit depends on guest entry XN and DACR
    shadow->xn = mapExecuteNeverBit(context, dom, guest->xn);
    DEBUG(MM_SHADOWING, "shadowMapSmallPage: shadow xn bit %x" EOL, shadow->xn);
  }

  shadow->c = peripheral ? 0 : guest->c;
  shadow->b = 0;
  shadow->tex = 0b100;
  shadow->s = 0;
  shadow->nG = guest->nG;

  DEBUG(MM_SHADOWING, "shadowMapSmallPage: Shadow at the end @ %p = %#.8x" EOL, shadow,
        *(u32int *)shadow);
}


/**
 * removes a shadow-mapped small page entry. does all necessary checks
 **/
void shadowUnmapSmallPage(GCONTXT *context, smallPageEntry* shadow, smallPageEntry* guest, u32int virtual)
{
  DEBUG(MM_SHADOWING, "shadowUnmapSmallPage: guest entry %#.8x @ %p" EOL, *(u32int *)guest, guest);
  DEBUG(MM_SHADOWING, "shadowUnmapSmallPage: shadow entry %#.8x @ %p" EOL, *(u32int *)shadow, shadow);
  DEBUG(MM_SHADOWING, "shadowUnmapSmallPage: virtual address %#.8x" EOL, virtual);

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
    DIE_NOW(NULL, "shadowUnmapSmallPage: not a RAM address, doublecheck");
  }
  DEBUG(MM_SHADOWING, "shadowUnmapSmallPage: VA %#.8x PA %#.8x" EOL, virtual, physAddr);


  // if the hypervisor memory starts/ends in this page, panic
  u32int startAddr = HYPERVISOR_BEGIN_ADDRESS;
  u32int endAddr = MEMORY_END_ADDR;
  if ( ((physAddr >= startAddr) && (physAddr <= endAddr)) ||
      (((physAddr+SMALL_PAGE_SIZE-1) >= startAddr) && ((physAddr+SMALL_PAGE_SIZE-1) <= endAddr)) )
  {
    DIE_NOW(NULL, "guest trying to unmap an address the hypervisor lives in");
  }

  // Flush block cache at the address range that the guest mapped originally.
  clearTranslationsByAddressRange(context->translationStore, virtual, (virtual+SMALL_PAGE_SIZE-1));

  if (context->pageTables->guestVirtual != NULL)
  {
    // if the current gPT1 base VA is in this section, lets 'forget' it in GC
    if ((virtual & PT1_ALIGN_MASK) == (u32int)context->pageTables->guestVirtual)
    {
      DIE_NOW(context, "shadowUnmapSection: unmapping entry that is used for gPT VA.");
    }
  }
  
  // and finally, remove the shadow entry!
  *(u32int *)shadow = 0;
  mmuInvalidateUTLBbyMVA(virtual);
  mmuDataMemoryBarrier();
}


/**
 * maps access permission bits from guest entry to current active shadow
 * takes into account the settings in guest domain access control register
 **/
u32int mapAccessPermissionBits(GCONTXT *context, u32int guestAP, u32int domain)
{
  u32int dacr = context->coprocRegBank[CP15_DACR].value;
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
void mapAPBitsSection(GCONTXT *context, sectionEntry* guest, simpleEntry* shadow, u32int virtual)
{
  DEBUG(MM_SHADOWING, "mapAPBitsSection: guest entry %#.8x @ %p" EOL, *(u32int *)guest, guest);
  DEBUG(MM_SHADOWING, "mapAPBitsSection: shadow entry %#.8x @ %p" EOL, *(u32int *)shadow, shadow);
  DEBUG(MM_SHADOWING, "mapAPBitsSection: virtual %#.8x" EOL, virtual);

  // get new access permission bits, that take into account guest DACR
  u32int sysCtrlReg = context->coprocRegBank[CP15_SCTRL].value;
  if ((sysCtrlReg & SYS_CTRL_ACCESS_FLAG))
  {
    // access flag enabled set, unimplemented
    DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
  }
  u32int guestAP = (guest->ap2 << 2) | guest->ap10;
  u32int shadowAP = mapAccessPermissionBits(context, guestAP, guest->domain);
  DEBUG(MM_SHADOWING, "mapAPBitsSection: guestAP %x, shadowAP %x" EOL, guestAP, shadowAP);

  switch (shadow->type)
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
      pageTableEntry *shadowPT = (pageTableEntry *)shadow;
      ptInfo *metadata = getPageTableInfo(context, shadowPT);
      if (metadata == NULL)
      {
        DIE_NOW(context, "mapAPBitsSection: could not find required metadata for PT2.\n");
      }

      u32int i;
      for (i = 0; i < PT2_ENTRIES; i++)
      {
        smallPageEntry *shadowSmallPage = (smallPageEntry *)(metadata->virtAddr + i * PT_ENTRY_SIZE);
        shadowSmallPage->ap2 = (shadowAP >> 2) & 0x1;
        shadowSmallPage->ap10 = shadowAP & 0x3;
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
    DEBUG(MM_SHADOWING, "mapAPBitsSection: vAddr maps to memory that contains a 1st lvl gPT" EOL);
    // 'virtual' is the virtual address that this section maps. add offset of PT1 in this section
    // get a correct full PT1 virtual address in this section and write-protect it.
    u32int virtualAddress =  virtual | (((u32int)context->pageTables->guestPhysical) & (~SECTION_MASK));
    DEBUG(MM_SHADOWING, "mapAPBitsSection: guest PT virtual address is %#.8x" EOL, virtualAddress);
    guestWriteProtect(context, virtualAddress, virtualAddress+PT1_SIZE-1);
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
      DEBUG(MM_SHADOWING, "mapAPBitsSection: vAddr maps to memory that contains a 2nd lvl gPT" EOL);
      u32int virtualAddress = virtual | (((u32int)head->physAddr) & (~SECTION_MASK));
      guestWriteProtect(context, virtualAddress, virtualAddress+PT2_SIZE-1);
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
void mapAPBitsPageTable(GCONTXT* context, pageTableEntry* guest, pageTableEntry* shadow)
{
  DEBUG(MM_SHADOWING, "mapAPBitsPageTable: guest %#.8x @ %p, shadow %#.8x @ %p" EOL,
        *(u32int *)guest, guest, *(u32int *)shadow, shadow);

  simpleEntry* ttbrBackup = mmuGetTTBR0();
  mmuSetTTBR0(context->pageTables->hypervisor, 0x1FF);

  u32int gptPhysAddr = getPhysicalAddress(context, context->pageTables->hypervisor, (guest->addr << 10));

  u32int guestVA  = gptPhysAddr;
  ptInfo* metadata = getPageTableInfo(context, shadow);
  if (metadata == 0)
  {
    DIE_NOW(context, "mapAPBitsPageTable: metadata not found");
  } 
  u32int shadowVA = metadata->virtAddr;
  DEBUG(MM_SHADOWING, "mapAPBitsPageTable: guestVA %#.8x shadowVA %#.8x" EOL, guestVA, shadowVA);

  // loop through all second level entries
  u32int i = 0;
  for (i = 0; i < PT2_ENTRIES; i++)
  {
    simpleEntry *guestEntry  = (simpleEntry *)(guestVA + i * 4);
    simpleEntry *shadowEntry = (simpleEntry *)(shadowVA + i * 4);

    // we only care about shadow-mapped second level entries 
    switch (shadowEntry->type)
    {
      case FAULT:
      {
        break;
      }
      case LARGE_PAGE:
      {
        DIE_NOW(context, "hit large page!");
        break;
      }
      case SMALL_PAGE:
      case SMALL_PAGE_3:
      {
        // get domain, call mapAPBitsSmallPage
        smallPageEntry *guestSmallPage  = (smallPageEntry *)guestEntry;
        smallPageEntry *shadowSmallPage = (smallPageEntry *)shadowEntry;
        // va now needs to be adjusted for each small page
        mapAPBitsSmallPage(context, guest->domain, guestSmallPage, shadowSmallPage);
        u32int pageAddress = metadata->mappedMegabyte + i * SMALL_PAGE_SIZE;
        mmuPageTableEdit((u32int)shadowSmallPage, pageAddress);
      }
    } // switch ends
  } // for ends

  mmuSetTTBR0(ttbrBackup, 0x100 | context->pageTables->contextID);
}


/**
 * Access control bit mapping from guest to shadow priv and shadow user
 * Checks if guest is not editing an entry that contains guest page tables
 **/
void mapAPBitsSmallPage(GCONTXT *context, u32int dom, smallPageEntry* guest, smallPageEntry* shadow)
{
  DEBUG(MM_SHADOWING, "mapAPBitsSmallPage: dom %#.8x, guest %#.8x @ %p, shadow %#.8x @ %p" EOL,
        dom, *(u32int *)guest, guest, *(u32int*)shadow, shadow);

  // get new access permission bits, that take into account guest DACR
  u32int sysCtrlReg = context->coprocRegBank[CP15_SCTRL].value;
  if (sysCtrlReg & SYS_CTRL_ACCESS_FLAG)
  {
    // access flag enabled set, unimplemented
    DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
  }
  u32int guestAP = (guest->ap2 << 2) | guest->ap10;
  u32int shadowAP = mapAccessPermissionBits(context, guestAP, dom);
  DEBUG(MM_SHADOWING, "mapAPBitsSmallPage: guestAP %x, shadowAP %x" EOL, guestAP, shadowAP);
  shadow->ap2  = (shadowAP >> 2) & 0x1;
  shadow->ap10 =  shadowAP & 0x3;

  u32int guestPhysical = guest->addr << 12;
  if ((guestPhysical >= (u32int)context->pageTables->guestPhysical) &&
      (guestPhysical <  (u32int)context->pageTables->guestPhysical + PT1_SIZE))
  {
    // 1st level page table lives in this small page!
    DIE_NOW(context, "mapAPBitsSmallPage: guest mapped a page to its 1st lvl PT!");
  }

  // maybe second level page tables live in this section?
  ptInfo *head = context->pageTables->gptInfo;
  if (head == NULL)
  {
    DIE_NOW(context, "mapAPBitsSmallPage: no PT meta data at all\n");
  }
  do
  {
    if ((guestPhysical >= (u32int)head->physAddr) &&
        (guestPhysical <  (u32int)head->physAddr + PT2_SIZE))
    {
      DIE_NOW(context, "mapAPBitsSmallPage: guest mapped a small page to its 2nd lvl PT!");
      break;
    }
    head = head->nextEntry;
  }
  while (head != NULL);
}


/**
 * checks guest domain access control, and correctly maps execute never bit
 **/
u32int mapExecuteNeverBit(GCONTXT *context, u32int guestDomain, u32int xn)
{
  if (!xn)
  {
    // guest doesnt want XN!
    return xn;
  }
  else
  {
    u32int dacr = context->coprocRegBank[CP15_DACR].value;
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
 * should map guest domain to host domain. currently all guest domains get squeezed
 * into one host GUEST_ACCESS_DOMAIN, so this function remains unused.
 **/
u8int mapGuestDomain(GCONTXT *context, u8int guestDomain)
{
  DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
  // may do a lot of work here... change AP bits based on DACR
}
