#include "common/assert.h"
#include "common/debug.h"
#include "common/memFunctions.h"

#include "guestManager/guestContext.h"
#include "guestManager/guestExceptions.h"

#include "memoryManager/memoryConstants.h"
#include "memoryManager/memoryProtection.h"
#include "memoryManager/mmu.h"


/**
 * this function called when we want to make sure an address range is not writable by the guest
 **/
void guestWriteProtect(u32int startAddress, u32int endAddress)
{
#ifdef MEM_PROT_DBG
  printf("guestWriteProtect: startAddress %x; endAddress %x\n", startAddress, endAddress);
#endif
  // 1. get page table entry for this address
  GCONTXT* gc = getGuestContext();

  if(startAddress > endAddress)
  {
    DIE_NOW(gc, "guestWriteProtect: startAddress is great than the endAddress.");
  }

  if (gc->virtAddrEnabled)
  {
    writeProtectRange(gc->pageTables->shadowUser, startAddress, endAddress);
    writeProtectRange(gc->pageTables->shadowPriv, startAddress, endAddress);
  }
  else
  {
    writeProtectRange(gc->pageTables->hypervisor, startAddress, endAddress);
  }
}


void writeProtectRange(simpleEntry* pageTable, u32int start, u32int end)
{
#ifdef MEM_PROT_DBG
  printf("writeProtectRange: PT %x, start %x; end %x\n", pageTable, start, end);
#endif
  u32int pageStartAddress = start;
  u32int pageEndAddress;

  do
  {
#ifdef MEM_PROT_DBG
    printf("writeProtectRange: current pageStartAddress: %x\n", pageStartAddress);
#endif
    // reset the page end address, will be recalculated
    pageEndAddress = 0;
    // if we find a section, make sure we split it up to smaller pages!
    simpleEntry* firstEntry = getEntryFirst(pageTable, start);
    switch(firstEntry->type)
    {
      case FAULT:
      {
        // virtual address probably not shadow mapped yet.
        return;
      }
      case SECTION:
      {
        // split section up to small pages, so we protect the least amount of space 
        splitSectionToSmallPages(pageTable, start);
        // now its PAGE_TABLE type, fall through...
      }
      case PAGE_TABLE:
      {
        // we are OK, its a second level page table
        break;
      }
      case RESERVED:
      {
        printf("writeProtectRange: startAddress %x; endAddress %x\n", start, end);
        DIE_NOW(0, "writeProtectRange: reserved 1st level page table entry!");
      }
      default:
        DIE_NOW(0, "writeProtectRange: Unrecognized first level entry. Error.");
    }

    simpleEntry* secondEntry = getEntrySecond((pageTableEntry*)firstEntry, pageStartAddress);
    switch(secondEntry->type)
    {
      case FAULT:
      {
        // virtual address must not be shadow mapped yet, but next page may be?
        pageEndAddress = (pageStartAddress & 0xFFFFF000) + (SMALL_PAGE_SIZE - 1);
        break;
      }
      case SMALL_PAGE:
      case SMALL_PAGE_3:
      {
        smallPageEntry* entry = (smallPageEntry*)secondEntry;
        // the only mode that allows usr mode read-write is 011
        if ( (entry->ap10 | (entry->ap2 << 2)) == PRIV_RW_USR_RW)
        {
          entry->ap10 = PRIV_RW_USR_RO & 0x3;
          entry->ap2 = PRIV_RW_USR_RO >> 2; 
          mmuInvalidateUTLBbyMVA(pageStartAddress);
        }
        pageEndAddress = (pageStartAddress & 0xFFFFF000) + (SMALL_PAGE_SIZE - 1);
        break;
      }
      case LARGE_PAGE:
      {
        DIE_NOW(0, "writeProtectRange: found large page, unimplemented.");
      }
      default:
      {
        DIE_NOW(0, "writeProtectRange: Unrecognized second level entry");
      }
    }

    if(pageEndAddress == 0)
    {
      DIE_NOW(0, "writeProtectRange: invalid page end address value.");
    }
#ifdef MEM_PROT_DBG
    printf("writeProtectRange: current pageEndAddress: %08x\n", pageEndAddress);
#endif
    pageStartAddress = pageEndAddress+1;
  }
  while (end > pageEndAddress);
}


/**
 * performs a full guest memory access fault checking sequence
 * if guest should abort, sets up the abort for the guest, returns TRUE
 **/
