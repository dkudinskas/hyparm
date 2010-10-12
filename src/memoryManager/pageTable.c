#include "pageTable.h"
#include "memoryConstants.h"
#include "serial.h"
#include "frameAllocator.h"
#include "memFunctions.h" //for memset
#include "mmu.h" // for setDomain
#include "guestContext.h"
#include "beGPTimer.h" /// should not be included here

//Uncomment to enable all page table debugging: #define PT_DEBUG
//Uncomment to show entries being mapped #define PT_SHORT_DEBUG
//Uncomment to show shadow page table code:#define PT_SHADOW_DEBUG
#define PT_DUMP_DEBUG

//Useful to have pageTableDumps with shadow debug
#ifdef PT_SHADOW_DEBUG
#define PT_DUMP_DEBUG
#endif

//Enabled everything
#ifdef PT_DEBUG
#define PT_SMALL_DEBUG
#define PT_LARGE_DEBUG
#define PT_SECTION_DEBUG
#define PT_SUPERSECTION_DEBUG
#define PT_SHADOW_DEBUG
#define PT_SHORT_DEBUG
#define PT_DUMP_DEBUG
#endif


extern GCONTXT * getGuestContext(void);

descriptor* hypervisorPtd;

u32int guestSecondLvlPageTables[256];

/* Don't want to make the hypervisorPtd global */
void setGuestPhysicalPt(GCONTXT* gc)
{
  gc->PT_physical = hypervisorPtd;
}

descriptor* createHypervisorPageTable()
{
  // zero metadata array
  int i = 0;
  for (i = 0; i < 256; i++)
  {
    guestSecondLvlPageTables[i] = 0;
  }

  //alloc some space for our 1st Level page table
  hypervisorPtd = createNew1stLevelPageTable(HYPERVISOR_FA_DOMAIN);

  //map in the hypervisor
  mapHypervisorMemory(hypervisorPtd);

  //set the domain (access control) for the hypervisor pages
  setDomain(HYPERVISOR_ACCESS_DOMAIN, client);

  //1:1 Map the entire of physical memory

  //small page map the first MB of mem so we can best protect the bootstrap code from self modification
  smallMapMemory(hypervisorPtd, MEMORY_START_ADDR, (MEMORY_START_ADDR + LARGE_PAGE_SIZE -1), GUEST_ACCESS_DOMAIN, GUEST_ACCESS_BITS, 0, 0, 0b000);
  smallMapMemory(hypervisorPtd, (MEMORY_START_ADDR + LARGE_PAGE_SIZE), (MEMORY_START_ADDR +SECTION_SIZE -1), GUEST_ACCESS_DOMAIN, GUEST_ACCESS_BITS, 0, 0, 0b000);
  sectionMapMemory(hypervisorPtd, (MEMORY_START_ADDR +SECTION_SIZE), (HYPERVISOR_START_ADDR-1), GUEST_ACCESS_DOMAIN, GUEST_ACCESS_BITS, 0, 0, 0b000);

  setDomain(GUEST_ACCESS_DOMAIN, client);

  //serial
  const u32int serial = SERIAL_BASE;
  if(addSmallPtEntry(hypervisorPtd, serial, serial,GUEST_ACCESS_DOMAIN, GUEST_ACCESS_BITS, 0, 0, 0) != 0)
  {
    serial_ERROR("Added serial mapping failed. Entering infinite loop.");
  }

  // uart1
  const u32int uart1 = 0x4806a000;
  if(addSmallPtEntry(hypervisorPtd, uart1, uart1,GUEST_ACCESS_DOMAIN, GUEST_ACCESS_BITS, 0, 0, 0) != 0)
  {
    serial_ERROR("Added uart1 mapping failed. Entering infinite loop.");
  }

  // interrupt controller
  const u32int interruptController = 0x48200000;
  addSectionPtEntry(hypervisorPtd, interruptController,interruptController,HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0);

  // gptimer1 - this looks dirty
  addSectionPtEntry(hypervisorPtd, GPTIMER1,GPTIMER1,HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0);

  // gptimer2
  const u32int gptimer2Addr = 0x49032000;
  if(addSmallPtEntry(hypervisorPtd,gptimer2Addr,gptimer2Addr,GUEST_ACCESS_DOMAIN, GUEST_ACCESS_BITS, 0, 0, 0) != 0)
  {
    serial_ERROR("Added gptimer2 mapping failed. Entering infinite loop.");
  }

  /*
  Exception vectors

  For some reason these do not seem to work unless mapped in as sections?!
  */

  //add section mapping for 0x14000 (base exception vectors)
  const u32int exceptionHandlerAddr = 0x14000;
  addSectionPtEntry(hypervisorPtd, exceptionHandlerAddr,exceptionHandlerAddr,HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0);

  //add section mapping for 0x40200000 (redirected exception vectors)
  //We will want to use the exception handler remap feature to put the page tables in the 0xffff0000 address space later
  const u32int exceptionHandlerRedirectAddr = 0x4020ffd0;
  addSectionPtEntry(hypervisorPtd, exceptionHandlerRedirectAddr,exceptionHandlerRedirectAddr,HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0);

  return hypervisorPtd;
}

descriptor* createGuestOSPageTable()
{
  descriptor* ptd = createNew1stLevelPageTable(HYPERVISOR_FA_DOMAIN);

  mapHypervisorMemory(ptd);

  //Map Serial
  const u32int serial = SERIAL_BASE;
  if(addSmallPtEntry(ptd, serial, serial,HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0) != 0)
  {
    serial_ERROR("Added serial mapping failed. Entering infinite loop");
  }

  /*  Map Exception vectors */

  //add section mapping for 0x14000 (base exception vectors)
  const u32int exceptionHandlerAddr = 0x14000;
  addSectionPtEntry(ptd, exceptionHandlerAddr,exceptionHandlerAddr,HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0);

  //add section mapping for 0x40200000 (redirected exception vectors)
  //We will want to use the exception handler remap feature to put the page tables in the 0xffff0000 address space later
  const u32int exceptionHandlerRedirectAddr = 0x4020ffd0;
  addSectionPtEntry(ptd, exceptionHandlerRedirectAddr, exceptionHandlerRedirectAddr, HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0);

  // interrupt controller
  const u32int interruptController = 0x48200000;
  addSectionPtEntry(ptd, interruptController,interruptController,HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0);

  // gptimer1
  addSectionPtEntry(ptd, GPTIMER1,GPTIMER1,HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0);

  //Map gptimer2
  const u32int gptimer2Addr = 0x49032000;
  if(addSmallPtEntry(ptd, gptimer2Addr, gptimer2Addr,HYPERVISOR_ACCESS_DOMAIN,HYPERVISOR_ACCESS_BITS, 0, 0, 0) != 0)
  {
    serial_ERROR("Added gptimer2 mapping failed. Entering infinite loop");
  }

#ifdef PT_SHADOW_DEBUG
  serial_putstring("New shadow PT dump @ 0x");
  serial_putint((u32int)ptd);
  serial_newline();
  dumpPageTable(ptd);
  serial_putstring("End shadow PT dump.");
#endif

  return ptd;
}

descriptor* createNew1stLevelPageTable(u8int domain)
{
  //alloc some some space for the 1stLevel table
  u32int* ptd = allocMultipleFrames(CHUNKS_FOR_PAGE_TABLE, domain);

  if(0 == (u32int)ptd)
  {
    //ERROR
    serial_ERROR("Frame alloc for 1st level page table failed. Entering Infinite Loop.");
  }

  //test ptd is correctly aligned
  if(((u32int)ptd & ~CHUNKS_FOR_PAGE_TABLE) != (u32int)ptd)
  {
    serial_ERROR("New 1st level page table is not correctly aligned. Entering infinite loop.");
  }

#ifdef PT_SHORT_DEBUG
  serial_putstring("Page Table base addr: 0x");
  serial_putint((u32int)ptd);
  serial_newline();
#endif

  //zero it
  memset(ptd, 0, (PAGE_TABLE_ENTRIES * PAGE_TABLE_ENTRY_WIDTH));
  return (descriptor*)ptd;
}

void sectionMapMemory(descriptor* ptd, u32int startAddr, u32int endAddr, u8int domain, u8int accessBits, u8int c, u8int b, u8int tex)
{
  while (startAddr < endAddr)
  {
#ifdef PT_SECTION_DEBUG
    serial_putstring("Address to 1:1 section map in: 0x");
    serial_putint(startAddr);
    serial_putstring(", domain: ");
    serial_putint_nozeros(domain);
    serial_putstring(", accessBits: ");
    serial_putint_nozeros(accessBits);
    serial_newline();
#endif

    u32int result = addSectionPtEntry(ptd, startAddr, startAddr, domain, accessBits, c, b, tex);
    if(result)
    {
#ifdef PT_SECTION_DEBUG
      serial_putstring("addSectionPtEntry failed.");
      serial_newline();
#endif
      break;
    }
    startAddr += SECTION_SIZE;
  }
}

void smallMapMemory(descriptor* ptd, u32int startAddr, u32int endAddr, u8int domain, u8int accessBits, u8int c, u8int b, u8int tex)
{
  while (startAddr < endAddr)
  {
#ifdef PT_SMALL_DEBUG
    serial_putstring("Address to 1:1 large page map in: 0x");
    serial_putint(startAddr);
    serial_putstring(", domain: ");
    serial_putint_nozeros(domain);
    serial_putstring(", accessBits: ");
    serial_putint_nozeros(accessBits);
    serial_newline();
#endif

    u32int result = addSmallPtEntry(ptd, startAddr, startAddr, domain, accessBits, c, b, tex);
    if(result)
    {
#ifdef PT_SMALL_DEBUG
      serial_putstring("addSmallPtEntry failed.");
      serial_newline();
#endif
      break;
    }
    startAddr += SMALL_PAGE_SIZE;
  }
}


void largeMapMemory(descriptor* ptd, u32int startAddr, u32int endAddr, u8int domain, u8int accessBits, u8int c, u8int b, u8int tex)
{
  while (startAddr < endAddr)
  {
#ifdef PT_LARGE_DEBUG
    serial_putstring("Address to 1:1 large map in: 0x");
    serial_putint(startAddr);
    serial_putstring(", domain: ");
    serial_putint_nozeros(domain);
    serial_putstring(", accessBits: ");
    serial_putint_nozeros(accessBits);
    serial_newline();
#endif

    u32int result = addLargePtEntry(ptd, startAddr, startAddr, domain, accessBits, c, b, tex);
    if(result)
    {
#ifdef PT_LARGE_DEBUG
      serial_putstring("addlargePtEntry failed.");
      serial_newline();
#endif
      break;
    }
    startAddr += LARGE_PAGE_SIZE;
  }
}

void mapHypervisorMemory(descriptor* ptd)
{
  //TODO: endAddr needs to be better defined
  u32int startAddr = HYPERVISOR_START_ADDR;
  u32int endAddr = startAddr + TOTAL_MACHINE_RAM/4;

  sectionMapMemory(ptd, startAddr, endAddr, HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 1, 0, 0b000);
}

