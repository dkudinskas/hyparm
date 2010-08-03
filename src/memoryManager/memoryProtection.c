#include "memoryProtection.h"
#include "serial.h"
#include "memoryConstants.h"
#include "frameAllocator.h"
#include "common.h" //for memset
#include "guestContext.h"
#include "assert.h"

//uncomment to enable Memory Protection debug: #define MEM_PROT_DEBUG

extern GCONTXT * getGuestContext(void);

u32int maxEntries;
u32int addEntry(u32int startAddr, u32int endAddr, memProtPtr ptr, ACCESS_TYPE protection, bool multiPage);
u32int removeEntry(u32int startAddr, u32int endAddr);

MEMPROT* initialiseMemoryProtection(void)
{
  serial_putstring("Initialising memory protection array.");
  serial_newline();

  //grab a 4KB chunk of mem
  u32int* addr = allocFrame(HYPERVISOR_FA_DOMAIN);

  if(0 == addr)
  {
    serial_ERROR("Unable to allocate memory for initialiseProtectionArray() in memoryProtection.c");
  }

  u32int arraySpace = FRAME_TABLE_CHUNK_SIZE;
  memset(addr, 0, arraySpace);

  MEMPROT* memProt = (MEMPROT*)addr;
  memProt->maxEntries = arraySpace / sizeof(MPE);

#ifdef MEM_PROT_DEBUG
  serial_putstring("Space for memProt array: 0x");
  serial_putint_nozeros(arraySpace);
  serial_newline();
  serial_putstring(", size of single MEMPROT: ");
  serial_putint_nozeros(sizeof(MEMPROT));
  serial_newline();
  serial_putstring(", size of single memoryProtectionEntry: ");
  serial_putint_nozeros(sizeof(MPE));
  serial_newline();
  serial_putstring(", maxEntries: ");
  serial_putint_nozeros(memProt->maxEntries);
  serial_newline();
#endif

  return memProt;
}


/** Add memory protection
returns 0 on success
*/
u32int addProtection(u32int startAddr, u32int endAddr, memProtPtr ptr, ACCESS_TYPE protection)
{
#ifdef MEM_PROT_DEBUG
  serial_putstring("PARTIAL IMPLEMENTATION. addProtection (memoryProtection.c)");
  serial_newline();
#endif

  GCONTXT* gc = getGuestContext();
  //MEMPROT* memProt = gc->memProt;
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;

#ifdef MEM_PROT_DEBUG
  serial_putstring("addProtection: Start Addr: 0x");
  serial_putint(startAddr);
  serial_putstring(", End Addr: 0x");
  serial_putint(endAddr);
  serial_newline();
#endif

  if(startAddr > endAddr)
  {
    serial_putstring("Start addr: 0x");
    serial_putint(startAddr);
    serial_putstring(", is greater than the end addr: 0x");
    serial_putint(endAddr);
    serial_ERROR("Entering infinite loop.");
  }

  //Simple way to find out if we cross PT entries
  //get the page table entry end address and see if its less than the end of the addr range we want to protect
  u32int pageEndAddr = getPageEndAddr(ptd, startAddr);
  u32int result;

#ifdef MEM_PROT_DEBUG
  serial_putstring("addProtection: pageEndAddr = ");
  serial_putint(pageEndAddr);
  serial_newline();
#endif

  if(0 == pageEndAddr)
  {
    serial_ERROR("ERROR: invalid getPageEndAddr return value. Entering infinite loop");
  }

  if(endAddr <= pageEndAddr)
  {
#ifdef MEM_PROT_DEBUG
    serial_putstring("addProtection: Single entry, partially implemented");
    serial_newline();
#endif
    //If the end of the address range we want to protect is inside that of a single pageTableEntry
    //Add a single entry
    result = setAccessBits(ptd, startAddr, protection);
  }
  else
  {
    //We cross multiple page table entries for this protection range
#ifdef MEM_PROT_DEBUG
    serial_putstring("addProtection: Multi entry, partially implemented");
    serial_newline();
#endif

    u32int pageStartAddr = startAddr;

    //We are dealing with multiple pages, loop to mark all pages USR_ read only
    while(endAddr > pageEndAddr)
    {
#ifdef MEM_PROT_DEBUG
      serial_putstring("addProtection: CurrentStartAddr: 0x"); serial_putint(pageStartAddr);
      serial_putstring(", currentEndAddr: 0x"); serial_putint(pageEndAddr);
      serial_newline();
#endif

      result = setAccessBits(ptd, pageStartAddr, protection);
      if(result > 7)
      {
        //Last addEntry failed, remove all Multi entries (could be the first)
        serial_ERROR("setAccessBits failed, while loop");
      }
      pageStartAddr = pageEndAddr+1;
      pageEndAddr = getPageEndAddr(ptd, pageStartAddr);
      if(0 == pageEndAddr)
      {
        serial_ERROR("ERROR: invalid getPageEndAddr return value. Entering infinite loop");
      }
    }//loop

    //exited while loop, the end of the range we want to protect is not greater than the current pageEntry
    result = setAccessBits(ptd, pageStartAddr, protection);
  }
  if(result > 7)
  {
      //Last addEntry failed, remove the previous ones
    serial_ERROR("setAccessBits failed");
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

  serial_putstring("UNIMPLEMENTED: removeProtection. Setting memoryAddress: 0x");
  serial_putint(startAddr);
  serial_putstring(" R/W for now. Continuing");
  serial_newline();

  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;

  u32int result = setAccessBits(ptd, startAddr, PRIV_RW_USR_RW);
  if( result > 7)
  {
    serial_ERROR("removeProtection Failed. Infinite Loop");
    return result;
  }
  return 0;
}


/** Searches array for match to data abort addresses
returns the function ptr of found.
0 for not
*/
memProtPtr checkProtectionArray(u32int address)
{
  serial_ERROR("UNIMPLEMENTED: checkProtectionArray");
  return 0;
}


void compile_time_check_memory_protection(void)
{
  //If the size of these change check assumptions as to space available for entries
  //May need to allocate more memory if they get any bigger!
  COMPILE_TIME_ASSERT( sizeof(MPE) == 28 , _size_of_struct_memProtectionEntry_has_changed);
  COMPILE_TIME_ASSERT( sizeof(MEMPROT) == 8 , _size_of_struct_memProtection_has_changed);
  COMPILE_TIME_ASSERT( sizeof(PLIST) == 12 , _size_of_struct_pageList_has_changed);
}