bool shouldDataAbort(bool privAccess, bool isWrite, u32int address)
{
#ifdef MEM_PROT_DBG
  printf("shouldDataAbort: priv %x, write %x, addr %x\n", privAccess, isWrite, address);
#endif

  GCONTXT* context = getGuestContext();

  simpleEntry* guestFirst = 0; 
  simpleEntry* guestSecond = 0;
  simpleEntry* shadowFirst = 0;
  simpleEntry* shadowSecond = 0;
  u32int backupFirst = 0;
  u32int backupSecond = 0;
  simpleEntry* gpt = 0;
  u32int gpt2 = 0; // this is just a the physical address as unsigned int
  simpleEntry* spt = context->pageTables->shadowActive;
  u32int sysCtrlReg = getCregVal(1, 0, 0, 0, &context->coprocRegBank[0]);

  if (sysCtrlReg & 0x00000002)
  {
    // guest enabled alignment checking (SCTRL.A) enabled
    // check alignment: if misaligned abort (alignment fault)
//    DIE_NOW(context, "shouldDataAbort: alignment checking enabled! unimplemented.\n");
  }


  if (context->pageTables->guestVirtual != 0)
  {
    // cool. we have the VA already! use it. 
#ifdef MEM_PROT_DBG
    printf("shouldDataAbort: guestVirtual PT set, %p\n", context->pageTables->guestVirtual);
#endif
    gpt = context->pageTables->guestVirtual;
  }
  else
  {
#ifdef MEM_PROT_DBG
    printf("shouldDataAbort: guestVirtual PT not set. hack a 1-2-1 of %p\n",
                                  context->pageTables->guestPhysical);
#endif
    // crap, we haven't shadow mapped the gPT VA->PA mapping yet.
    // hack a 1-2-1 mapping for now.
    shadowFirst = getEntryFirst(spt, (u32int)context->pageTables->guestPhysical);
    backupFirst = *(u32int*)shadowFirst;
#ifdef MEM_PROT_DBG
    printf("shouldDataAbort: backed up entry %08x @ %p\n", backupFirst, shadowFirst);
#endif
    mapSection(context->pageTables->shadowActive, (u32int)context->pageTables->guestPhysical, 
              (u32int)context->pageTables->guestPhysical, HYPERVISOR_ACCESS_DOMAIN,
              HYPERVISOR_ACCESS_BITS, TRUE, FALSE, 0);
    mmuInvalidateUTLBbyMVA((u32int)context->pageTables->guestPhysical);
    gpt = context->pageTables->guestPhysical;
#ifdef MEM_PROT_DBG
    printf("shouldDataAbort: gpt now set to %p\n", gpt);
#endif
  }

  bool returnValue = FALSE;

  /***************************** step 1 **************************/
  /* get PT entry (first, and second if needed), check for FAULT */ 
  /***************************************************************/
  guestFirst = getEntryFirst(gpt, address);
  switch(guestFirst->type)
  {
    case FAULT:
    {
#ifdef MEM_PROT_DBG
      printf("shouldDataAbort(): gpt entry lvl1 type FAULT!\n");
#endif
      throwDataAbort(context, address, dfsTranslationSection, isWrite, ((sectionEntry*)guestFirst)->domain);
      returnValue = TRUE;
      break;
    }
    case RESERVED:
    {
      DIE_NOW(context, "shouldDataAbort: first lvl entry RESERVED.");
    }
    case SECTION:
    {
      // first level entry section, valid. move to next check
      // must have prefetch aborted for another reason
      returnValue = FALSE;
      break;
    }
    case PAGE_TABLE:
    {
      // must check 2nd level PT entry
      // have to read guest 2nd lvl page table: hack a 1-2-1 memory mapping
      // in the shadow page table.
      gpt2 = (((pageTableEntry*)guestFirst)->addr) << 10;
      shadowSecond = getEntryFirst(context->pageTables->shadowActive, gpt2);
      backupSecond = *(u32int*)shadowSecond;
#ifdef MEM_PROT_DBG
      printf("shouldDataAbort: backed up PT2 entry %08x @ %p\n", backupSecond, shadowSecond);
#endif
      mapSection(context->pageTables->shadowActive, gpt2, gpt2,
           HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, TRUE, FALSE, 0);
      mmuInvalidateUTLBbyMVA(gpt2);

      // now PT2 is shadow mapped 1-2-1 VA/PA. can extract second level entry.
      u32int index = (address & 0x000FF000) >> 10;
      u32int entryAddress = gpt2 | index;
      guestSecond = (simpleEntry*)entryAddress;
      if (guestSecond->type == FAULT)
      {
        // if second level entry fault, page prefetch translation fault
#ifdef MEM_PROT_DBG
        printf("shouldDataAbort: abort - translation fault page!\n");
#endif
        returnValue = TRUE;
        throwDataAbort(context, address, dfsTranslationPage, isWrite, ((pageTableEntry*)guestFirst)->domain);
        break;
      }
    }
  } // switch ends

  if (returnValue)
  {
    // we already know we have a guest translation fault! restore backed up entry, return
    if (shadowFirst != 0)
    {
      *(u32int*)shadowFirst = backupFirst;
#ifdef MEM_PROT_DBG
      printf("shouldDataAbort: restored entry %08x @ %p\n", backupFirst, shadowFirst);
#endif
      mmuInvalidateUTLBbyMVA((u32int)context->pageTables->guestPhysical);
      mmuDataMemoryBarrier();
    }
    if (shadowSecond != 0)
    {
      // we had to backup an entry for a gPT2 as well. restore now
      // restore hacked entry to shadow page table.
      *(u32int*)shadowSecond = backupSecond;
#ifdef MEM_PROT_DBG
      printf("shouldDataAbort: restored PT2 entry %08x @ %p\n", backupSecond, shadowSecond);
#endif
      mmuInvalidateUTLBbyMVA(gpt2);
      mmuDataMemoryBarrier();
    }
    // and return
    return returnValue;
  }

  // check if access flag is enabled (SCTRL.AFE)
  if (sysCtrlReg & 0x20000000)
  {
    // check access flag: if AF=0 abort (access flag fault)
    DIE_NOW(context, "shouldDataAbort: access flag enabled set, simplified AP model unimplemented.\n");
  }

  // check domain
  u32int dom  = ((sectionEntry*)guestFirst)->domain;
  u32int dacr = getCregVal(3, 0, 0, 0, &context->coprocRegBank[0]);
  u32int domBits = (dacr >> (dom*2)) & 0x3;
  
  switch (domBits)
  {
    case DACR_NO_ACCESS:
    {
      // no-access for domain: abort (domain fault)
      DIE_NOW(context, "shouldDataAbort(): DACR 0: no access! unimplemented");
      break;
    }
    case DACR_RESERVED:
    {
      DIE_NOW(context, "shouldDataAbort(): domain access control 2: reserved!");
      break;
    }
    case DACR_MANAGER:
    {
      // manager access; do not check page table entry bits, allow access
      returnValue = FALSE;
      break;
    }
    case DACR_CLIENT:
    {
      // client access: need to check page table entry access control bits
      bool section = FALSE;
      u8int permissionBits = 0;
      if (guestFirst->type == SECTION)
      {
        sectionEntry* guestSection = (sectionEntry*)guestFirst;
        permissionBits = guestSection->ap10 | (guestSection->ap2 << 2);
        section = TRUE;
      }
      else // this is going to be a guest 2nd lvl PAGE_TABLE
      {
        // doesnt matter if small/large page, AP bits in same place
        // guestSecond pointer was calculated and set when checking for translation faults
        // can still rely on the value being correct 
        smallPageEntry* guestSmallPage = (smallPageEntry*)guestSecond;
        permissionBits = guestSmallPage->ap10 | (guestSmallPage->ap2 << 2);
        section = FALSE;
      }

      switch (permissionBits)
      {
        case PRIV_NO_USR_NO:      // priv no access, usr no access
        {
          throwDataAbort(context, address, section ? dfsPermissionSection : dfsPermissionPage, isWrite, dom);
          returnValue = TRUE;
          break;
        }
        case PRIV_RW_USR_NO:      //priv read/write, usr no access
        {
          if (privAccess)
          {
            returnValue = FALSE;
            break;
          }
          else
          {
            throwDataAbort(context, address, section ? dfsPermissionSection : dfsPermissionPage, isWrite, dom);
            returnValue = TRUE;
            break;
          }
        }
        case PRIV_RW_USR_RO:      // priv read/write, usr read only
        {
          if ((!privAccess) && (isWrite))
          {
            throwDataAbort(context, address, section ? dfsPermissionSection : dfsPermissionPage, isWrite, dom);
            returnValue = TRUE;
            break;
          }
          else
          {
            returnValue = FALSE;
            break;
          }
        }
        case PRIV_RW_USR_RW:       // priv read/write, usr read/write
        {
          returnValue = FALSE;
          break;
        }
        case AP_RESERVED:         // reserved!
        {
          printf("shouldDataAbort: guestFirst %08x @ %p\n", *(u32int*)guestFirst, guestFirst);
          printf("shouldDataAbort: guestSecond %08x @ %p\n", *(u32int*)guestSecond, guestSecond);
          DIE_NOW(context, "shouldDataAbort(): RESERVED access bits in PT entry!");
          break;
        }
        case PRIV_RO_USR_NO:      // priv read only, usr no access
        {
          if (!privAccess)
          {
            throwDataAbort(context, address, section ? dfsPermissionSection : dfsPermissionPage, isWrite, dom);
            returnValue = TRUE;
            break;
          }
          else if (isWrite)
          {
            throwDataAbort(context, address, section ? dfsPermissionSection : dfsPermissionPage, isWrite, dom);
            returnValue = TRUE;
            break;
          }
          else
          {
            returnValue = FALSE;
            break;
          }
        }
        case DEPRECATED:          // priv read only, usr read only
        case PRIV_RO_USR_RO:      // priv read only, usr read only
        {
          if (isWrite)
          {
            throwDataAbort(context, address, section ? dfsPermissionSection : dfsPermissionPage, isWrite, dom);
            returnValue = TRUE;
            break;
          }
          else
          {
            returnValue = FALSE;
            break;
          }
        }
        default:
          DIE_NOW(context, "shouldDataAbort: invalid AP bits.");
      } // AP bits switch ends
    } // case CLIENT ends
  } // switch domain bits ends

  if (shadowFirst != 0)
  {
    *(u32int*)shadowFirst = backupFirst;
#ifdef MEM_PROT_DBG
    printf("shouldDataAbort: restored entry %08x @ %p\n", backupFirst, shadowFirst);
#endif
    mmuInvalidateUTLBbyMVA((u32int)context->pageTables->guestPhysical);
    mmuDataMemoryBarrier();
  }
  if (shadowSecond != 0)
  {
    // we had to backup an entry for a gPT2 as well. restore now
    // restore hacked entry to shadow page table.
    *(u32int*)shadowSecond = backupSecond;
#ifdef MEM_PROT_DBG
    printf("shouldDataAbort: restored PT2 entry %08x @ %p\n", backupSecond, shadowSecond);
#endif
    mmuInvalidateUTLBbyMVA(gpt2);
    mmuDataMemoryBarrier();
  }
//  mmuInvalidateUTLB();
  return returnValue;
}