/* We come here first, to check that stupid things don't occur whe mapping memory -> indicating potential problems */
u32int addSectionPtEntry(descriptor* ptd, u32int virtual, u32int physical, u8int domain, u8int accessBits, u8int c, u8int b, u8int tex)
{
#ifdef PT_SECTION_DEBUG
  serial_putstring("addSectionPtEntry. Virtual Addr: 0x");
  serial_putint(virtual);
  serial_putstring(", physical addr: 0x");
  serial_putint(physical);
  serial_newline();
#endif

  /*
  First we check for any existing entries
  Before adding a new one.
  */

  descriptor* ptd1st;
  ptd1st = get1stLevelPtDescriptorAddr(ptd, virtual);
  switch(ptd1st->type)
  {
    case FAULT:
      //no existing entry
#ifdef PT_SECTION_DEBUG
      serial_putstring("Page Type: FAULT.  Creating new section descriptor");
      serial_newline();
#endif
      addNewSectionDescriptor((sectionDescriptor*)ptd1st, physical, domain, accessBits, c, b, tex);
      return 0;
      break;
    case SECTION:
    {
      sectionDescriptor* sd = (sectionDescriptor*)ptd1st;
      if((sd->addr == (physical >> 20)) && (sd->sectionType == 0))
      {
        serial_ERROR("pageTable.c Attempting to re-add section entry with same mapping. Entering infinite loop");

        return 0; //probably ok to continue if it's the same mapping
      }
      else
      {
        //error!
        serial_putstring("Page Table entry: ");
        serial_putint((u32int) ptd1st);
        serial_putstring("at address: 0x");
        serial_putint(*(u32int*)ptd1st);
        serial_putstring(", descriptor: 0x");
        serial_putint(*(u32int*)ptd1st);
        serial_ERROR(", is already mapped.");
        return 1;
      }
      break;
    }
    case PAGE_TABLE:
      //error!
      serial_ERROR("Attempting to add section entry over the top of existing small/large sub-table descriptor. Entering infinite loop.");
      return 1;
      break;
    case RESERVED:
    default:
      serial_ERROR("Reserved Page table entry! Entering infinite loop.");
      return 1;
      break;
  }//switch

  //if we get here then we have not explicitly returned and want to stop here for debugging
  serial_ERROR("Entering infinite loop. addSectionPtEntry");
  return 1;
}

/* Returns 0 if completed with no errors
   non zero for error?
*/
u32int addSmallPtEntry(descriptor* ptd, u32int virtual, u32int physical, u8int domain, u8int accessBits, u8int c, u8int b, u8int tex)
{
#ifdef PT_SMALL_DEBUG
  serial_putstring("addSmallPtEntry: Virtual Addr: 0x");
  serial_putint(virtual);
  serial_putstring(", physical: 0x");
  serial_putint(physical);
  serial_putstring(", dom: ");
  serial_putint_nozeros(domain);
  serial_putstring(" AP: ");
  serial_putint_nozeros(accessBits);
  serial_putstring(" c: ");
  serial_putint_nozeros(c);
  serial_putstring(" b: ");
  serial_putint_nozeros(b);
  serial_putstring(" tex: ");
  serial_putint_nozeros(tex);
  serial_newline();
#endif
  /* Two stage process
  *First we need to check for an existing 1st Level page table entry
  *If not found then create it
  *Then we check the second level
  *If found check if it is the same mapping -> true error and continue
  *found and not the same error and crash (overwrite in future?)
  *else create entry :)
   */

  /* retrieve 1st level entry */
  descriptor* ptd1st;
  ptd1st = get1stLevelPtDescriptorAddr(ptd, virtual);
  switch(ptd1st->type)
  {
    case FAULT:
    {
    //no entry
#ifdef PT_SMALL_DEBUG
      serial_putstring("addSmallPtEntry: Page Type: FAULT.  Creating new descriptor");
      serial_newline();
#endif
      u32int result = addNewPageTableDescriptor(ptd1st, virtual, domain);
      if(0 != result)
      {
#ifdef PT_SMALL_DEBUG
        serial_putstring("Creation of new 1st level descriptor failed. At address : 0x");
        serial_putint((u32int)ptd1st);
        serial_putstring("For virtual address: 0x");
        serial_putint(virtual);
        serial_newline();
        serial_ERROR("Error.");
#endif
        return result;
      }
      break;
    }
    case PAGE_TABLE:
#ifdef PT_SMALL_DEBUG
      serial_putstring("addSmallPtEntry: Page Type: page table, correct.");
      serial_newline();
#endif
      /* Then we have the correct type for a small/large pt descriptor */
      break;
    case SECTION:
      serial_putstring("Page Table entry: ");
      serial_putint((u32int) ptd1st);
      serial_putstring("at address: 0x");
      serial_putint(*(u32int*)ptd1st);
      serial_putstring(", is a section/supersection.");
      serial_newline();
      serial_ERROR("Entering infinite loop.");
      break;
    case RESERVED:
    default:
      //error!
      serial_putstring("RESERVED type: 0x");
      serial_putint((u32int) ptd1st);
      serial_putstring("at address: 0x");
      serial_putint(*(u32int*)ptd1st);
      serial_ERROR("Entering infinite loop.");
  }//switch

  /* At this point we know its a small / large page table descriptor */

  /* Retrieve second level entry */
  descriptor* ptd2nd;
  ptd2nd = get2ndLevelPtDescriptor((pageTableDescriptor*)ptd1st, virtual);

  /* Again here we want to check the existing entry is no valid, could indicate problems elsewhere */

  if(FAULT == ptd2nd->type)
  {
#ifdef PT_SMALL_DEBUG
    serial_putstring("2nd Level FAULT. Creating new small descriptor");
    serial_newline();
#endif
    addNewSmallDescriptor((smallDescriptor*)ptd2nd, physical, accessBits, c, b, tex);
  }
  else //Existing small or large page
  {
#ifdef PT_SMALL_DEBUG
    serial_putstring("Error Entry for VirtualAddr: 0x");
    serial_putint(virtual);
    serial_putstring(", already exists");
    serial_newline();
#endif

    /* Dump some more debug */
    u32int physicalAddr = physical >> 12;
    u32int ptPhyscialAddr = *(u32int*)ptd2nd >> 12;
    if(physicalAddr == (physicalAddr & ptPhyscialAddr))
    {
      //match
#ifdef PT_SMALL_DEBUG
      serial_putstring(", attempt to re-add existing entry for PhysicalAddr: 0x");
      serial_putint(physical);
      serial_putstring("continuing.");
      serial_newline();
#endif
    }
    else
    {
      //Overwrite?
      serial_putstring(", attempt to overwrite existing entry for PhysicalAddr: 0x");
      serial_putint(physical);
      serial_newline();
      serial_ERROR("Entering infinte loop");
      return 1;
    }
  }
  return 0;
}

/* Returns 0 if completed with no errors
   non zero for error?
*/
u32int addLargePtEntry(descriptor* ptd, u32int virtual, u32int physical, u8int domain, u8int accessBits, u8int c, u8int b, u8int tex)
{
#ifdef PT_LARGE_DEBUG
  serial_putstring("addLargePtEntry. Virtual Addr: 0x");
  serial_putint(virtual);
  serial_putstring(", physical: 0x");
  serial_putint(physical);
  serial_newline();
#endif
  /* Two stage process
  *First we need to check for an existing 1st Level page table entry
  *If not found then create it
  *Then we check the second level
  *If found check if it is the same mapping -> true error and continue
  *found and not the same error and crash (overwrite in future?)
  *else create entry :)
  */

  /* retrieve 1st level entry */
  descriptor* ptd1st;
  ptd1st = get1stLevelPtDescriptorAddr(ptd, virtual);

  switch(ptd1st->type)
  {
    case FAULT:
      ;
    //no entry
#ifdef PT_LARGE_DEBUG
      serial_putstring("Page Type: FAULT.  Creating new descriptor");
      serial_newline();
#endif
      u32int result = addNewPageTableDescriptor(ptd1st, virtual, domain);

      if(0 != result)
      {
#ifdef PT_LARGE_DEBUG
        serial_putstring("Creation of new 1st level descriptor failed. At address : 0x");
        serial_putint((u32int)ptd1st);
        serial_putstring("For virtual address: 0x");
        serial_putint(virtual);
        serial_newline();
#endif
        return result;
      }
    case PAGE_TABLE:
      /* Then we have the correct type for a small/large pt descriptor */
      break;
    case SECTION:
    //error!
      serial_putstring("Page Table entry: ");
      serial_putint((u32int) ptd1st);
      serial_putstring("at address: 0x");
      serial_putint(*(u32int*)ptd1st);
      serial_putstring(", is a section/supersection.");
      serial_newline();
      serial_ERROR("Entering infinite loop.");
      return 1;
      break;
    case RESERVED:
    default:
      //error!
      serial_putstring("RESERVED type: 0x");
      serial_putint((u32int) ptd1st);
      serial_putstring("at address: 0x");
      serial_putint(*(u32int*)ptd1st);

      serial_ERROR("Entering infinite loop.");
      return 1;
      break;
  }//switch

  /* At this point we know its a small / large page table descriptor */

  /* Retrieve second level entry */
  descriptor* ptd2nd;
  ptd2nd = get2ndLevelPtDescriptor((pageTableDescriptor*)ptd1st, virtual);

  /* Again here we want to check the existing entry is no valid, could indicate problems elsewhere */

  if(FAULT == ptd2nd->type)
  {
#ifdef PT_LARGE_DEBUG
    serial_putstring("2nd Level FAULT. Creating new small descriptor");
    serial_newline();
#endif
    addNewLargeDescriptor((largeDescriptor*)ptd2nd, physical, accessBits, c, b, tex);
  }
  else //Existing small or large page
  {
#ifdef PT_LARGE_DEBUG
    serial_putstring("Error Entry for VirtualAddr: 0x");
    serial_putint(virtual);
    serial_putstring(", already exists");
    serial_newline();
#endif

    /* Dump some more debug */
    u32int physicalAddr = physical >> 12;
    u32int ptPhyscialAddr = *(u32int*)ptd2nd >> 12;
    if(physicalAddr == (physicalAddr & ptPhyscialAddr))
    {
      //match
#ifdef PT_LARGE_DEBUG
      serial_putstring(", attempt to re-add existing entry for PhysicalAddr: 0x");
      serial_putint(physical);
      serial_putstring("continuing.");
      serial_newline();
#endif
    }
    else
    {
      //Overwrite?
      serial_putstring(", attempt to overwrite existing entry for PhysicalAddr: 0x");
      serial_putint(physical);
      serial_newline();
      serial_ERROR("Entering infinte loop");
      return 1;
    }
  }
  return 0;
}

descriptor* get1stLevelPtDescriptorAddr(descriptor* ptd, u32int virtualAddress)
{
  /*
  ptd (page table base addr) aligned to 16KB boundary
  31             14        0
  |  Base Addr   |         |

  Virtual Addr
  31         20            0
  |  Index   |             |

  1stLevel descriptor addr
  31             14       2 1 0
  | Base Addr    | Index  |0|0|
  */
  u32int tableIndex = 0;
  tableIndex = (virtualAddress & 0xFFF00000) >> 18;
  u32int baseAddress = 0;
  baseAddress = ((u32int)ptd & 0xFFFFC000);
  descriptor* addr = (descriptor*)(tableIndex | baseAddress);
  return addr;
}

descriptor* get2ndLevelPtDescriptor(pageTableDescriptor* ptd1st, u32int virtual)
{
  /*
  ptd2nd aligned to a 1024 byte boundary
  31             10        0
  |  Base Addr   |         |

  Virtual Addr
  31   19      12          0
  |    | index  |          |

  1stLevel descriptor addr
  31             10       2 1 0
  | Base Addr    | Index  |0|0|
  */

  u32int baseAddr = *(u32int*)ptd1st & 0xFFFFFC00; // base addr bits 31:10 -> bits 31:10
  u32int index = (virtual & 0x000FF000) >> 10; //virt addr bits 19:12 -> bits 9:2
  u32int descrAddr = baseAddr | index;
  return (descriptor*)descrAddr;
}

/*
Allocates mem, zeros & writes the new entry to the 1st level pt
return 0 for success.
*/
u32int addNewPageTableDescriptor(descriptor* ptd1st, u32int physical, u8int domain)
{
  //alloc 1KB for the 2nd level descriptor (256 x 32bit entries)
  descriptor* ptBaseAddr;
  ptBaseAddr = (descriptor*)allocFrame(HYPERVISOR_FA_DOMAIN); //This gives us 4KB, fix later

  if(0 == (u32int)ptBaseAddr)
  {
    serial_ERROR("Memory allocation fail for the 2nd level descriptor table.");
    return 1;
  }

#ifdef PT_SHORT_DEBUG
  serial_putstring("Allocated memory for second level pt at addr: 0x");
  serial_putint((u32int)ptBaseAddr);
  serial_newline();
#endif

  //zero it
  memset(ptBaseAddr, 0, SECOND_LEVEL_PAGE_TABLE_SIZE);

  //create the new entry & write it
  u32int imp = 0;
  u8int sbz = 0;
  u8int ns = 0;
  u8int sbz2 = 0;
  u8int type = 1;

  u32int* entry = (u32int*)ptd1st;
  *entry = ((u32int)ptBaseAddr & 0xFFFFFC00) | imp << 9 | domain << 5 | sbz <<4 | ns <<3| sbz2 <<2 | type;

#ifdef PT_SHORT_DEBUG
  serial_putstring("Written new 1st level entry at 0x");
  serial_putint((u32int)ptd1st);
  serial_putstring(", value: 0x");
  serial_putint(*(u32int*)ptd1st);
  serial_newline();
#endif
  return 0;
}

