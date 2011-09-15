#include "common/debug.h"

#include "guestManager/blockCache.h"

#include "memoryManager/addressing.h"
#include "memoryManager/memoryConstants.h"
#include "memoryManager/memoryProtection.h"
#include "memoryManager/mmu.h"
#include "memoryManager/pageTable.h"


extern void setGuestPhysicalPt(GCONTXT* gc);


void initialiseVirtualAddressing()
{
  descriptor* ptAddr = createHypervisorPageTable();

  printf("Initializing Virtual Addressing. Page Table @ %08x\n", (u32int)ptAddr);

  mmuInit();
  mmuInsertPt0(ptAddr); //Map Hypervisor PT into TTBR0
  mmuEnableVirtAddr();
}

/* virtual machine startup, need to add a new guestPhysical to ReadPhysical address map */
/* NOT used in current deliverable */
void createVirtualMachineGPAtoRPA(GCONTXT* gc)
{
#ifdef ADDRESSING_DEBUG
  printf("createVirtualMachineGPAtoRPA: TODO createVirtualMachineGPAtoRPA (addressing.c)\n");
#endif

  /*
   * The hypervisor ptd is the guest physical ptd for now
   * Markos: Well, what this means is the the hypervisor page table is the page table the the guest will use to access memory
   */
  setGuestPhysicalPt(gc);

  /*Alex: implementation thoughts
   Allocate section of real memory for VM (rather than doing it dynamically for now)
  create GuestPhysical to RealPhysical address mappings.
  create a Global Shadow Page Table?
  cache maintainace?
  Need to tell something what co-processor / other state needs to be intercepted and faked during startup
  Loading kernel into memory, need to add a GuestPhysical to RealPhysical address mapping into PT
  boot Kernel
  */
}

/* Intercept Linux enabling virtual memory */
void initialiseGuestShadowPageTable(u32int guestPtAddr)
{
  GCONTXT* context = getGuestContext();

#ifdef ADDRESSING_DEBUG
  printf("initialiseGuestShadowPageTable: new pt addr %08x @ pc %08x\n", guestPtAddr, context->R15);
#endif

  //This needs changing to, or similar
  //if(gc->decompressionDone)
  if(guestPtAddr == 0x80004000)
  {
#ifdef ADDRESSING_DEBUG
    printf("initialiseGuestShadowPageTable: TTBR0 Linux identity mapping bootstrap, ignoring.\n");
#endif
    return;
  }

  //This was really annoying to find, linux set the bottom bits for some reason?!
  //Perhaps to check that the table ptr has been updated and is not still the identity map?
  guestPtAddr = guestPtAddr &  0xFFFFC000;

  if(context->virtAddrEnabled)
  {
    // explode block cache
    explodeCache(context->blockCache);

    // create a new shadow page table. Mapping in hypervisor address space
    descriptor* newShadowPt = createGuestOSPageTable();

    // remove metadata about old sPT1
    removePT2Metadata();

    // update guest context entries
    // map new page table address to a 1-2-1 virtual address
    // (now we can safely tamper with sPT, as it will be discarded soon)
    sectionMapMemory(context->PT_shadow, (guestPtAddr & 0xFFF00000),
                 (guestPtAddr & 0xFFF00000)+(SECTION_SIZE-1),
                 HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0b000);
    // in this new 1st level sPT, find guestVirtual address for 1st lvl gPT address
    // update 1st level shadow page table pointer
    context->PT_os_real = (descriptor*)guestPtAddr;
    context->PT_os = (descriptor*)guestPtAddr;
    u32int ptGuestVirtual = findGuestVAforPA(guestPtAddr);
    context->PT_os = (descriptor*)ptGuestVirtual;

    // update 1st level shadow page table pointer
    descriptor* oldShadowPt = context->PT_shadow;
    context->PT_shadow = newShadowPt;

    // copy new gPT1 entries to new sPT1
    copyPageTable((descriptor*)guestPtAddr, context->PT_shadow);
#ifdef ADDRESSING_DEBUG
    printf("initialiseGuestShadowPageTable: Copy PT done.\n");
#endif

    // anything in caches needs to be written back now
    dataBarrier();

    // tell CP15 of this new base PT
    setTTBCR((u32int)context->PT_shadow);

    // clean tlb and cache entries
    clearTLB();
    clearInstructionCache();
    clearDataCache();

    // add protection to guest page table.
    u32int guestPtVirtualEndAddr = ptGuestVirtual + PAGE_TABLE_SIZE - 1;
    //function ptr to the routine that handler gOS edits to its PT
    addProtection(ptGuestVirtual, guestPtVirtualEndAddr, &pageTableEdit, PRIV_RW_USR_RO);

    // 9. clean old shadow page table
    invalidateSPT1(oldShadowPt);

    //now running with new shadow page tables
  }
  else
  {
#ifdef ADDRESSING_DEBUG
    printf("initialiseGuestShadowPageTable: set gPT ptr in gContext\n");
#endif
    // guest virtual addressing is not active, no need to spend time
    // faulting on PT that the OS is going to add entries to before it activates
    context->PT_os = (descriptor*)guestPtAddr;
  }
}


