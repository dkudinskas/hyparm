#include "common/assert.h"
#include "common/debug.h"
#include "common/stddef.h"

#include "guestManager/guestContext.h"
#include "guestManager/guestExceptions.h"

#include "memoryManager/memoryConstants.h"
#include "memoryManager/memoryProtection.h"
#include "memoryManager/mmu.h"


/**
 * this function called when we want to make sure an address range is not writable by the guest
 **/
void guestWriteProtect(GCONTXT *gc, u32int startAddress, u32int endAddress)
{
#ifdef MEM_PROT_DBG
  printf("guestWriteProtect: startAddress %x; endAddress %x\n", startAddress, endAddress);
#endif
  // 1. get page table entry for this address

  if (startAddress > endAddress)
  {
    DIE_NOW(NULL, "guestWriteProtect: startAddress is great than the endAddress.");
  }

  if (gc->virtAddrEnabled)
  {
    writeProtectRange(gc, gc->pageTables->shadowUser, startAddress, endAddress);
    writeProtectRange(gc, gc->pageTables->shadowPriv, startAddress, endAddress);
  }
  else
  {
    writeProtectRange(gc, gc->hypervisorPageTable, startAddress, endAddress);
  }
}


void writeProtectRange(GCONTXT *gc, simpleEntry* pageTable, u32int start, u32int end)
{
#ifdef MEM_PROT_DBG
  printf("writeProtectRange: PT %p, start %x; end %x\n", pageTable, start, end);
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
        DIE_NOW(NULL, "writeProtectRange: reserved 1st level page table entry!");
      }
      default:
        DIE_NOW(NULL, "writeProtectRange: Unrecognized first level entry. Error.");
    }

    simpleEntry* secondEntry = getEntrySecond(gc, (pageTableEntry*)firstEntry, pageStartAddress);
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
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
      }
      default:
      {
        DIE_NOW(NULL, "writeProtectRange: Unrecognized second level entry");
      }
    }

    if(pageEndAddress == 0)
    {
      DIE_NOW(NULL, "writeProtectRange: invalid page end address value.");
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
bool shouldDataAbort(GCONTXT *context, bool privAccess, bool isWrite, u32int address)
{
#ifdef MEM_PROT_DBG
  printf("shouldDataAbort: priv %x, write %x, addr %x\n", privAccess, isWrite, address);
#endif

  simpleEntry* guestFirst = 0; 
  simpleEntry* guestSecond = 0;
  simpleEntry* gpt = 0;
  u32int gpt2 = 0; // this is just a the physical address as unsigned int
  u32int sysCtrlReg = context->coprocRegBank[CP15_SCTRL].value;

  if (sysCtrlReg & 0x00000002)
  {
    // guest enabled alignment checking (SCTRL.A) enabled
    // check alignment: if misaligned abort (alignment fault)
//    DIE_NOW(context, "shouldDataAbort: alignment checking enabled! unimplemented.\n");
  }

  simpleEntry* ttbrBackup = mmuGetTTBR0();
  mmuSetTTBR0(context->hypervisorPageTable, 0x1FF);

  gpt = context->pageTables->guestPhysical;

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
      printf("shouldDataAbort: gpt entry lvl1 type FAULT!\n");
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
      gpt2 = (((pageTableEntry*)guestFirst)->addr) << 10;
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
    mmuSetTTBR0(ttbrBackup, 0x100 | context->pageTables->contextID);
    return returnValue;
  }

  // check if access flag is enabled (SCTRL.AFE)
  if (sysCtrlReg & 0x20000000)
  {
    // check access flag: if AF=0 abort (access flag fault)
    // access flag enabled set, simplified AP model unimplemented!
    DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
  }

  // check domain
  u32int dom  = ((sectionEntry*)guestFirst)->domain;
  u32int dacr = context->coprocRegBank[CP15_DACR].value;
  u32int domBits = (dacr >> (dom*2)) & 0x3;
  
  switch (domBits)
  {
    case DACR_NO_ACCESS:
    {
      // no-access for domain: abort (domain fault)
      DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
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

  mmuSetTTBR0(ttbrBackup, 0x100 | context->pageTables->contextID);
  return returnValue;
}


/**
 * function called when the host catches a prefetch abort exception
 * and hypervisor fails to shadow map the entry corresponding to the faulting address
 * it must be the case that the prefetch abort has to be forwarded to the guest
 **/
bool shouldPrefetchAbort(GCONTXT *context, bool privAccess, u32int address)
{
#ifdef MEM_PROT_DBG
  printf("shouldPrefetchAbort(priv %x, address %08x)\n", privAccess, address);
#endif

  simpleEntry* guestFirst = 0; 
  simpleEntry* guestSecond = 0;
  simpleEntry* gpt = 0;
  u32int gpt2 = 0; // this is just a the physical address as unsigned int
  u32int sysCtrlReg = context->coprocRegBank[CP15_SCTRL].value;

  if (sysCtrlReg & 0x00000002)
  {
    // guest enabled alignment checking (SCTRL.A) enabled
    // check alignment: if misaligned abort (alignment fault)
//    DIE_NOW(context, "shouldDataAbort: alignment checking enabled! unimplemented.\n");
  }

  simpleEntry* ttbrBackup = mmuGetTTBR0();
  mmuSetTTBR0(context->hypervisorPageTable, 0x1FF);

  gpt = context->pageTables->guestPhysical;

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
      gpt2 = (((pageTableEntry*)guestFirst)->addr) << 10;
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
    mmuSetTTBR0(ttbrBackup, 0x100 | context->pageTables->contextID);
    return returnValue;
  }

  // check if access flag is enabled (SCTRL.AFE)
  if (sysCtrlReg & 0x20000000)
  {
    // check access flag: if AF=0 abort (access flag fault)
    // access flag enabled set, simplified AP model unimplemented
    DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
  }

  /***************************** step 2 **************************/
  /* check domain: access type manager/client/no access          */ 
  /***************************************************************/
  u32int dom  = ((sectionEntry*)guestFirst)->domain;
  u32int dacr = context->coprocRegBank[CP15_DACR].value;
  u32int domBits = (dacr >> (dom*2)) & 0x3;
  
  switch (domBits)
  {
    case DACR_NO_ACCESS:
    {
      // no-access for domain: abort (domain fault)
      DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
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

  mmuSetTTBR0(ttbrBackup, 0x100 | context->pageTables->contextID);
  return returnValue;
}
