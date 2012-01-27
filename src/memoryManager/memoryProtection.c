#include "common/assert.h"
#include "common/debug.h"
#include "common/memFunctions.h"

#include "guestManager/guestContext.h"
#include "guestManager/guestExceptions.h"

#include "memoryManager/frameAllocator.h"
#include "memoryManager/memoryConstants.h"
#include "memoryManager/memoryProtection.h"
#include "memoryManager/mmu.h"


extern GCONTXT * getGuestContext(void);

u32int maxEntries;
u32int addEntry(u32int startAddr, u32int endAddr, memProtPtr ptr, ACCESS_TYPE protection, bool multiPage);
u32int removeEntry(u32int startAddr, u32int endAddr);

MEMPROT* initialiseMemoryProtection(void)
{
#ifdef MEM_PROT_DBG
  printf("Initialising memory protection array.\n");
#endif

  //grab a 4KB chunk of mem
  u32int* addr = allocFrame(HYPERVISOR_FA_DOMAIN);

  if (0 == addr)
  {
    DIE_NOW(0, "Unable to allocate memory for initialiseProtectionArray() in memoryProtection.c");
  }
  u32int arraySpace = FRAME_TABLE_CHUNK_SIZE;
  memset(addr, 0, arraySpace);

  MEMPROT* memProt = (MEMPROT*)addr;
  memProt->maxEntries = arraySpace / sizeof(MPE);

#ifdef MEM_PROT_DBG
  printf("Space for memProt array: %x, size of single MEMPROT: %x\n", arraySpace, sizeof(MEMPROT));
  printf("Size of single memoryProtectionEntry: %x, , maxEntries: %x", sizeof(MPE), memProt->maxEntries);
#endif

  return memProt;
}


/** Add memory protection
returns 0 on success
*/
u32int addProtection(u32int startAddr, u32int endAddr, memProtPtr ptr, ACCESS_TYPE protection)
{
#ifdef MEM_PROT_DBG
  printf("PARTIAL IMPLEMENTATION. addProtection (memoryProtection.c)\n");
#endif

  GCONTXT* gc = getGuestContext();

  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;

#ifdef MEM_PROT_DBG
  printf("addProtection: Start Addr: %08x, End Addr: %08x\n", startAddr, endAddr);
#endif

  if (startAddr > endAddr)
  {
    printf("Start addr: %08x, is greater than the end addr: %08x\n", startAddr, endAddr);
    DIE_NOW(gc, "Entering infinite loop.");
  }

  //Simple way to find out if we cross PT entries
  //get the page table entry end address and see if its less than the end of the addr range we want to protect
  u32int pageEndAddr = getPageEndAddr(ptd, startAddr);
  u32int result;

#ifdef MEM_PROT_DBG
  printf("addProtection: pageEndAddr = %08x\n", pageEndAddr);
#endif

  if (pageEndAddr == 0)
  {
    DIE_NOW(0, "ERROR: invalid getPageEndAddr return value. Entering infinite loop");
  }

  if(endAddr <= pageEndAddr)
  {
#ifdef MEM_PROT_DBG
    printf("addProtection: Single entry, partially implemented\n");
#endif
    //If the end of the address range we want to protect is inside that of a single pageTableEntry
    //Add a single entry
    result = setAccessBits(ptd, startAddr, protection);
  }
  else
  {
    //We cross multiple page table entries for this protection range
#ifdef MEM_PROT_DBG
    printf("addProtection: Multi entry, partially implemented\n");
#endif

    u32int pageStartAddr = startAddr;

    //We are dealing with multiple pages, loop to mark all pages USR_ read only
    while (endAddr > pageEndAddr)
    {
#ifdef MEM_PROT_DBG
      printf("addProtection: CurrentStartAddr: %x, currentEndAddr: %x\n", pageStartAddr, pageEndAddr);
#endif

      result = setAccessBits(ptd, pageStartAddr, protection);
      if (result > 7)
      {
        //Last addEntry failed, remove all Multi entries (could be the first)
        DIE_NOW(0, "setAccessBits failed, while loop");
      }
      pageStartAddr = pageEndAddr+1;
      pageEndAddr = getPageEndAddr(ptd, pageStartAddr);
      if (0 == pageEndAddr)
      {
        DIE_NOW(0, "ERROR: invalid getPageEndAddr return value. Entering infinite loop");
      }
    }//loop

    //exited while loop, the end of the range we want to protect is not greater than the current pageEntry
    result = setAccessBits(ptd, pageStartAddr, protection);
  }
  if (result > 7)
  {
      //Last addEntry failed, remove the previous ones
    DIE_NOW(0, "setAccessBits failed");
  }
  return result;
}


/**
 * Assumes calling fuction that protected the specific area of mem, remembers the start addresses
 * Returns 0 on sucess
 */
u32int removeProtection(u32int startAddr)
{
  /* Simple prototype implementation for now. Until proper memoryProtection is written */
  printf("UNIMPLEMENTED: removeProtection. Setting memoryAddress: %08x R/W for now.\n", startAddr);

  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;

  u32int result = setAccessBits(ptd, startAddr, PRIV_RW_USR_RW);
  if (result > 7)
  {
    DIE_NOW(0, "removeProtection Failed. Infinite Loop");
  }
  return 0;
}