void addNewSmallDescriptor(smallDescriptor* sd, u32int physical, u8int accessBits, u8int c, u8int b, u8int tex)
{
  sd->addr = (physical >> 12);
  sd->nG=0; //using global paging
  sd->s=0; //memory sharing / caching bits B3.7.2 of ARM ARM
  sd->ap2= (accessBits & 0x4) >> 2;
  sd->tex=(tex & 0x7);  //memory sharing / caching bits B3.7.2 of ARM ARM
  sd->ap10= accessBits & 0x3;  //full access
  sd->c=(c & 0x1);  //memory sharing / caching bits B3.7.2 of ARM ARM
  sd->b=(b & 0x1);  //memory sharing / caching bits B3.7.2 of ARM ARM
  sd->type= (SMALL_PAGE >> 1) & 0x1 ; //set to small page
  sd->xn=0; //execute of memory allowed
#ifdef PT_SMALL_DEBUG
  serial_putstring("Small descriptor written: 0x");
  serial_putint(*(u32int*)sd);
  serial_newline();
#endif
}

void addNewLargeDescriptor(largeDescriptor* ld, u32int physical, u8int accessBits, u8int c, u8int b, u8int tex)
{
  ld->addr = (physical >> 16);
  ld->nG=0; //using global paging
  ld->s=0; //memory sharing / caching bits B3.7.2 of ARM ARM
  ld->ap2= (accessBits & 0x4) >> 2;
  ld->tex=(tex & 0x7);  //memory sharing / caching bits B3.7.2 of ARM ARM
  ld->ap10= accessBits & 0x3;  //full access
  ld->c=(c & 0x1);  //memory sharing / caching bits B3.7.2 of ARM ARM
  ld->b=(b & 0x1);  //memory sharing / caching bits B3.7.2 of ARM ARM
  ld->type=LARGE_PAGE; //set to large page
  ld->xn=0; //execute of memory allowed
#ifdef PT_LARGE_DEBUG
  serial_putstring("Large descriptor written: 0x");
  serial_putint(*(u32int*)ld);
  serial_newline();
#endif

  //Need to copy the large entry 16 times
  replicateLargeEntry(ld);
}

void addNewSectionDescriptor(sectionDescriptor* sd, u32int physical, u8int domain, u8int accessBits, u8int c, u8int b, u8int tex)
{
  sd->addr= (physical >> 20);
  sd->type= 0b10; //set to section
  sd->b=(b & 0x1); //memory sharing / caching bits B3.7.2 of ARM ARM
  sd->c=(c & 0x1); //memory sharing / caching bits B3.7.2 of ARM ARM
  sd->xn= 0; //execute of memory allowed
  sd->domain= domain;
  sd->imp= 0; //currently unused
  sd->ap10= accessBits & 0x3;
  sd->tex=(tex & 0x7) ; //memory sharing / caching bits B3.7.2 of ARM ARM
  sd->ap2= (accessBits & 0x4) >> 2;
  sd->s= 0; //memory sharing / caching bits B3.7.2 of ARM ARM
  sd->nG= 0; //using global paging
  sd->sectionType= 0; //normal section (=1 for supersection)
  sd->ns= 1; //non secure memory?
#ifdef PT_SECTION_DEBUG
  serial_putstring("Section descriptor written: 0x");
  serial_putint(*(u32int*)sd);
  serial_newline();
#endif
}

/** Debug / entire page table dump to serial console */

void dumpPageTable(descriptor* ptd)
{
#ifdef PT_DUMP_DEBUG
  //assumes a 16KB pageTable mapped into memory
  serial_putstring("page table entries: 0x");
  serial_putint(PAGE_TABLE_ENTRIES);
  serial_newline();

  serial_putstring("VIRTUAL ADDR | TYPE | DETAILS");
  serial_newline();
  serial_putstring("-----------------------------");
  serial_newline();

  serial_putstring("ptd addr: 0x");
  serial_putint((u32int)ptd);
  serial_newline();

  descriptor* currentPtd = ptd;
  u32int i;

  for(i=0; i < PAGE_TABLE_ENTRIES; i++)
  {
    switch(currentPtd->type)
    {
      case FAULT: // 0b00
        break;
      case PAGE_TABLE: // 0b01
        dumpVirtAddr(i);
        serial_putstring("-0x");
        serial_putint(((i+1) << 20)-1);
        serial_putstring(" ");
        serial_putstring("Entry for 2nd level table: 0x");
        serial_putint( *((u32int*)currentPtd) );
        serial_newline();
        dump2ndLevel((pageTableDescriptor*)currentPtd, (i << 20));
        break;
      case SECTION: // 0b10
        dumpVirtAddr(i);
        serial_putstring(" ");
        dumpSection((sectionDescriptor*) currentPtd);
        break;
      case RESERVED: // 0b11
        dumpVirtAddr(i);
        serial_putstring(" RESERVED");
        serial_newline();
        break;
      default:
        serial_ERROR("dumpPageTables: INVALID pt entry type.");
    }//switch
    currentPtd++;
  }
#else
  serial_putstring("Enable page table debug: #define PT_DUMP_DEBUG, to dump page table contents");
  serial_newline();
#endif
}

#ifdef PT_DUMP_DEBUG
void dumpVirtAddr(u32int i)
{
  serial_putstring("0x");
  serial_putint(i << 20);
}

void dump2ndVirtAddr(u32int virtual, u32int i, u32int pageSize)
{
  //Indent by 4 characters
  serial_putstring("    0x");
  switch (pageSize)
  {
    case PAGE_64KB:
      virtual += i << 16; // 16 bits -> 64KB    
      break;
    case PAGE_4KB:
      virtual += i << 12; // 12 bits -> 4KB 
      break;
    case PAGE_1KB:
      virtual += i << 10; // 10 bits -> 1KB
      break;
    default:
      serial_ERROR("dump2ndVirtAddr: invalid page size.");
  } 
  serial_putint(virtual);
  serial_putstring(" ");
}

void dump2ndLevel(pageTableDescriptor* ptd, u32int virtual)
{
  serial_putstring("    SUB PAGE TABLE");
  serial_putstring(" domain ");
  serial_putint_nozeros(ptd->domain);

  u32int baseAddr = ptd->addr << 10;
  serial_putstring(" baseAddr ");
  serial_putint_nozeros(baseAddr);
  serial_newline();

  if ( (baseAddr > BEAGLE_RAM_START) &&
       (baseAddr <=BEAGLE_RAM_END-SECOND_LEVEL_PAGE_TABLE_SIZE) )
  {
    serial_putstring("    Virtual Addr | TYPE | DETAILS");
    serial_newline();
    serial_putstring("    -----------------------------");
    serial_newline();

    descriptor* currentPtd = (descriptor*) (ptd->addr << 10);
    int i = 0;
    for(i = 0; i < SECOND_LEVEL_PAGE_TABLE_ENTRIES; i++)
    {
      switch(currentPtd->type)
      {
        case FAULT:        // 0b00: translation fault (no valid entry)
          // no entry here
          break;
        case LARGE_PAGE:   // 0b01: large page (64KB)
          dump2ndVirtAddr(virtual, i, PAGE_64KB);
          dumpLargePage((largeDescriptor*)currentPtd);
          break;
        case SMALL_PAGE:   // 0b10: small page (4KB)
          dump2ndVirtAddr(virtual, i, PAGE_4KB);
          dumpSmallPage((smallDescriptor*)currentPtd);
          break;
        case SMALL_PAGE_3: // 0b11: tiny page  (1KB)
          serial_ERROR("dump2ndLevel: can't handle small_page 1KB yet.");
        default:
          serial_ERROR("dump2ndLevel: INVALID pt entry type.");
      }
      currentPtd++;
    }
  }
  else
  {
    serial_putstring("Address of the second level page table is not in physical RAM: 0x");
    serial_putint(baseAddr);
    serial_newline();
  }
}

void dumpSmallPage(smallDescriptor* sd)
{
  serial_putstring("SMALL PAGE");

  serial_putstring(" PhysAddr: 0x");
  serial_putint(sd->addr << 12);
  /*
  serial_putstring(" nG:");
  serial_putint_nozeros(sd->nG);
  serial_putstring(" s:");
  serial_putint_nozeros(sd->s);
  serial_putstring(" tex:");
  serial_putint_nozeros(sd->tex);
  serial_putstring(" xn:")
  serial_putint_nozeros(sd->xn);
  */
  serial_putstring(" c:");
  serial_putint_nozeros(sd->c);
  serial_putstring(" b:");
  serial_putint_nozeros(sd->b);
  serial_putstring(" accessBits:");
  u32int accessBits = (sd->ap2 << 2) | sd->ap10;
  serial_putint_nozeros(accessBits);
  serial_newline();
}

void dumpLargePage(largeDescriptor* ld)
{
  serial_putstring("LARGE PAGE");
  serial_putstring(" PhysAddr: 0x");
  serial_putint(ld->addr << 16);
  /*
  serial_putstring(" nG:");
  serial_putint_nozeros(ld->nG);
  serial_putstring(" s:");
  serial_putint_nozeros(ld->s);
  serial_putstring(" tex:");
  serial_putint_nozeros(ld->tex);
  serial_putstring(" c:");
  serial_putint_nozeros(ld->c);
  serial_putstring(" b:")
  serial_putint_nozeros(ld->b);
  serial_putstring(" xn:")
  serial_putint_nozeros(ld->xn);
  */
  serial_putstring(" accessBits:");
  u32int accessBits = (ld->ap2 << 2) | ld->ap10;
  serial_putint_nozeros(accessBits);
  serial_newline();
}

void dumpSection(sectionDescriptor* sd)
{
  if(0 != sd->sectionType)
  {
    dumpSuperSection(sd);
    return;
  }
  serial_putstring("SECTION ");
  serial_putstring(" PhysAddr: 0x");
  serial_putint(sd->addr << 20);
  serial_putstring(" domain:");
  serial_putint_nozeros(sd->domain);
  serial_putstring(" accessBits:");
  u32int accessBits = (sd->ap2) << 2 | sd->ap10;
  serial_putint_nozeros(accessBits);
  serial_putstring(" nG:");
  serial_putint_nozeros(sd->nG);
  /*
  serial_putstring(" ns:");
  serial_putint(sd->ns);
  serial_putstring(" s:");
  serial_putint(sd->s);
  serial_putstring(" tex:");
  serial_putint(sd->tex);
  serial_putstring(" imp:");
  serial_putint(sd->imp);
  serial_putstring(" xn:");
  serial_putint(sd->xn);
  serial_putstring(" c:");
  serial_putint(sd->c);
  serial_putstring(" b:");
  serial_putint(sd->b);
  */
  serial_newline();
}

void dumpSuperSection(void* sd)
{
  serial_putstring("SUPERSECTION");
  serial_newline();

  serial_ERROR("UNIMPLEMENTED: dumpSuperSection (pageTable.c)");
}
#endif