/**
 * function called when the host catches a prefetch abort exception
 * and hypervisor fails to shadow map the entry corresponding to the faulting address
 * it must be the case that the prefetch abort has to be forwarded to the guest
 **/
bool shouldPrefetchAbort(bool privAccess, u32int address)
{
#ifdef MEM_PROT_DBG
  printf("shouldPrefetchAbort(priv %x, address %08x)\n", privAccess, address);
#endif
  GCONTXT* context = getGuestContext();

  simpleEntry* guestFirst = 0; 
  simpleEntry* guestSecond = 0;
  simpleEntry* shadowFirst = 0;
  simpleEntry* shadowSecond = 0;
  u32int backupFirst = 0;
  u32int backupSecond = 0;
  simpleEntry* gpt = 0;
  u32int gpt2 = 0; // this is just a the physical address as unsigned int
  simpleEntry* spt = context->pageTables->shadowActive;
  u32int sysCtrlReg = getCregVal(1, 0, 0, 0, &context->coprocRegBank[0]);
/*
  if (sysCtrlReg & 0x00000002)
  {
    // guest enabled alignment checking (SCTRL.A) enabled
    // check alignment: if misaligned abort (alignment fault)
//    DIE_NOW(context, "shouldDataAbort: alignment checking enabled! unimplemented.\n");
  }*/

  if (context->pageTables->guestVirtual != 0)
  {
    // cool. we have the VA already! use it. 
#ifdef MEM_PROT_DBG
    printf("shouldPrefetchAbort: guestVirtual PT set, %p\n", context->pageTables->guestVirtual);
#endif
    gpt = context->pageTables->guestVirtual;
  }
  else
  {
#ifdef MEM_PROT_DBG
    printf("shouldPrefetchAbort: guestVirtual PT not set. hack a 1-2-1 of %p\n",
                                  context->pageTables->guestPhysical);
#endif
    // crap, we haven't shadow mapped the gPT VA->PA mapping yet.
    // hack a 1-2-1 mapping for now.
    shadowFirst = getEntryFirst(spt, (u32int)context->pageTables->guestPhysical);
    backupFirst = *(u32int*)shadowFirst;
    mapSection(context->pageTables->shadowActive, (u32int)context->pageTables->guestPhysical, 
              (u32int)context->pageTables->guestPhysical, HYPERVISOR_ACCESS_DOMAIN,
              HYPERVISOR_ACCESS_BITS, TRUE, FALSE, 0);
    mmuInvalidateUTLBbyMVA((u32int)context->pageTables->guestPhysical);
    gpt = context->pageTables->guestPhysical;
#ifdef MEM_PROT_DBG
    printf("shouldPrefetchAbort: gpt now set to %p\n", gpt);
#endif
  }

  bool returnValue = FALSE;

  /***************************** step 1 **************************/
  /* get PT entry (first, and second if needed), check for FAULT */ 
  /***************************************************************/
  guestFirst = getEntryFirst(gpt, address);
  switch(guestFirst->type)
  {
    case FAULT:
    {
#ifdef MEM_PROT_DBG
      printf("shouldPrefetchAbort: gpt entry lvl1 type FAULT!\n");
#endif
      throwPrefetchAbort(context, address, ifsTranslationFaultSection);
      returnValue = TRUE;
      break;
    }
    case RESERVED:
    {
      DIE_NOW(context, "shouldPrefetchAbort: first lvl entry RESERVED.");
    }
    case SECTION:
    {
      // first level entry section, valid. move to next check
      // must have prefetch aborted for another reason
      returnValue = FALSE;
      break;
    }
    case PAGE_TABLE:
    {
      // must check 2nd level PT entry
      // have to read guest 2nd lvl page table: hack a 1-2-1 memory mapping
      // in the shadow page table.
      gpt2 = (((pageTableEntry*)guestFirst)->addr) << 10;
      shadowSecond = getEntryFirst(context->pageTables->shadowActive, gpt2);
      backupSecond = *(u32int*)shadowSecond;
#ifdef MEM_PROT_DBG
      printf("shouldPrefetchAbort: backed up PT2 entry %08x @ %p\n", backupSecond, shadowSecond);
#endif
      mapSection(context->pageTables->shadowActive, gpt2, gpt2,
           HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, TRUE, FALSE, 0);
      mmuInvalidateUTLBbyMVA(gpt2);

      // now PT2 is shadow mapped 1-2-1 VA/PA. can extract second level entry.
      u32int index = (address & 0x000FF000) >> 10;
      u32int entryAddress = gpt2 | index;
      guestSecond = (simpleEntry*)entryAddress;
      if (guestSecond->type == FAULT)
      {
        // if second level entry fault, page prefetch translation fault
#ifdef MEM_PROT_DBG
        printf("shouldPrefetchAbort: abort - translation fault page!\n");
#endif
        returnValue = TRUE;
        throwPrefetchAbort(context, address, ifsTranslationFaultPage);
        break;
      }
    }
  } // switch ends
  
  // if we are prefetch aborting with Translation Fault, return already, exception thrown.
  if (returnValue)
  {
    // if we hacked a 1-2-1 entry of the guest first level page table, restore
    if (shadowFirst != 0)
    {
      *(u32int*)shadowFirst = backupFirst;
      mmuInvalidateUTLBbyMVA((u32int)context->pageTables->guestPhysical);
      mmuDataMemoryBarrier();
    }
    if (shadowSecond != 0)
    {
      // we had to backup an entry for a gPT2 as well. restore now
      // restore hacked entry to shadow page table.
      *(u32int*)shadowSecond = backupSecond;
      mmuInvalidateUTLBbyMVA(gpt2);
      mmuDataMemoryBarrier();
    }
    // and return
    return returnValue;
  }

  // check if access flag is enabled (SCTRL.AFE)
  if (sysCtrlReg & 0x20000000)
  {
    // check access flag: if AF=0 abort (access flag fault)
    DIE_NOW(context, "shouldPrefetchAbort: access flag enabled set, simplified AP model unimplemented.\n");
  }

  /***************************** step 2 **************************/
  /* check domain: access type manager/client/no access          */ 
  /***************************************************************/
  u32int dom  = ((sectionEntry*)guestFirst)->domain;
  u32int dacr = getCregVal(3, 0, 0, 0, &context->coprocRegBank[0]);
  u32int domBits = (dacr >> (dom*2)) & 0x3;
  
  switch (domBits)
  {
    case DACR_NO_ACCESS:
    {
      // no-access for domain: abort (domain fault)
      DIE_NOW(context, "shouldPrefetchAbort(): DACR 0: no access! unimplemented");
      break;
    }
    case DACR_RESERVED:
    {
      DIE_NOW(context, "shouldPrefetchAbort(): domain access control 2: reserved!");
      break;
    }
    case DACR_MANAGER:
    {
      // manager access; do not check page table entry bits, allow access
       returnValue = FALSE;
       break;
    }
    case DACR_CLIENT:
    {
      // client access: need to check page table entry access control bits
      bool section = FALSE;
      u8int permissionBits = 0;
      if (guestFirst->type == SECTION)
      {
        sectionEntry* guestSection = (sectionEntry*)guestFirst;
        permissionBits = guestSection->ap10 | (guestSection->ap2 << 2);
        section = TRUE;
      }
      else // this is going to be a guest 2nd lvl PAGE_TABLE
      {
        // doesnt matter if small/large page, AP bits in same place
        // guestSecond pointer was calculated and set when checking for translation faults
        // can still rely on the value being correct 
        smallPageEntry* guestSmallPage = (smallPageEntry*)guestSecond;
        permissionBits = guestSmallPage->ap10 | (guestSmallPage->ap2 << 2);
        section = FALSE;
      }

      switch (permissionBits)
      {
        case PRIV_NO_USR_NO:      // priv no access, usr no access
        {
          throwPrefetchAbort(context, address, section ? ifsPermissionFaultSection : ifsPermissionFaultPage);
          returnValue = TRUE;
          break;
        }
        case PRIV_RW_USR_NO:      //priv read/write, usr no access
        {
          if (!privAccess)
          {
            throwPrefetchAbort(context, address, section ? ifsPermissionFaultSection : ifsPermissionFaultPage);
            returnValue = TRUE;
            break;
          }
          else
          {
            returnValue = FALSE;
            break;
          }
        }
        case PRIV_RO_USR_NO:      // priv read only, usr no access
        {
          if (!privAccess)
          {
            throwPrefetchAbort(context, address, section ? ifsPermissionFaultSection : ifsPermissionFaultPage);
            returnValue = TRUE;
            break;
          }
          else
          {
            returnValue = FALSE;
            break;
          }
        }
        case AP_RESERVED:         // reserved!
        {
          DIE_NOW(context, "shouldPrefetch(): RESERVED access bits in PT entry!");
          break;
        }
        case PRIV_RW_USR_RO:      // priv read/write, usr read only
        case PRIV_RW_USR_RW:      // priv read/write, usr read/write
        case DEPRECATED:          // priv read only, usr read only
        case PRIV_RO_USR_RO:      // priv read only, usr read only
        {
          // only no-access mapping can cause prefetch abort. read-access allowed in these cases!
          returnValue = FALSE;
          break;
        }
        default:
        {
          DIE_NOW(context, "shouldPrefetch: invalid AP bits.");
        }
      } // AP bits switch ends
      break;
    } // DACR client case ends
  } // switch domBits ends

  // if we dont have gPT1 VA we must have backed up the lvl1 entry. restore now
  if (shadowFirst != 0)
  {
    *(u32int*)shadowFirst = backupFirst;
    mmuInvalidateUTLBbyMVA((u32int)context->pageTables->guestPhysical);
    mmuDataMemoryBarrier();
  }
  if (shadowSecond != 0)
  {
    // we had to backup an entry for a gPT2 as well. restore now
    // restore hacked entry to shadow page table.
    *(u32int*)shadowSecond = backupSecond;
    mmuInvalidateUTLBbyMVA(gpt2);
    mmuDataMemoryBarrier();
  }
  mmuInvalidateUTLB();
  return returnValue;
}