/** Searches array for match to data abort addresses
returns the function ptr of found.
0 for not
*/
memProtPtr checkProtectionArray(u32int address)
{
  DIE_NOW(0, "UNIMPLEMENTED: checkProtectionArray");
}


void compile_time_check_memory_protection(void)
{
  //If the size of these change check assumptions as to space available for entries
  //May need to allocate more memory if they get any bigger!
  COMPILE_TIME_ASSERT( sizeof(MPE) == 28 , _size_of_struct_memProtectionEntry_has_changed);
  COMPILE_TIME_ASSERT( sizeof(MEMPROT) == 8 , _size_of_struct_memProtection_has_changed);
  COMPILE_TIME_ASSERT( sizeof(PLIST) == 12 , _size_of_struct_pageList_has_changed);
}

// returns abort flag if access is denied
bool shouldDataAbort(bool privAccess, bool isWrite, u32int address)
{
  GCONTXT* context = getGuestContext();
  descriptor* pt1Entry;

  /* get page table entry for address
   * If the Guest OS does not support virtual
   * addressing, The table that maps the guest physical
   * to host virtual must be used */
  if (context->virtAddrEnabled == 1)
  {
    pt1Entry = get1stLevelPtDescriptorAddr(context->PT_os, address);
  }
  else
  {
    pt1Entry = get1stLevelPtDescriptorAddr(context->PT_physical, address);
  }

  // check guest domain: if manager, allow access.
  u32int dacr = getCregVal(3, 0, 0, 0, &context->coprocRegBank[0]);
  u32int dom  = ((sectionDescriptor*)pt1Entry)->domain;

  u32int domBits = (dacr >> (dom*2)) & 0x3;

  switch (domBits)
  {
    case 0:
    {
      // throw guest data abort with domain_fault!
      DIE_NOW(context, "shouldDataAbort(): domain access control 0: no access!");
      break;
    }
    case 1:
    {
      // client access: need to check page table entry access control bits
      switch (pt1Entry->type)
      {
        case FAULT:
        {
#ifdef MEM_PROT_DBG
          printf("shouldDataAbort(): dacr 1, ptEntry type fault!\n");
#endif
          throwDataAbort(address, dfsTranslationSection, isWrite, dom);
          return TRUE;
        }
        case SECTION:
        {
          sectionDescriptor* ptEntry = (sectionDescriptor*)pt1Entry;
          u8int accPerm = ptEntry->ap10 | (ptEntry->ap2 << 2);
          switch (accPerm)
          {
            case PRIV_NO_USR_NO:      //priv no access, usr no access
            {
              throwDataAbort(address, dfsPermissionSection, isWrite, dom);
              return TRUE;
            }
            case PRIV_RW_USR_NO:      //priv read/write, usr no access
            {
              if (privAccess)
              {
                return FALSE;
              }
              else
              {
                throwDataAbort(address, dfsPermissionSection, isWrite, dom);
                return TRUE;
              }
            }
            case PRIV_RW_USR_RO:      // priv read/write, usr read only
            {
              if ((!privAccess) && (isWrite))
              {
                throwDataAbort(address, dfsPermissionSection, isWrite, dom);
                return TRUE;
              }
              else
              {
                return FALSE;
              }
            }
            case PRIV_RW_USR_RW:       // priv read/write, usr read/write
            {
              return FALSE;
            }
            case AP_RESERVED:         // reserved!
            {
              DIE_NOW(context, "shouldDataAbort(): RESERVED access bits in PT entry!");
            }
            case PRIV_RO_USR_NO:      // priv read only, usr no access
            {
              if (!privAccess)
              {
                throwDataAbort(address, dfsPermissionSection, isWrite, dom);
                return TRUE;
              }
              else if (isWrite)
              {
                throwDataAbort(address, dfsPermissionSection, isWrite, dom);
                return TRUE;
              }
              else
              {
                return FALSE;
              }
            }
            case DEPRECATED:          // priv read only, usr read only
            case PRIV_RO_USR_RO:      // priv read only, usr read only
            {
              if (isWrite)
              {
                throwDataAbort(address, dfsPermissionSection, isWrite, dom);
                return TRUE;
              }
              else
              {
                return FALSE;
              }
            }
          } // AP bits switch ends
          break;
        } // case SECTION: ends
        case PAGE_TABLE:
        {
          pageTableDescriptor* ptDesc = (pageTableDescriptor*)pt1Entry;
          smallDescriptor* ptEntry =
             (smallDescriptor*)get2ndLevelPtDescriptor((pageTableDescriptor*)ptDesc, address);
          switch (((descriptor*)ptEntry)->type)
          {
            case SMALL_PAGE:
            case SMALL_PAGE_3:
            {
              u8int accPerm = ptEntry->ap10 | (ptEntry->ap2 << 2);
              switch (accPerm)
              {
                case PRIV_NO_USR_NO:      //priv no access, usr no access
                {
                  throwDataAbort(address, dfsPermissionPage, isWrite, dom);
                  return TRUE;
                }
                case PRIV_RW_USR_NO:      //priv read/write, usr no access
                {
                  if (privAccess)
                  {
                    return FALSE;
                  }
                  else
                  {
                    throwDataAbort(address, dfsPermissionPage, isWrite, dom);
                    return TRUE;
                  }
                }
                case PRIV_RW_USR_RO:      // priv read/write, usr read only
                {
                  if ((!privAccess) && (isWrite))
                  {
                    throwDataAbort(address, dfsPermissionPage, isWrite, dom);
                    return TRUE;
                  }
                  else
                  {
                    return FALSE;
                  }
                }
                case PRIV_RW_USR_RW:       // priv read/write, usr read/write
                {
                  return FALSE;
                }
                case AP_RESERVED:         // reserved!
                {
                  DIE_NOW(context, "shouldDataAbort(): RESERVED access bits in PT entry!");
                }
                case PRIV_RO_USR_NO:      // priv read only, usr no access
                {
                  if (!privAccess)
                  {
                    throwDataAbort(address, dfsPermissionPage, isWrite, dom);
                    return TRUE;
                  }
                  else if (isWrite)
                  {
                    throwDataAbort(address, dfsPermissionPage, isWrite, dom);
                    return TRUE;
                  }
                  else
                  {
                    return FALSE;
                  }
                }
                case DEPRECATED:          // priv read only, usr read only
                case PRIV_RO_USR_RO:      // priv read only, usr read only
                {
                  if (isWrite)
                  {
                    throwDataAbort(address, dfsPermissionPage, isWrite, dom);
                    return TRUE;
                  }
                  else
                  {
                    return FALSE;
                  }
                }
              } // AP bits switch ends
              break;
            }
            case LARGE_PAGE:
            {
              DIE_NOW(context, "shouldAbort: 2nd lvl ptEntry - large page. unimplemented.");
            }
            case FAULT:
            {
              throwDataAbort(address, dfsTranslationPage, isWrite, dom);
              return TRUE;
            }
          }
          break;
        }
        case RESERVED:
        {
          DIE_NOW(context, "shouldDataAbort: guest 1st lvl pt entry: reserved!");
          break;
        }
      }
      break;
    } // client access: ends
    case 2:
      DIE_NOW(context, "shouldDataAbort(): domain access control 2: reserved!");
      break;
    case 3:
      // manager access: do NOT check page table entry AP bits.
      // however, pt entry must be valid.
      if (pt1Entry->type == FAULT)
      {
        throwDataAbort(address, dfsTranslationSection, isWrite, dom);
        return TRUE;
      }
      else
      {
        return FALSE;
      }
  } // domain switch ends

  DIE_NOW(context, "shouldDataAbort: should never get to end of function.");
  // compiler happy
  return FALSE;
}