/** Change the accessBits of a descriptor given valid accessBits & a descriptor

returns previous descriptor (value 0-7)
value > 7 is an error
*/
ACCESS_TYPE setAccessBits(descriptor* ptd, u32int virtual, ACCESS_TYPE newAccessBits)
{
  if(0 == ptd)
  {
    ptd = hypervisorPtd;
  }

  u8int currentAccessBits = 0; //We return the current access bits for the page, so they can be restored if needed

  descriptor* pte = get1stLevelPtDescriptorAddr(ptd, virtual);

#ifdef PT_DEBUG
  serial_putstring("setAccessBits: ptd=");
  serial_putint((u32int)ptd);
  serial_putstring(" virtual=");
  serial_putint(virtual);
  serial_putstring(" newAccessBits=");
  serial_putint_nozeros((u32int)newAccessBits);
  serial_newline();
  serial_putstring("setAccessBits: PTentry=");
  serial_putint((u32int)pte);
  serial_putstring(" value=");
  serial_putint(*(u32int*)pte);
  serial_newline();
#endif
  
  switch(pte->type)
  {
    case SECTION:
      ; //GCC label bug
      sectionDescriptor* sd = (sectionDescriptor*)pte;
      if(sd->sectionType != 0)
      {
        //error supersection
        serial_ERROR("setting accessBits for a SuperSection is not implemented. Entering infinite loop...");
      }
      else
      {
        currentAccessBits = (sd->ap2 << 2) | sd->ap10;
#ifdef PT_DEBUG
        serial_putstring("setAccessBits: Setting accessBits for SECTION page pte: 0x");
        serial_putint((u32int)pte);
        serial_putstring(" old bits: ");
        serial_putint_nozeros(currentAccessBits);
        serial_putstring(", new bits: ");
        serial_putint_nozeros(newAccessBits);
        serial_newline();
#endif
        sd->ap2 = newAccessBits >> 2;
        sd->ap10 = newAccessBits & 0x3;
      }
      break;
    case PAGE_TABLE:
      ;//GCC label bug
      descriptor* pte2nd = get2ndLevelPtDescriptor((pageTableDescriptor*)pte, virtual);
      switch(pte2nd->type)
      {
        case SMALL_PAGE://fall through
        case SMALL_PAGE_3:
          ;//GCC label bug
          smallDescriptor* sd = (smallDescriptor*)pte2nd; //large & small pages have their access bits in the same place
          currentAccessBits = (sd->ap2 << 2) | sd->ap10;
#ifdef PT_SMALL_DEBUG
          serial_putstring("Setting accessBits for SMALL page pte: 0x");
          serial_putint((u32int)pte);
          serial_putstring(" old bits: ");
          serial_putint_nozeros(currentAccessBits);
          serial_putstring(", new bits: ");
          serial_putint_nozeros(newAccessBits);
          serial_newline();
#endif
          sd->ap2 = newAccessBits >> 2;
          sd->ap10 = newAccessBits & 3;
          break;
        case LARGE_PAGE:
          ;//GCC label bug
          largeDescriptor* lpd = (largeDescriptor*)(u32int)pte2nd;
          //retrieve the current accessBits for return
          currentAccessBits = (lpd->ap2 << 2) | lpd->ap10;
#ifdef PT_LARGE_DEBUG
          serial_putstring("Setting accessBits for (16) LARGE page pte: 0x");
          serial_putint((u32int)lpd);
          serial_putstring("-0x");
          serial_putint((u32int)&lpd[15]);
          serial_putstring(" old bits: ");
          serial_putint_nozeros(currentAccessBits);
          serial_putstring(", new bits: ");
          serial_putint_nozeros(newAccessBits);
          serial_newline();
#endif
          //set the new accessBits
          lpd->ap2 = newAccessBits >> 2;
          lpd->ap10 = newAccessBits & 3;

          //copy it to the other 15 entries
          replicateLargeEntry(lpd);
          break;
        case FAULT: //fall through
        default:
          serial_ERROR("Error: Attempt to set accessBits for page type: FAULT, in 2nd level page table");
          return 8;
          break;
      }//switch PAGE_TABLE
      break;
    case FAULT:
      serial_ERROR("Error: Attempt to set accessBits for page type: FAULT!");
      return 8;
      break;
      //else fall through
    case RESERVED:
      serial_ERROR("Error: Attempt to set accessBits for page type: RESERVED!");
      //fall through
    default:
      return 8; //error
    break;
  }

  //toggle imp defined bit to signify memProtection active / inactive
  pte->imp = pte->imp ^ 0x1;
  clearTLB();
  return (ACCESS_TYPE)currentAccessBits;
}


void replicateLargeEntry(largeDescriptor* ld)
{
  //looks nasty but otherwise gcc insists on using memcpy, yuck!
  u32int* toCopy = (u32int*)ld;

  //get address of first large page entry
  volatile u32int* descriptor = (u32int*) ((u32int)ld & 0xFFFFFFC0);

  u32int i;
  for(i=0; i < 16; i++)
  {
    descriptor[i] = *toCopy;
  }
}


descriptor* getPageTableEntry(descriptor* ptd, u32int address)
{
  if(0 == ptd)
  {
    serial_ERROR("ptd is 0; getPageTableEntry (pageTable.c)");
  }

  descriptor* pte1st = get1stLevelPtDescriptorAddr(ptd, address);
  switch(pte1st->type)
  {
    case SECTION:
    {
      if( ((sectionDescriptor*)pte1st)->sectionType == 1)
      {
        serial_ERROR("SUPERSECTION not implemented");
        return 0;
      }
      else
      {
        return pte1st;
      }
      break;
    }
    case PAGE_TABLE:
    {
      descriptor* pte2nd = get2ndLevelPtDescriptor((pageTableDescriptor*)pte1st, address);
      switch(pte2nd->type)
      {
        case LARGE_PAGE:
          return pte2nd;
          break;
        case SMALL_PAGE: //fall through
        case SMALL_PAGE_3:
          return pte2nd;
          break;
        case FAULT:
        default:
          return pte2nd;
          break;
      }
      break;
    }
    case FAULT: //fall through
    case RESERVED: //fall through
    default:
      return pte1st;
      break;
  }
}