void guestEnableVirtMem()
{
  GCONTXT* gc = getGuestContext();

  if(gc->PT_os == 0)
  {
#ifdef ADDRESSING_DEBUG
    printf("guestEnableVirtMem: gc->PT_os=0. Must be identity mapping bootstrap, ignore hypervised\n");
#endif
    return;
  }

#ifdef ADDRESSING_DEBUG
  dumpGuestContext(gc);
  printf("guestEnableVirtMem: Dumping guest page table from addr in gc->PT_os @ %08x\n", (u32int)gc->PT_os);
  dumpPageTable(gc->PT_os);
  printf("guestEnableVirtMem: PT dump done\n");
#endif

  if(gc->virtAddrEnabled)
  {
#ifdef ADDRESSING_DEBUG
    DIE_NOW(gc, "guest virtual addressing is already enabled");
#endif
  }

#ifdef ADDRESSING_DEBUG
  printf("guestEnableVirtMem: Enabling guest virtual / shadow page tables\n");
#endif

  //create a new shadow page table. Mapping in hypervisor address space
  descriptor* sPT = createGuestOSPageTable();
  gc->PT_shadow = sPT;

  // remove metadata about old sPT1
  removePT2Metadata();

  //map all the guest OS pt entries into the shadow PT, using the GuestPhysical to ReadPhysical PT map
  copyPageTable(gc->PT_os, sPT);

#ifdef ADDRESSING_DEBUG
  printf("guestEnableVirtMem: Copy PT done. Dumping shadow PT\n");
  dumpPageTable(sPT);
  printf("guestEnableVirtMem: shadow PT dump done.\n");
  printf("guestEnableVirtMem: About to switch to sPT\n");
#endif


  /* Quick prototyping code for now. needs doing properly at some point */
  explodeCache(gc->blockCache);

  dataBarrier();  //anything in caches needs to be written back now
  setTTBCR((u32int)sPT); //swap shadow PT
  clearTLB(); //clean out all TLB entries - may have conflicting entries
  clearInstructionCache(); //just to make sure

#ifdef ADDRESSING_DEBUG
  printf("guestEnableVirtMem: Using sPT. Continuing\n");
#endif

  /**  WARNING: HACK
    linux removes 1:1 mapping for PT soon after jumping into 0xc00xxxxx addr space
    We need to keep the physical addr of the PT
    for PT switches, e.g. when switching to other guests
  */
  gc->PT_os_real = gc->PT_os;
  gc->PT_os = (descriptor*)0xc0004000; //This value needs to be auto calculated?
  //Perhaps loop over the current PT looking for the first underlying phyAddr that matches

  //u32int pt_virt_addr = findVirtualAddr(gc->PT_os, gc->PT_os_real);
  /** End HACK */

  //Mark virtual addressing as now enabled
  gc->virtAddrEnabled = TRUE;

  u32int guestPtAddr = (u32int)gc->PT_os;
  u32int guestPtEndAddr = guestPtAddr + PAGE_TABLE_SIZE;

  // get the shadow entry for where guest PT lives in
  descriptor* shadowEntry = get1stLevelPtDescriptorAddr(gc->PT_shadow, guestPtAddr);
  // if the shadow entry is a section, split it up to pages for better protection
  if (shadowEntry->type == SECTION)
  {
    splitSectionToSmallPages(gc->PT_shadow, guestPtAddr);
  }

  //function ptr to the routine that handler gOS edits to its PT
  addProtection(guestPtAddr, guestPtEndAddr-1, &pageTableEdit, PRIV_RW_USR_RO);
}


void changeGuestDomainAccessControl(u32int oldVal, u32int newVal)
{
  GCONTXT* context = getGuestContext();
  if(context->virtAddrEnabled)
  {
    // loop through DACR entries checking which entry was changed
    u32int i = 0;
    for (i = 0; i < 16; i++)
    {

      // look for domains that had their configuration changed.
      if ( ((oldVal >> (i*2)) & 0x3) != ((newVal >> (i*2)) & 0x3) )
      {
#ifdef ADDRESSING_DEBUG
        printf("changeGuestDomainAccessControl: changing config for dom %x\n", i);
#endif
        // for every entry changed, loop through all page table entries
        u32int y = 0;
        for (y = 0; y < PAGE_TABLE_ENTRIES; y++)
        {
          // if page table entry is assigned to that domain remap AP bits
          sectionDescriptor* ptEntry = (sectionDescriptor*)&context->PT_os[y];
          if ((ptEntry->type == FAULT) || (ptEntry->type == RESERVED))
          {
            // invalid entry, leave.
            continue;
          }
          if (ptEntry->domain == i)
          {
#ifdef ADDRESSING_DEBUG
            printf("changeGuestDomainAccessControl: PTe %x = %08x needs AP bits remapped.\n", y, *(u32int*)ptEntry);
#endif
            if (ptEntry->type == SECTION)
            {
              descriptor* shadowPtEntry = &(context->PT_shadow[y]);
              mapAPBitsSection(y*1024*1024, ptEntry, shadowPtEntry);
#ifdef ADDRESSING_DEBUG
              printf("changeGuestDomainAccessControl: remapped to %08x\n", *(u32int*)ptEntry);
#endif
            }
            else if (ptEntry->type == PAGE_TABLE)
            {
#ifdef ADDRESSING_DEBUG
              printf("changeGuestDomainAccessControl: remap AP for page table entry\n");
#endif
              descriptor* shadowPtEntry = &(context->PT_shadow[y]);
              mapAPBitsPageTable(y*1024*1024, (pageTableDescriptor*)ptEntry,
                                         (pageTableDescriptor*)shadowPtEntry);
            }
          }

        } // for loop - all page table entries

      } // if - domain config changed

    } // for loop - through all domain entries
  }
  else
  {
    // virtual memory turned off. doesnt effect anything.
    return;
  }
}