bool shouldPrefetchAbort(u32int address)
{
  GCONTXT* context = getGuestContext();
#ifdef MEM_PROT_DBG
  printf("shouldPrefetchAbort(%08x)\n", address);
#endif
  descriptor* ptEntry;
  // get page table entry for address
  if(context->virtAddrEnabled==1)
  {
    ptEntry = get1stLevelPtDescriptorAddr(context->PT_os, address);
  }
  else
  {
    ptEntry = get1stLevelPtDescriptorAddr(context->PT_physical, address);
  }

  // client access: need to check page table entry access control bits
  switch (ptEntry->type)
  {
    case FAULT:
    {
#ifdef MEM_PROT_DBG
      printf("shouldPrefetchAbort(): Lvl1 ptEntry type FAULT!\n");
#endif
      throwPrefetchAbort(address, ifsTranslationFaultSection);
      return TRUE;
    }
    case SECTION:
    {
      DIE_NOW(context, "shouldPrefetchAbort: guest PT1 entry Section. Investigate.");
      break;
    }
    case PAGE_TABLE:
    {
#ifdef MEM_PROT_DBG
      printf("shouldPrefetchAbort(): Lvl1 ptEntry type PageTable!\n");
#endif
      // get 2nd level table entry address
      descriptor* ptd2nd = get2ndLevelPtDescriptor((pageTableDescriptor*)ptEntry, address);
      switch (ptd2nd->type)
      {
        case SMALL_PAGE:
        case SMALL_PAGE_3:
        {
          DIE_NOW(context, "shouldPrefetchAbort(): Lvl2 ptEntry type SmallPage! Investigate.");
        }
        case LARGE_PAGE:
        {
          DIE_NOW(context, "shouldPrefetchAbort(): Lvl2 ptEntry type Large! Investigate.");
        }
        case FAULT:
        {
          throwPrefetchAbort(address, ifsTranslationFaultPage);
          return TRUE;
        }
      }
      break;
    }
    case RESERVED:
    {
      DIE_NOW(context, "shouldPrefetchAbort: guest PT1 entry Reserved. Investigate.");
      break;
    }
  }

  // compiler happy
  return FALSE;
}