void copySectionEntry(sectionDescriptor* guest, sectionDescriptor* shadow)
{
  GCONTXT* gc = (GCONTXT*)getGuestContext();
  descriptor* guestPhysical = gc->PT_physical;

  if(guest->sectionType == 1)
  {
    // this is a 16Mb super section
    copySuperSectionEntry(guest, shadow);
  }
  else
  {
#ifdef PT_SHADOW_DEBUG
    serial_putstring("Guest entry to copy: ");
    serial_newline();
    dumpSection(guest);
#endif

    /* Address mapping */
    u32int guestPhysicalAddr = guest->addr << 20;

    //Check physical address is within RAM address range
    if( (guestPhysicalAddr >= BEAGLE_RAM_START) &&
        (guestPhysicalAddr < BEAGLE_RAM_END -SECTION_SIZE +1) )
    {

      sectionDescriptor* guestReal = (sectionDescriptor*)get1stLevelPtDescriptorAddr(guestPhysical, guestPhysicalAddr);

      if(guestReal->type != SECTION)
      {
      /*
        Hitting this means we allocated guestReal memory in small/large pages
        or converted it the guestPhysical PT to smaller entries for purposes of memory protection perf
        Need to check if any protection is still in place, if not convert back to section map?

        This should be performed by the removeProtection code?
        And therefore make this just a check for contiguous memoyr
      */
#ifdef PT_SHADOW_DEBUG
        serial_putstring("PARTIAL_IMPLEMENTATION: guestPhysical contiguous memory check copySectionEntry (pageTable.c). continuing");
        serial_newline();
#endif

        /* Hack for now, really should check that the addr space is contiguous
        & remap the real physical to a section
        Also a hack in the fact we only want the top 12bits of the address,
        this casting is a bit wierd as a result of the hack
        */
        guestReal = (sectionDescriptor*)get2ndLevelPtDescriptor((pageTableDescriptor*)guestReal, guestPhysicalAddr);

        if(FAULT == guestReal->type)
        {
          serial_ERROR("Underlying address space is not mapped. copySectionEntry (pageTable.c) Entering infinite loop");
        }
      }
#ifdef PT_SHADOW_DEBUG
      u32int guestRealAddr = guestReal->addr << 20;
      serial_putstring("guest real addr: 0x");
      serial_putint(guestRealAddr);
      serial_newline();
#endif

      shadow->type = SECTION;
      shadow->addr = guestReal->addr;

      /* Access control bit mapping */
#ifdef PT_SHADOW_DEBUG
      serial_putstring("Setting shadow Access control bits: ");
#endif
      if(guest->ap2 == 1)
      {
#ifdef PT_SHADOW_DEBUG
        serial_putstring("USR read only.");
        serial_newline();
#endif
        shadow->ap2 = PRIV_RW_USR_RO >> 2;
        shadow->ap10 = PRIV_RW_USR_RO & 0x3;
      }
      else
      {
        if(guest->ap10 == 0x0)
        {
#ifdef PT_SHADOW_DEBUG
          serial_putstring("USR No access");
          serial_newline();
#endif
          shadow->ap2 = PRIV_RW_USR_NO >> 2;
          shadow->ap10 = PRIV_RW_USR_NO & 0x3;
        }
        else
        {
#ifdef PT_SHADOW_DEBUG
          serial_putstring("USR R/W access");
          serial_newline();
#endif
          shadow->ap2 = PRIV_RW_USR_RW >> 2;
          shadow->ap10 = PRIV_RW_USR_RW & 0x3;
        }
      }

      /* Check/map guest domain for access control */
      shadow->domain = mapGuestDomain(guest->domain);

      /* WARNING:Position dependant. Must be after the access control being set above! */
      //Assume this is correct, helps us with memory protection
      u8int xn = shadow->xn = guest->xn;
      if(xn == 1)
      {
        // guest maps memory as EXECUTE NEVER.
        serial_putstring("XN bit is set.  Protecting code space from self-modification block cache corruption.");
        serial_newline();
        // When memory protection is properly implemented
        // then we should point to a function that deals with this!
        //Perhaps an optimised addProtection method for the pageTable class here
        //using an extern! -> just need to add the single protection entry to the LL!
        addProtection((u32int)shadow, (u32int)shadow, 0, PRIV_RW_USR_RO);
      }
      /* End WARNING */

      //Currently just map these, may need to correct this with something proper later
      shadow->c = 0;
      shadow->b = 0;
      shadow->s = 0;
      shadow->tex = 0;
      shadow->nG = guest->nG;
      shadow->ns = guest->ns;

      if(1 == guest->imp)
      {
        // I have an idea to use the imp bit for quick memory protection in use checking,
        // without having to traverse the memory protection list/sub system
        // Unless memProtection is implemented don't worry about this at all
        serial_putstring("WARNING:guest OS PT using the imp bit.");
        serial_newline();
        serial_putstring("Please tell Virt Mem maintainer about this copySectionEntry (pageTable.c)");
        serial_newline();
      }
    }
    else if( 0x00000000 == (guestPhysicalAddr & 0xFFF0000))
    {
      //First time we access this we will fault
/*
      serial_putstring("Guest map virt: 0x");
      serial_putint(((u32int)shadow) << 18);
      serial_putstring(" to phy: 0x");
      serial_putint(guestPhysicalAddr);
      serial_putstring(". Low address handler space. Marking no access");
      serial_newline();
*/
      shadow->type = SECTION;
      shadow->addr = guest->addr;
      shadow->ap10 = PRIV_RW_USR_NO & 0x3;
      shadow->ap2 = PRIV_RW_USR_NO >> 2;
      shadow->domain = mapGuestDomain(guest->domain);

      //Leave the rest zero for now
    }
    else if( 0x40200000 == (guestPhysicalAddr & 0xFFF00000))
    {
      //First time we access this we will fault
/*
      serial_putstring("Guest map virt: 0x");
      serial_putint(((u32int)shadow) << 18);
      serial_putstring(" to phy: 0x");
      serial_putint(guestPhysicalAddr);
      serial_putstring(". Internal SRAM. Marking no access");
      serial_newline();
*/
      shadow->type = SECTION;
      shadow->addr = guest->addr;
      shadow->ap10 = PRIV_RW_USR_NO & 0x3;
      shadow->ap2 = PRIV_RW_USR_NO >> 2;
      shadow->domain = mapGuestDomain(guest->domain);

      //Leave the rest zero for now
    }
    else if(0x49000000 == (guestPhysicalAddr & 0xFFF00000))
    {
/*
      serial_putstring("Guest mapping Serial address space. Already mapped by Hypervisor.");
      serial_newline();
      serial_putstring("Guest map virt: 0x");
      serial_putint(((u32int)shadow) << 18);
      serial_putstring(" to phy: 0x");
      serial_putint(guestPhysicalAddr);
      serial_putstring(". Serial lives here. Marking no access");
      serial_newline();
*/
      shadow->type = SECTION;
      shadow->addr = guest->addr;
      shadow->ap10 = PRIV_RW_USR_NO & 0x3;
      shadow->ap2 = PRIV_RW_USR_NO >> 2;
      shadow->domain = mapGuestDomain(guest->domain);
      //Leave the rest zero for now
    }
    else if( 0x48000000 == (guestPhysicalAddr & 0xFF000000))
    {
      //First time we access this we will fault
      u32int virtAddr = (u32int)shadow << 18;
/*
      serial_putstring("Guest map virt: 0x");
      serial_putint(virtAddr);
      serial_putstring(" to phy: 0x");
      serial_putint(guestPhysicalAddr);
      serial_putstring(". L4 Interconnect space. Mapping no access");
      serial_newline();
*/
      /* HACK: Map in small pages for granularity

      A proper memory protection solution would do the PT reconfiguration for use
      When the addProtection is called.

      For now map in in 4KB chunks
      */
      u32int startAddr = 0x48300000;

      if(guestPhysicalAddr == startAddr)
      {
        u32int endAddr = 0x483FFFFF;
        descriptor* shadowPTD = (descriptor*)((u32int)shadow & 0xFFFFC000);
        while (startAddr < endAddr)
        {
          u32int result = addSmallPtEntry(shadowPTD, virtAddr, startAddr, mapGuestDomain(guest->domain), PRIV_RW_USR_NO, 0, 0, 0b000);
          if(result)
          {
            serial_ERROR("Hack failed: copySectionEntry (pageTable.c)");
            break;
          }
          startAddr += SMALL_PAGE_SIZE;
          virtAddr += SMALL_PAGE_SIZE;
        }

        setAccessBits(shadowPTD, 0xd830A000, PRIV_RW_USR_RO);
        /* End HACK */
      }
      else
      {
        shadow->type = SECTION;
        shadow->addr = guest->addr;
        shadow->ap10 = PRIV_RW_USR_NO & 0x3;
        shadow->ap2 = PRIV_RW_USR_NO >> 2;
        shadow->domain = mapGuestDomain(guest->domain);
      }

      //Leave the rest zero for now
    }
    else if(0x54000000 == (guestPhysicalAddr & 0xFF000000 ))
    {
      //First time we access this we will fault
/*
      serial_putstring("Guest map virt: 0x");
      serial_putint(((u32int)shadow) << 18);
      serial_putstring(" to phy: 0x");
      serial_putint(guestPhysicalAddr);
      serial_putstring(". L4-emu Interconnect space. Marking no access");
      serial_newline();
*/
      shadow->type = SECTION;
      shadow->addr = guest->addr;
      shadow->ap10 = PRIV_RW_USR_NO & 0x3;
      shadow->ap2 = PRIV_RW_USR_NO >> 2;
      shadow->domain = mapGuestDomain(guest->domain);

      //Leave the rest zero for now
    }
    else if(0x55000000 == (guestPhysicalAddr & 0xFF000000 ))
    {
      //First time we access this we will fault
/*
      serial_putstring("Guest map virt: 0x");
      serial_putint(((u32int)shadow) << 18);
      serial_putstring(" to phy: 0x");
      serial_putint(guestPhysicalAddr);
      serial_putstring(". ??? space. Marking no access");
      serial_newline();
*/
      shadow->type = SECTION;
      shadow->addr = guest->addr;
      shadow->ap10 = PRIV_RW_USR_NO & 0x3;
      shadow->ap2 = PRIV_RW_USR_NO >> 2;
      shadow->domain = mapGuestDomain(guest->domain);

      //Leave the rest zero for now
    }
    else if(0x56000000 == (guestPhysicalAddr & 0xFF000000 ))
    {
      //First time we access this we will fault
/*
      serial_putstring("Guest map virt: 0x");
      serial_putint(((u32int)shadow) << 18);
      serial_putstring(" to phy: 0x");
      serial_putint(guestPhysicalAddr);
      serial_putstring(". ??? space. Marking no access");
      serial_newline();
*/
      shadow->type = SECTION;
      shadow->addr = guest->addr;
      shadow->ap10 = PRIV_RW_USR_NO & 0x3;
      shadow->ap2 = PRIV_RW_USR_NO >> 2;
      shadow->domain = mapGuestDomain(guest->domain);

      //Leave the rest zero for now
    }
    else if(0x57000000 == (guestPhysicalAddr & 0xFF000000 ))
    {
      //First time we access this we will fault
/*
      serial_putstring("Guest map virt: 0x");
      serial_putint(((u32int)shadow) << 18);
      serial_putstring(" to phy: 0x");
      serial_putint(guestPhysicalAddr);
      serial_putstring(". ??? space. Marking no access");
      serial_newline();
*/
      shadow->type = SECTION;
      shadow->addr = guest->addr;
      shadow->ap10 = PRIV_RW_USR_NO & 0x3;
      shadow->ap2 = PRIV_RW_USR_NO >> 2;
      shadow->domain = mapGuestDomain(guest->domain);

      //Leave the rest zero for now
    }
    else if(0x68000000 == (guestPhysicalAddr & 0xFF000000 ))
    {
      //First time we access this we will fault
/*
      serial_putstring("Guest map virt: 0x");
      serial_putint(((u32int)shadow) << 18);
      serial_putstring(" to phy: 0x");
      serial_putint(guestPhysicalAddr);
      serial_putstring(". L3 Interconnect space. Marking no access");
      serial_newline();
*/
      shadow->type = SECTION;
      shadow->addr = guest->addr;
      shadow->ap10 = PRIV_RW_USR_NO & 0x3;
      shadow->ap2 = PRIV_RW_USR_NO >> 2;
      shadow->domain = mapGuestDomain(guest->domain);

      //Leave the rest zero for now
    }
    else if(0x6C000000 == (guestPhysicalAddr & 0xFF000000 ))
    {
      //First time we access this we will fault
/*
      serial_putstring("Guest map virt: 0x");
      serial_putint(((u32int)shadow) << 18);
      serial_putstring(" to phy: 0x");
      serial_putint(guestPhysicalAddr);
      serial_putstring(". SMS register space. Marking no access");
      serial_newline();
*/
      shadow->type = SECTION;
      shadow->addr = guest->addr;
      shadow->ap10 = PRIV_RW_USR_NO & 0x3;
      shadow->ap2 = PRIV_RW_USR_NO >> 2;
      shadow->domain = mapGuestDomain(guest->domain);

      //Leave the rest zero for now
    }
    else if(0x6D000000 == (guestPhysicalAddr & 0xFF000000 ))
    {
      //First time we access this we will fault
/*
      serial_putstring("Guest map virt: 0x");
      serial_putint(((u32int)shadow) << 18);
      serial_putstring(" to phy: 0x");
      serial_putint(guestPhysicalAddr);
      serial_putstring(". SDRC space. Marking no access");
      serial_newline();
*/
      shadow->type = SECTION;
      shadow->addr = guest->addr;
      shadow->ap10 = PRIV_RW_USR_NO & 0x3;
      shadow->ap2 = PRIV_RW_USR_NO >> 2;
      shadow->domain = mapGuestDomain(guest->domain);

      //Leave the rest zero for now
    }
    else if(0x6E000000 == (guestPhysicalAddr & 0xFF000000 ))
    {
      //First time we access this we will fault
/*
      serial_putstring("Guest map virt: 0x");
      serial_putint(((u32int)shadow) << 18);
      serial_putstring(" to phy: 0x");
      serial_putint(guestPhysicalAddr);
      serial_putstring(". GMPC registers. Marking no access");
      serial_newline();
*/
      shadow->type = SECTION;
      shadow->addr = guest->addr;
      shadow->ap10 = PRIV_RW_USR_NO & 0x3;
      shadow->ap2 = PRIV_RW_USR_NO >> 2;
      shadow->domain = mapGuestDomain(guest->domain);

      //Leave the rest zero for now
    }
    else
    {
      serial_putstring("Write handler for guestPhysical mapping to non RAM range.");
      serial_newline();
      serial_putstring("copySectionEntry (pageTable.c)");
      serial_newline();
      serial_putstring("Guest Physical Address: 0x");
      serial_putint(guestPhysicalAddr);
      serial_newline();
      serial_ERROR("Entering infinite loop...");
    }
  }

#ifdef PT_SHADOW_DEBUG
  serial_putstring("Shadow entry after copy: ");
  serial_newline();
  dumpSection(shadow);
#endif

}

void copySuperSectionEntry(sectionDescriptor* guest, sectionDescriptor* shadow)
{
  serial_ERROR("UNIMPLEMENTED: copySuperSectionEntry (pageTable.c)");
}

void copyPageTableEntry(pageTableDescriptor* guest, pageTableDescriptor* shadow)
{
  GCONTXT* gc = getGuestContext();

  u32int *newFrame = allocFrame(HYPERVISOR_FA_DOMAIN);
  if (newFrame == 0x0)
  {
    serial_ERROR("frameAllocator returned null ptr. copyPageTableEntry (pageTable.c)");
  }

#ifdef PT_SHADOW_DEBUG
  serial_putstring("copyPageTableEntry: guest ");
  serial_putint((*(u32int*)guest));
  serial_putstring(" shadow ");
  serial_putint((*(u32int*)shadow));
  serial_putstring(" newFrameAddr ");
  serial_putint((u32int)newFrame);
  serial_newline();
#endif

  memset(newFrame, 0, SECOND_LEVEL_PAGE_TABLE_SIZE);

  shadow->addr = (u32int)newFrame >> 10;

  //This is just a copy of the high level descriptor
  shadow->type = PAGE_TABLE;
  shadow->domain = mapGuestDomain(guest->domain);
  shadow->ns = guest->ns;

  //If the guest addr ptr to the 2nd level pt is valid, then copy it
  u32int phyAddr = getPhysicalAddress(gc->PT_physical, (guest->addr << 10));
  
  u32int metaArrayIndex = 0;
  while (guestSecondLvlPageTables[metaArrayIndex] != 0)
  {
    metaArrayIndex++;
  }
  guestSecondLvlPageTables[metaArrayIndex] = phyAddr;
  
#ifdef PT_SHADOW_DEBUG
  serial_putstring("copyPageTableEntry: guest 2nd level PT physAddr ");
  serial_putint(phyAddr);
  serial_newline();
#endif
  
  if( (phyAddr >= BEAGLE_RAM_START) && (phyAddr < BEAGLE_RAM_END) )
  {

    // this addr is physical. if we want to do operations on it, we need a 1-2-1 mapping
    u32int guestPageTableAddr = guest->addr << 10;
    
    // calculate address in the shadow page table of the entry
    // where we must add 1-2-1 mapping
    u32int entryAddress = guestPageTableAddr >> 20;
    entryAddress = entryAddress * 4 + (u32int)gc->PT_shadow;
    // save old entry from that address whatever it is, and make a new entry
    u32int oldEntry = *((u32int*)entryAddress);

    u32int newEntry = ((guest->addr << 10) & 0xFFF00000) | 0x805E2;

    *(u32int*)entryAddress = newEntry;

    descriptor* guestPte  = (descriptor*)((guest->addr << 10));
    descriptor* shadowPte  = (descriptor*)newFrame;
    u32int i = 0;

    for(i=0; i < SECOND_LEVEL_PAGE_TABLE_ENTRIES; i++)
    {
      if(guestPte[i].type != FAULT)
      {
        if(guestPte[i].type == LARGE_PAGE)
        {
          copyLargeEntry((largeDescriptor*)&guestPte[i], (largeDescriptor*)&shadowPte[i]);
        }
        else
        {
          copySmallEntry((smallDescriptor*)&guestPte[i], (smallDescriptor*)&shadowPte[i]);
        }
      }
    }
    *(u32int*)entryAddress = oldEntry;
    
    // done copying table. Now find all virtual mappings to this physical addr
    // and add protection.
    // TODO: this only finds first VA mapping ot PA. this can be many-2-one, so find all..
    u32int virtAddr = findVAforPA(phyAddr);

    u32int res = addProtection(virtAddr, virtAddr+1023, 0, PRIV_RW_USR_RO);
    if (res > 7)
    {
      serial_ERROR("copyPageTableEntry: failed to add memory protection.");
    }

  }
}

void copyLargeEntry(largeDescriptor* guest, largeDescriptor* shadow)
{
#ifdef PT_SHADOW_DEBUG
  serial_putstring("copyLargeEntry(guest=");
  serial_putint(*(u32int*)guest);
  serial_putstring(", shadow=");
  serial_putint(*(u32int*)shadow);
  serial_newline();
#endif
  serial_ERROR("UNIMPLEMENTED: copyLargeEntry (pageTable.c)");
}

void copySmallEntry(smallDescriptor* guest, smallDescriptor* shadow)
{
  GCONTXT* gc = (GCONTXT*)getGuestContext();

#ifdef PT_SHADOW_DEBUG
  serial_putstring("copySmallEntry: guestEntry:");
  serial_putint(*(u32int*)guest);
  serial_putstring("; corresponding shadow entry @ ");
  serial_putint((u32int)shadow);
  serial_putstring(" = ");
  serial_putint(*(u32int*)shadow);
  serial_newline();
#endif

  // Address mapping: guest entry points to guest physical. Translate to host physical
  u32int guestPA = guest->addr << 12;

  //Check physical address is within RAM address range
  if ( (guestPA >= BEAGLE_RAM_START) && (guestPA < (BEAGLE_RAM_END - SECTION_SIZE + 1)) )
  {
    descriptor* hostPAentry = (descriptor*)get1stLevelPtDescriptorAddr(gc->PT_physical, guestPA);

    u32int hostPA = 0;
    switch (hostPAentry->type)
    {
      case SECTION:
      {
        sectionDescriptor* hostPAsection = (sectionDescriptor*)hostPAentry;
        hostPA = ((hostPAsection->addr << 20) & 0xFFF00000) | (guestPA & 0x000FFFFF);
#ifdef PT_SHADOW_DEBUG
        serial_putstring("copySmallEntry: guestPA 0x");
        serial_putint(guestPA);
        serial_putstring(" maps to hostPA 0x");
        serial_putint(hostPA);
        serial_newline();
#endif
        break;
      }
      case PAGE_TABLE:
      {
        descriptor* hostPAsecond =  
          get2ndLevelPtDescriptor((pageTableDescriptor*)hostPAentry, guestPA);
        switch (hostPAsecond->type)
        {
          case SMALL_PAGE:
          {
            smallDescriptor* hostPAsmall = (smallDescriptor*)hostPAsecond;
            hostPA = (hostPAsmall->addr << 12) & 0xFFFFF000;
            break;
          }
          case SMALL_PAGE_3:
          case LARGE_PAGE:
          case FAULT:
          default:
            serial_ERROR("copySmallEntry: hostPA second level entry unimplemented.");
        }
        break;
      }
      case FAULT:
      case RESERVED:
      default:
        serial_putstring("copySmallEntry: gc->PT_physical = 0x");
        serial_putint((u32int)gc->PT_physical);
        serial_putstring(" hostPAentry @ 0x");
        serial_putint((u32int)hostPAentry);
        serial_putstring(" is ");
        serial_putint(*(u32int*)hostPAentry);
        serial_newline();
        serial_ERROR("copySmallEntry: invalid entry found translating guestPA to hostPA.");
    }


    shadow->type = 1; // small page entry.
    shadow->addr = (hostPA >> 12) & 0xFFFFF;

    /* Access control bit mapping */
#ifdef PT_SHADOW_DEBUG
    serial_putstring("Setting shadow Access control bits: ");
#endif

    u32int accessProt = (guest->ap2) << 2 | guest->ap10;

    switch (accessProt)
    {
      case PRIV_NO_USR_NO:
        shadow->ap2 = PRIV_RW_USR_NO >> 2;
        shadow->ap10 = PRIV_RW_USR_NO & 0x3;
        break;
      case PRIV_RW_USR_NO:
      case PRIV_RW_USR_RO:
        shadow->ap2 = PRIV_RW_USR_RW >> 2;
        shadow->ap10 = PRIV_RW_USR_RW & 0x3;
        break;
      case PRIV_RO_USR_NO:
      case PRIV_RO_USR_RO:
      case DEPRECATED:
        shadow->ap2 = PRIV_RW_USR_RO >> 2;
        shadow->ap10 = PRIV_RW_USR_RO & 0x3;
        break;
      case PRIV_RW_USR_RW:
        shadow->ap2 = PRIV_RW_USR_RW >> 2;
        shadow->ap10 = PRIV_RW_USR_RW & 0x3;
        break;
      case AP_RESERVED:
        serial_ERROR("copySmallEntry: access protection reserved case hit.");
        break;
    }


    /* WARNING:Position dependant. Must be after the access control being set above! */
    //Assume this is correct, helps us with memory protection
    u8int xn = shadow->xn = guest->xn;
    if(xn == 1)
    {
      // guest maps memory as EXECUTE NEVER.
      serial_putstring("XN bit is set.  Protecting code space from self-modification block cache corruption.");
      serial_newline();
      // When memory protection is properly implemented
      // then we should point to a function that deals with this!
      //Perhaps an optimised addProtection method for the pageTable class here
      //using an extern! -> just need to add the single protection entry to the LL!
      addProtection((u32int)shadow, (u32int)shadow, 0, PRIV_RW_USR_RO);
    }
    /* End WARNING */

    //Currently just map these, may need to correct this with something proper later
    shadow->c = 0;
    shadow->b = 0;
    shadow->s = 0;
    shadow->tex = 0;
    shadow->nG = guest->nG;
  }
  else
  {
    // mapping some peripheral in small pages?
    serial_putstring("copySmallEntry: guest *(");
    serial_putint((u32int)guest);
    serial_putstring(")=");
    serial_putint(*(u32int*)guest);
    serial_putstring("; shadow *(");
    serial_putint((u32int)shadow);
    serial_putstring(")=");
    serial_putint(*(u32int*)shadow);
    serial_newline();
    serial_putstring("copySmallEntry: guestPA = ");
    serial_putint(guestPA);
    serial_newline();
    serial_ERROR("copySmallEntry: mapping non-ram physical address, unimplemented.");
  }
}

void removePageTableEntry(pageTableDescriptor* shadow)
{
  serial_ERROR("UNIMPLEMENTED: removePageTableEntry (pageTable.c)");
}


void splitSectionToSmallPages(descriptor* ptd, u32int vAddr)
{
#ifdef PT_DEBUG
  serial_putstring("splitSectionToSmallPages: 1st level PT @ ");
  serial_putint((u32int)ptd);
  serial_newline();
#endif
  // 1. get section entry
  sectionDescriptor* sectionEntryPtr = (sectionDescriptor*)get1stLevelPtDescriptorAddr(ptd, vAddr);
  // 2. invalidate entry. (in the future - implement and call removePTEntry method)
  sectionEntryPtr->type = 0;
  // 3. map memory in small pages
  vAddr = vAddr & 0xFFF00000;
  u32int pAddr = sectionEntryPtr->addr;
  pAddr = pAddr << 20;
  
#ifdef PT_DEBUG
  serial_putstring("splitSectionToSmallPages: vaddr ");
  serial_putint(vAddr);
  serial_putstring(", pAddr ");
  serial_putint(pAddr);
  serial_newline();
#endif

  u32int protectionBits = sectionEntryPtr->ap10;
  u32int domainBits = sectionEntryPtr->domain;
  u32int index = 0;
  for (index = 0; index < 256; index++)
  {
    addSmallPtEntry(ptd, vAddr+(SMALL_PAGE_SIZE*index), pAddr+(SMALL_PAGE_SIZE*index), domainBits, protectionBits, 0, 0, 0);
  }
}      



u32int findVAforPA(u32int physAddr)
{
  GCONTXT* gc = getGuestContext();
  u32int mask = 0xFFF00000;
  u32int masked = physAddr & mask;
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;

  int i = 0;
  
  for (i = 0; i <= PAGE_TABLE_ENTRIES; i++)
  {
    u32int entryAddress = (u32int)ptd + i*4; // entry size = 1 word
    u32int entryAtIndex = *(u32int*)entryAddress;

    u32int type = entryAtIndex & 0x3;
    switch(type)
    {
      case SECTION:
        if ((entryAtIndex & mask) == masked) 
        {
          u32int VA = i * 1024 * 1024;
          return VA+(physAddr & 0x000FFFFF);
        }
        break;
      case PAGE_TABLE:
      case FAULT:
      case RESERVED:
      default:
        break;
    } // switch ends
  }

  return 0;
}

// check if a virtual address is in any of the page tables.
bool isAddrInGuestPT(u32int vaddr)
{
  GCONTXT* gc = getGuestContext();

  if (gc->PT_os == 0)
  {
    // no guest page table yet.
    return FALSE;
  }

  // is the address in guests first level page table?
  if((vaddr >= (u32int)gc->PT_os) && (vaddr < ((u32int)gc->PT_os + PAGE_TABLE_SIZE - 3)) ) //16KB
  {
    return TRUE;
  }
  
  // if not in first level page table, maybe in some second level page table?
  u32int metaArrayIndex = 0;
  while (guestSecondLvlPageTables[metaArrayIndex] != 0)
  {
    // found a second level guest page table.
    u32int paddrSecond = guestSecondLvlPageTables[metaArrayIndex];
    u32int paddrGiven = getPhysicalAddress(gc->PT_os, vaddr);
    if((paddrGiven >= paddrSecond) && (paddrGiven < (paddrSecond + SECOND_LEVEL_PAGE_TABLE_SIZE - 3)) )
    {
      return TRUE;
    }
    metaArrayIndex++;
  }
  // otherwise, not found - return false
  return FALSE; 
}


/** Copy the entries from the guestVirtual -> guestPhysical -> realPhysical
Takes three page Tables:
* the shadow destination PT
* the guest OS PT
* the hypervisor PT for guest Physical to real physical mapping

Note this is a VERY expensive operation!

*/
void copyPageTable(descriptor* guest, descriptor* shadow)
{
  if(0 == shadow)
  {
    serial_ERROR("Shadow pageTable is null. copyPageTable (pageTable.c) Entering infinite loop...");
  }

  if(0 == guest)
  {
    serial_ERROR("guest pageTable is null. copyPageTable (pageTable.c) Entering infinite loop...");
  }

#ifdef PT_SHADOW_DEBUG
  serial_putstring("copyPageTable: Dumping guest pagetable");
  serial_newline();
  dumpPageTable(guest);
  serial_putstring("copyPageTable: End guest pageTable dump");
  serial_newline();
#endif

  //loop over the guest PT
  u32int i;
  for(i=0; i < PAGE_TABLE_ENTRIES; i++)
  {
   /** Page table spec does not mandate that bits 31-2 are clear
    *This could fail if the guest OS does not clear the entry when removing it
   *(i.e. rather than writing 32bit zero to the entry, the OS just clears/sets the type bit field to 0b00)
   *and we should do the relativly more expensive test (0 != currentPTE->type)
    */

    if( (guest[i].type == SECTION) || (guest[i].type == PAGE_TABLE) )
    {
      /*
       This (guest pagetable mapping hypervisor memory space) could cause serious problems
       if not caught early
      */
      const u32int hypervisorStart = (HYPERVISOR_START_ADDR) >> 20;
      const u32int hypervisorEnd = (HYPERVISOR_START_ADDR + (TOTAL_MACHINE_RAM/4) -1) >> 20;
      if(i >= hypervisorStart && i <= hypervisorEnd)
      {
        serial_putstring("guest is attemping to use hypervisor 1:1 mapped address space. 0x");
        serial_putint(i << 20);
        serial_newline();
        serial_ERROR("Entering infinite loop...");
      }

      if(guest[i].type == SECTION)
      {
        sectionDescriptor* guestSd = (sectionDescriptor*) &guest[i];
        sectionDescriptor* shadowSd = (sectionDescriptor*) &shadow[i];

        //Supersection check is done inside copySectionEntry
        copySectionEntry(guestSd, shadowSd);
      }
      else
      {
        //We are looking at small / large pages
        pageTableDescriptor* guestPTD = (pageTableDescriptor*) &guest[i];
        pageTableDescriptor* shadowPTD = (pageTableDescriptor*) &shadow[i];

        /** Step 1 */
        //Check the Page Table entry points to a valid 2nd level address
        u32int baseAddr = guestPTD->addr << 10;
        if( (baseAddr < BEAGLE_RAM_START) || (baseAddr > BEAGLE_RAM_END-3) )
        {
          //2nd level address is not valid, don't bother creating an entry in the shadow PT
          //When the guest OS actually updates the addr for this entry.
          //We will recreate it
          break;
        }

        /** Step 2 */
        //Check for existance of shadow 2nd level PT at this virtual address
        if(FAULT == shadowPTD->type)
        {
          //Doesn't already exist, need to create a new one
          u32int* newFrame = allocFrame(HYPERVISOR_FA_DOMAIN);
          if(0 == newFrame)
          {
            serial_ERROR("Could not allocate new frame in copyPageTable (pageTable.c)");
          }

          memset(newFrame, 0, SECOND_LEVEL_PAGE_TABLE_SIZE);
          shadowPTD = (pageTableDescriptor*) newFrame;
        }

        /** Step 3 */
        //We have a shadow 2nd level page table to add entries to

        //Has the guest actually created the 2nd level table?
        if( (guestPTD->addr >= BEAGLE_RAM_START) &&
             (guestPTD->addr < BEAGLE_RAM_END -1 - SECOND_LEVEL_PAGE_TABLE_SIZE) )
        {
          //2nd level guest pte is in range

          //Loop over the 2nd level guest page table
          u32int j;
          for(j=0; j < SECOND_LEVEL_PAGE_TABLE_ENTRIES; j++)
          {
            if(FAULT == guestPTD[j].type)
            {
            //easier to check for a fault first (most likly case?)
              break;
            }
            if(LARGE_PAGE == guestPTD[j].type)
            {
              largeDescriptor* guestLd = (largeDescriptor*) &(guestPTD[j]);
              largeDescriptor* shadowLd = (largeDescriptor*) &(shadowPTD[j]);
              copyLargeEntry(shadowLd, guestLd);
            }
            else
            {
              //Must be small
              smallDescriptor* guestSd = (smallDescriptor*) &(guestPTD[j]);
              smallDescriptor* shadowSd = (smallDescriptor*) &(shadowPTD[j]);
              copySmallEntry(shadowSd, guestSd);
            }
          }//for loop 2nd level page table
        }
      }//else !SECTION
    }// if !FAULT
#ifdef PT_SHADOW_DEBUG
    else if(RESERVED == guest[i].type)
    {
      //Not going to copy this entry.  The guest will never get to read the sPT, so no need to.
      //Whatever reason the guest is using the RESERVED entry for!
      serial_putstring("RESERVED entry in shadow page table copyPageTable (pageTable.c). continuing");
      serial_newline();
    }
#endif
  }//for loop 1st level page table
}

/*
Needs putting somewhere more appropriate, perhaps its own file
once we implement guest domain mapping (held in CP15?/guestContext?)
*/
u8int  mapGuestDomain(u8int guestDomain)
{
#ifdef PT_SHADOW_DEBUG
  serial_putstring("UNIMPLEMENTED: mapGuestDomain (cp15coproc.c). continuing");
  serial_newline();
#endif
#ifdef PT_DOMAIN_DEBUG
  if(guestDomain == HYPERVISOR_ACCESS_DOMAIN)
  {
    serial_putstring("guestDomain is same as hypervisor domain. mapGuestDomain (pageTable.c)");
    serial_newline();
  }
#endif

  return GUEST_ACCESS_DOMAIN;
}

u32int getPageEndAddr(descriptor* ptd, u32int address)
{
  //Get the PT type, mask the virtual address, return the end addr of the page
  descriptor* pte1st = get1stLevelPtDescriptorAddr(ptd, address);
#ifdef PT_DEBUG
  serial_putstring("getPageEndAddr: ptd=");
  serial_putint((u32int)ptd);
  serial_putstring(" addr=");
  serial_putint(address);
  serial_putstring(" PTentry=");
  serial_putint((u32int)pte1st);
  serial_putstring(" val=");
  serial_putint(*(u32int*)pte1st);
  serial_newline();
#endif

  u32int mask;

  switch(pte1st->type)
  {
    case SECTION:
      ;//GCC label before statement bug
      sectionDescriptor* sd = (sectionDescriptor*)pte1st;
      if(1 == sd->sectionType)
      {
        //SUPERSECTION
        serial_ERROR("UNIMPLEMENTED: getPageEndAddr for SUPERSECTION. Entering infinite loop");
        return 0; // error
      }
      else
      {
        mask = 0xFFF00000;
        address = address & mask;
        address += SECTION_SIZE -1;
        return address;
      }
      break;
    case PAGE_TABLE:
      ;
      descriptor* pte2nd = get2ndLevelPtDescriptor((pageTableDescriptor*)pte1st, address);

      switch(pte2nd->type)
      {
        case LARGE_PAGE:
          ;
          mask = 0xFFFF0000;
          address = address & mask;
          address += LARGE_PAGE_SIZE -1;
          return address;
          break;
        case SMALL_PAGE: //fall through
        case SMALL_PAGE_3:
          ;
          mask = 0xFFFFF000;
          address = address & mask;
          address += SMALL_PAGE_SIZE -1;
          return address;
          break;
        case FAULT: //fall through
        default:
#ifdef PT_SHORT_DEBUG
          serial_putstring("Error: getPageEndAddr FAULT/RESERVED (pageTable.c)");
          serial_newline();
#endif
          return 0; //This is an impossible value, so good for errors
          break;
      }
      break;
    case FAULT: //fall through
    case RESERVED: //fall through
    default:
#ifdef PT_SHORT_DEBUG
      serial_putstring("Error: getPageEndAddr FAULT/RESERVED (pageTable.c)");
      serial_newline();
#endif
      return 0; //This is an impossible value, so good for errors
      break;
  }
}

/* Called from the instruction emulator when we have a write privilege fault */
void pageTableEdit(u32int address, u32int newVal)
{
#ifdef PT_DEBUG
  serial_putstring("PageTableEdit: address ");
  serial_putint(address);
  serial_putstring(" newval: ");
  serial_putint(newVal);
  serial_newline();
#endif
  /*
  * So we have the new value to store and the address its going to happen at.
  * We need to update the sPT entry with the new information.
  * In order to do this we need to get the virtual address of the guestOS PT being written
  * So we can traverse our sPT.
  * Especially so if the entry being updated is in a 2nd level table
  * as we have no idea where it is in relation to the 1st level page table
  */

  GCONTXT* gc = (GCONTXT*)getGuestContext();

  u32int virtualAddr;
  bool firstLevelEntry;

  /**
    Step 1: Get the virtual fault address
  */

  //If we faulted on a write to the 1st level PT then we already have the virtual address
  if( (u32int)gc->PT_os == (address & 0xFFFFC000) )
  {
    //Bits [31:14] of the fault address match the base addr of the guest OS pT.
    //We are dealing with a write to the main PT (section / page table entry)
    firstLevelEntry = TRUE;

    //So get the virtual address by shifting bits 13:2 of the fault addr to be bits 31:20 of the virt addr
    virtualAddr = address << 18;
  }
  else
  {
    //Not a write to the first level PT.
    //So its a write to an existing 2nd Level table, which we now need to find, to get the full virtual address
    firstLevelEntry = FALSE;

    //The first level PT contains a ptr to the physical address of the (1KB) 2nd level PT.
    //We have the address of the 2nd level PT, but don't know which segment of virt mem it serves
    //Address to 2nd level PT that we want to match (held in the PageTable struct so its right shifted)
    descriptor* pteAddr = get1stLevelPtDescriptorAddr(gc->PT_os, address);
    u32int pte = 0;
    switch (pteAddr->type)
    { 
      case FAULT:
        serial_ERROR("pageTableEdit: fault entry hit - ?");
        break;
      case PAGE_TABLE:
        serial_ERROR("pageTableEdit: second level entry hit - unimplemented.");
        break;
      case SECTION:
      {
        pte = (((sectionDescriptor*)pteAddr)->addr) << 20; 
#ifdef PT_SHADOW_DEBUG
        serial_putstring("1st level page table section entry @ ");
        serial_putint((u32int)pteAddr);
        serial_putstring("=");
        serial_putint(pte);
        serial_newline();
#endif
        break;
      }
      case RESERVED:
        serial_ERROR("pageTableEdit: reserved entry hit - ?");
        break;
      default:
        break;
    } // switch ends

    // translate faulting address to physical address
    u32int faultPA = pte | (address & 0x000FFFFF);
    faultPA = faultPA & 0xFFFFFC00;

#ifdef PT_SHADOW_DEBUG
    serial_putstring("pageTableEdit: 2nd level entry we are looking for: 0x");
    serial_putint(faultPA);
    serial_newline();
#endif


    u32int gPageTableBase = (u32int)gc->PT_os;
    // loop over the entire GUEST 1st level PT 
    // looking for a match to the 2nd level base address.
    u32int i;
    for(i=0;i < PAGE_TABLE_ENTRIES; i++)
    {
      u32int lvl1entry = *(u32int*)(gPageTableBase + i*4);
      if ( ((lvl1entry & 0x3) == PAGE_TABLE) &&
           ((lvl1entry & 0xFFFFFC00) == faultPA) )
      {
#ifdef PT_SHADOW_DEBUG
        serial_putstring("pageTableEdit: found entry at index ");
        serial_putint_nozeros(i);
        serial_putstring(" = ");
        serial_putint(lvl1entry);
        serial_newline();
#endif
        break;
      }
    }

    //Now have the first 12bits of the virtual address bits 31:20
    //and the bits 19:12 of the virtual address are from bits 10:2 of the fault address
    virtualAddr = (i << 20) | ((address & 0x3FC) << 10);
  }
  // virtualAddr points to the shadow 1st level page table entry for the faulting addr
  // get this entry from the shadow table
  descriptor* shadowEntry = get1stLevelPtDescriptorAddr(gc->PT_shadow, virtualAddr);
  descriptor* oldGuestEntry = (descriptor*)address;
  descriptor* newGuestEntry = (descriptor*)&newVal;


#ifdef PT_SHADOW_DEBUG
  if (newVal != 0)
  {
    serial_putstring("pageTableEdit: addr ");
    serial_putint(address);
    serial_putstring(" newVal ");
    serial_putint(newVal);
    serial_putstring(" virtualAddr ");
    serial_putint(virtualAddr);
    serial_newline();
    serial_putstring("pageTableEdit: shadowEntry ");
    serial_putint((*(u32int*)shadowEntry));
    serial_putstring(" oldGuestEntry ");
    serial_putint((*(u32int*)oldGuestEntry));
    serial_putstring(" newGuestEntry ");
    serial_putint((*(u32int*)newGuestEntry));
    serial_newline();
  }
#endif

  /**
  Step 2: Work Out what kind of change we are dealing with
  */

  /** Remove, or change entry type (Remove & Add) */
  //Changing pte type if not from !Fault to Fault, is a remove then add operation
  if ((oldGuestEntry->type != newGuestEntry->type) && (oldGuestEntry->type != FAULT) )
  {
#ifdef PT_SHADOW_DEBUG
    serial_putstring("pageTableEdit: remove/change type case");
    serial_newline();
#endif
    //Need to do special things if its a PAGE_TABLE entry or a LARGE_PAGE
    if (firstLevelEntry && (oldGuestEntry->type == PAGE_TABLE))
    {
      //issues with memory protection needing to be removed & flushing of all the sub mappings
      removePageTableEntry((pageTableDescriptor*)shadowEntry);
    }
    else
    {
      //Need to flush block cache at these addresses first
      // but which addresses to flush? shadow entries might have been fragmented to pages from a section...
      // so flush address range that the guest mapped originally.
      validateCacheMultiPreChange(gc->blockCache, virtualAddr, getPageEndAddr(gc->PT_os,  virtualAddr));

      if(!firstLevelEntry && (LARGE_PAGE == oldGuestEntry->type) )
      {
        *(u32int*)shadowEntry = 0;
        //have to remove all 16 copies of the entry
        replicateLargeEntry((largeDescriptor*)shadowEntry);
      }
      else
      {
        //Small & section types aren't complicated
        *(u32int*)shadowEntry = 0;
      }
    }
  }

  /** Add */
  if( (newGuestEntry->type != FAULT) && (oldGuestEntry->type != newGuestEntry->type) )
  {
#ifdef PT_SHADOW_DEBUG
    serial_putstring("pageTableEdit: Add entry case");
    serial_newline();
#endif
    if(firstLevelEntry)
    {
      if(SECTION == newGuestEntry->type)
      {
        copySectionEntry((sectionDescriptor*)newGuestEntry, (sectionDescriptor*)shadowEntry);
      }
      else
      {
        copyPageTableEntry((pageTableDescriptor*)newGuestEntry, (pageTableDescriptor*)shadowEntry);
      }
    }
    else
    {
      if(LARGE_PAGE == newGuestEntry->type)
      {
        // unimplemented
        copyLargeEntry((largeDescriptor*)newGuestEntry, (largeDescriptor*)shadowEntry);
      }
      else
      {
        // to copySmallEntry:
        // need to give newGuestEntry, and a pointer to the second level shadow page
        // table, where this new entry should be stored (adjusted!)
        // ATM shadowEntry points to the 1st level page table entry. need to get
        // second level page table address.
        u32int secondLvlEntryAddr = (*(u32int*)shadowEntry) & 0xFFFFFC00;
        // now find the correct entry address in the second level shadow table...
        secondLvlEntryAddr = secondLvlEntryAddr | ((virtualAddr >> 10) & 0x3FC);
        copySmallEntry((smallDescriptor*)newGuestEntry, (smallDescriptor*)secondLvlEntryAddr);
      }
    }
  }
  else
  {
    /** Change entry details */
    //early exit if OS is not actually updating anything
    if(*(u32int*)oldGuestEntry == newVal)
    {
#ifdef PT_SHADOW_DEBUG
      serial_putstring("Init (zeroing entry) @ 0x");
      serial_putint(address);
      serial_putstring(", Virtual Page : 0x");
      serial_putint(virtualAddr);
      serial_newline();
#endif
      return;
    }

#ifdef PT_SHADOW_DEBUG
    serial_putstring("pageTableEdit, EDIT entry case @ 0x");
    serial_putint(address);
    serial_putstring(", oldValue: 0x");
    serial_putint(*(u32int*)address);
    serial_putstring(", newValue: 0x");
    serial_putint(newVal);
    serial_putstring(", shadowEntry: 0x");
    serial_putint(*(u32int*)shadowEntry);
    serial_newline();
#endif

    //So both entries are of the same types, SECTION or PAGE_TABLE
    if( firstLevelEntry && (oldGuestEntry->type == SECTION))
    {
      sectionDescriptor* oldSd = (sectionDescriptor*)oldGuestEntry;
      sectionDescriptor* newSd = (sectionDescriptor*)newGuestEntry;
      sectionDescriptor* shadowSd = (sectionDescriptor*)shadowEntry;

      if( (oldSd->sectionType != newSd->sectionType)  || (oldSd->addr != newSd->addr) )
      {
        //Change of section to Supersection or change of physical address is a remove & add operation
        validateCacheMultiPreChange(gc->blockCache, virtualAddr, getPageEndAddr(gc->PT_shadow, virtualAddr));
        *(u32int*)shadowSd = 0;
        copySectionEntry(newSd, shadowSd);
      }

      if(newSd->sectionType == 1)
      {
        //Assums the initial case when the sd is written/copied that we stop on attempts to add supersections
        //So don't need to check the old value
        serial_ERROR("UNIMPLEMENTED: pageTableEdit SUPERSECTIONS");
        //Only difference is that we may have an extended base address
        //There is no domain field.
      }

      //Domain changes
      if(oldSd->domain != newSd->domain)
      {
        //need domain emulation support?
        //To start just need to check that the domain the guest tries to use is not used by the Hypervisor
        //and that it is set to "client mode" so we can do memory access control
        serial_ERROR("UNIMPLEMENTED: pageTableEdit, edit case: section, change of domain field");
      }

      //Access permission changes
      if(oldSd->ap2 == newSd->ap2)
      {
        //crazy case that we probably won't see very often (Mapping a page but giving no access to it at all)
        if( (0 == oldSd->ap2) && (oldSd->ap10 != newSd->ap10) && (newSd->ap10 == 0b00) )
        {
          //Set PRIV R/W, USR No access
          shadowSd->ap2 = PRIV_RW_USR_NO >> 2;
          shadowSd->ap10 = PRIV_RW_USR_NO & 0x3;
        }
      }
      else
      {
        //changes to ap2
        if(newSd->ap2 == 0)
        {
          //Set PRIV RW USR No access
          shadowSd->ap2 = PRIV_RW_USR_NO >> 2;
          shadowSd->ap10 = PRIV_RW_USR_NO & 0x3;
        }
        else
        {
          //Set PRIV RW USR RO
          shadowSd->ap2 = PRIV_RW_USR_RO >> 2;
          shadowSd->ap10 = PRIV_RW_USR_RO & 0x3;
        }
      }

      //Do the check anyway reads are less expensive than writes, & the values might be in registers
      if(oldSd->xn != newSd->xn)
      {
        shadowSd->xn = newSd->xn;
      }

      //Carefull of this one, field is used by the hypervisor
      if(oldSd->imp != newSd->imp )
      {
        //If we hit this then I need to rethink how we check for memory protection being set on a page before modification
        //As the guestOS is now using the field!
        serial_ERROR("pageTableEdit: edit case: pageTable, Use of implementation defined field. Check what guestOS does with this as the Hypervisor uses it!");
      }

      //Other fields are: NS(Non secure), C(caching), B(Bufferable),S(shareable),TEX, nG
      //nG(non-global) is not included on the basis that all OS entries will be global?

      //We don't care about these right now.

      //All done
    }
    else if(firstLevelEntry)
    {
      //PAGE_TABLE type
      pageTableDescriptor* newPtd = (pageTableDescriptor*)newGuestEntry;
      pageTableDescriptor* oldPtd = (pageTableDescriptor*)oldGuestEntry;

      if(oldPtd->addr != newPtd->addr)
      {
        //Change in address is same as a removeEntry and addNew one
        removePageTableEntry((pageTableDescriptor*)shadowEntry);
        copyPageTableEntry(newPtd, (pageTableDescriptor*)shadowEntry);
      }

      if(oldPtd->domain != newPtd->domain)
      {
        //need domain emulation support?
        //To start just need to check that the domain the guest tries to use is not used by the Hypervisor
        //and that it is set to "client mode" so we can do memory access control
        serial_ERROR("UNIMPLEMENTED: pageTableEdit, edit case: pageTable, change of domain field");
      }

      /* Carefull of this one, field is used by the hypervisor */
      if(oldPtd->imp != newPtd->imp )
      {
        //If we hit this then I need to rethink how we check for memory protection being set on a page before modification
        //As the guestOS is now using the field!
        serial_ERROR("pageTableEdit: edit case: pageTable, Use of implementation defined field. Check what guestOS does with this as the Hypervisor uses it!");
      }

      //Other fields are: NS(Non secure).
      //We don't care about these now.

      //All done
    }
    else if(LARGE_PAGE == newGuestEntry->type)
    {
      largeDescriptor* ld = (largeDescriptor*) address;
      serial_ERROR("UNIMPLEMENTED: pageTableEdit: edit large page case");
      //Large pages need to be copied 16times.
      replicateLargeEntry(ld);

      //Also means all 16 entries will need to be flushed if present
      u32int i;
      for(i=0; i< 16; i++)
      {
        clearTLBbyMVA(address);
        address+=4;
      }
    }
    else
    {
      //Small Page
      serial_ERROR("UNIMPLEMENTED: pageTableEdit, edit Small page case");
    }
  }

  clearTLBbyMVA(address);
  return;
}

/* Given a virtual address, retrieves the underlying physical address */
u32int getPhysicalAddress(descriptor* ptd, u32int virtualAddress)
{
  descriptor* pte1st = get1stLevelPtDescriptorAddr(ptd, virtualAddress);
  switch(pte1st->type)
  {
    case SECTION:
      ;//GCC label bug
      sectionDescriptor* sd = (sectionDescriptor*)pte1st;
      if(1 == sd->sectionType)
      {
        serial_ERROR("UNIMPLEMENTED: supersection getPhysicalAddress (pageTable.c)");
      }
      else
      {
        u32int result = (sd->addr << 20) | (virtualAddress & 0x000FFFFF);
        return result;
      }
      break;
    case PAGE_TABLE:
      ;//GCC label bug
      descriptor* pte2nd = get2ndLevelPtDescriptor((pageTableDescriptor*)pte1st, virtualAddress);
      switch(pte2nd->type)
      {
        case LARGE_PAGE:
          ;//GCC label bug
          largeDescriptor* ld = (largeDescriptor*)pte2nd;
          return (ld->addr << 16) | (virtualAddress & 0x0000FFFF);
          break;
        case SMALL_PAGE: //fall through
        case SMALL_PAGE_3:
          ;//GCC label bug
          smallDescriptor* sd = (smallDescriptor*)pte2nd;
          return (sd->addr << 12) | (virtualAddress & 0x00000FFF);
          break;
        case FAULT: //fall through
        default:
          return 0xffffffff; //most likly to be invalid / cause an error!
          break;
      }
      break;
    case FAULT: //fall through
    case RESERVED: //fall through
    default:
      return 0xffffffff;
      break;
  }
}

void disableHypervisorCaches(descriptor* ptd)
{
  sectionDescriptor* sd;

  u32int i;
  const u32int hypervisorEndAddr = (HYPERVISOR_START_ADDR + (TOTAL_MACHINE_RAM/4)) >> 20;
  for(i= (HYPERVISOR_START_ADDR >> 20); i <= hypervisorEndAddr; i++ )
  {
    sd = (sectionDescriptor*)(&ptd[i]);
    sd->c=0;
    sd->b=0;
    sd->tex=0;
  }
}

/** Set of asserts used to check constraints on the pageTable subsystem at compile time */
void pt_compile_time_check(void)
{
  /* Check packed struct size are correct */
  COMPILE_TIME_ASSERT((sizeof(descriptor) == sizeof(u32int)) , _descriptor_struct_not_32bit);
  COMPILE_TIME_ASSERT((sizeof(pageTableDescriptor) == sizeof(u32int)) , _pageTableDescriptor_struct_not_32bit);
  COMPILE_TIME_ASSERT((sizeof(sectionDescriptor) == sizeof(u32int)) , _sectionDescriptor_struct_not_32bit);
  //COMPILE_TIME_ASSERT((sizeof(superDescriptor) == sizeof(u32int)) , _superDescriptor_struct_not_32bit);
  COMPILE_TIME_ASSERT((sizeof(largeDescriptor) == sizeof(u32int)) , _largeDescriptor_struct_not_32bit);
  COMPILE_TIME_ASSERT((sizeof(smallDescriptor) == sizeof(u32int)) , _smallDescriptor_struct_not_32bit);

  /* Check the pageType enum */
  COMPILE_TIME_ASSERT(0 == FAULT, __pageType_fault_enum_has_changed_);
  COMPILE_TIME_ASSERT(1 == PAGE_TABLE, __pageType_page_table_enum_has_changed_);
  COMPILE_TIME_ASSERT(2 == SECTION, __pageType_section_enum_has_changed_);
  COMPILE_TIME_ASSERT(3 == RESERVED, __pageType_reserved_enum_has_changed_);
  COMPILE_TIME_ASSERT(1 == LARGE_PAGE, __pageType_large_page_enum_has_changed_);
  COMPILE_TIME_ASSERT(2 == SMALL_PAGE, __pageType_small_page_enum_has_changed_);
  COMPILE_TIME_ASSERT(3 == SMALL_PAGE_3, __pageType_small_page_3_enum_has_changed_);
}
