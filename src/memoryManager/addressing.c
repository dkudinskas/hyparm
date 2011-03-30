#include "common/debug.h"

#include "guestManager/blockCache.h"

#include "memoryManager/addressing.h"
#include "memoryManager/memoryConstants.h"
#include "memoryManager/memoryProtection.h"
#include "memoryManager/mmu.h"
#include "memoryManager/pageTable.h"


extern GCONTXT * getGuestContext(void); //from main.c
extern void setGuestPhysicalPt(GCONTXT* gc);


void initialiseVirtualAddressing()
{
  DEBUG_STRING("Initializing Virtual Addressing.");
  descriptor* ptAddr = createHypervisorPageTable();

  DEBUG_STRING(" Page Table @ 0x");
  DEBUG_INT((u32int)ptAddr);
  DEBUG_STRING("...");

  mmuInit();
  mmuInsertPt0(ptAddr); //Map Hypervisor PT into TTBR0
  mmuEnableVirtAddr();

  DEBUG_STRING("done");
  DEBUG_NEWLINE();
}

/* virtual machine startup, need to add a new guestPhysical to ReadPhysical address map */
/* NOT used in current deliverable */
void createVirtualMachineGPAtoRPA(GCONTXT* gc)
{
#ifdef ADDRESSING_DEBUG
  DEBUG_STRING("createVirtualMachineGPAtoRPA: TODO createVirtualMachineGPAtoRPA (addressing.c)");
  DEBUG_NEWLINE();
#endif

  //The hypervisor ptd is the guest physical ptd for now
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
#ifdef ADDRESSING_DEBUG
  DEBUG_STRING("initialiseGuestShadowPageTable: new pt addr ");
  DEBUG_INT(guestPtAddr);
  DEBUG_NEWLINE();
#endif

  GCONTXT* context = getGuestContext();

  //This needs changing to, or similar
  //if(gc->decompressionDone)
  if(guestPtAddr == 0x80004000)
  {
#ifdef ADDRESSING_DEBUG
    DEBUG_STRING("initialiseGuestShadowPageTable: TTBR0 Linux identity mapping bootstrap, ignoring.");
    DEBUG_NEWLINE();
#endif
    DIE_NOW(context, "initialiseGuestShadowPageTable");
    return;
  }

  //This was really annoying to find, linux set the bottom bits for some reason?!
  //Perhaps to check that the table ptr has been updated and is not still the identity map?
  guestPtAddr = guestPtAddr &  0xFFFFC000;

#ifdef ADDRESSING_DEBUG
  DEBUG_STRING("initialiseGuestShadowPageTable: Dumping guest page table @ 0x");
  DEBUG_INT(guestPtAddr);
  DEBUG_NEWLINE();

  descriptor* ptd = (descriptor*)guestPtAddr;
  dumpPageTable(ptd);

  DEBUG_NEWLINE();
  DEBUG_STRING("Guest pt dump finished");
  DEBUG_NEWLINE();
#endif

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
    DEBUG_STRING("initialiseGuestShadowPageTable: Copy PT done.");
    DEBUG_NEWLINE();
#endif

    // anything in caches needs to be written back now
    dataBarrier();
    
    // tell CP15 of this new base PT
    setTTBCR((u32int)context->PT_shadow);
    
    // clean tlb and cache entries
    clearTLB();
    clearCache();
    
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
    DEBUG_STRING("initialiseGuestShadowPageTable: set gPT ptr in gContext");
    DEBUG_NEWLINE();
#endif
    //guest virtual addressing is not active, no need to spend time faulting on PT that the OS is going to add entries to before it activates
    context->PT_os = (descriptor*)guestPtAddr;
  }
}


void guestEnableVirtMem()
{
  GCONTXT* gc = getGuestContext();

  if(gc->PT_os == 0)
  {
#ifdef ADDRESSING_DEBUG
    DEBUG_STRING("guestEnableVirtMem: No entry in gc. Must be identity mapping bootstrap, ignore hypervised. Continuing boot...");
    DEBUG_NEWLINE();
#endif
    return;
  }

#ifdef ADDRESSING_DEBUG
  dumpGuestContext(gc);
  DEBUG_STRING("guestEnableVirtMem: Dumping guest page table from addr in gc->PT_os @ 0x");
  DEBUG_INT((u32int)gc->PT_os);
  DEBUG_NEWLINE();

  dumpPageTable(gc->PT_os);

  DEBUG_STRING("guestEnableVirtMem: PT dump done");
  DEBUG_NEWLINE();
#endif

  if(gc->virtAddrEnabled)
  {
#ifdef ADDRESSING_DEBUG
    DIE_NOW(0, "guest virtual addressing is already enabled");
#endif
  }

#ifdef ADDRESSING_DEBUG
  DEBUG_STRING("guestEnableVirtMem: Enabling guest virtual / shadow page tables");
  DEBUG_NEWLINE();
#endif

  //create a new shadow page table. Mapping in hypervisor address space
  descriptor* sPT = createGuestOSPageTable();
  gc->PT_shadow = sPT;

  // remove metadata about old sPT1
  removePT2Metadata();

  //map all the guest OS pt entries into the shadow PT, using the GuestPhysical to ReadPhysical PT map
  copyPageTable(gc->PT_os, sPT);

#ifdef ADDRESSING_DEBUG
  DEBUG_STRING("guestEnableVirtMem: Copy PT done. Dumping shadow PT");
  DEBUG_NEWLINE();
  dumpPageTable(sPT);
  DEBUG_STRING("guestEnableVirtMem: shadow PT dump done.");
  DEBUG_NEWLINE();
  DEBUG_STRING("guestEnableVirtMem: About to switch to sPT");
  DEBUG_NEWLINE();
#endif


  /* Quick prototyping code for now. needs doing properly at some point */
  explodeCache(gc->blockCache);

  dataBarrier();  //anything in caches needs to be written back now
  setTTBCR((u32int)sPT); //swap shadow PT
  clearTLB(); //clean out all TLB entries - may have conflicting entries
  clearCache(); //just to make sure

#ifdef ADDRESSING_DEBUG
  DEBUG_STRING("guestEnableVirtMem: Using sPT. Continuing");
  DEBUG_NEWLINE();
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
        DEBUG_STRING("changeGuestDomainAccessControl: changing config for dom ");
        DEBUG_INT(i);
        DEBUG_NEWLINE();
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
            DEBUG_STRING("changeGuestDomainAccessControl: page table entry ");
            DEBUG_INT(y);
            DEBUG_STRING(" = ");
            DEBUG_INT(*(u32int*)ptEntry);
            DEBUG_STRING(" needs AP bits remapped.");
            DEBUG_NEWLINE();
#endif
            if (ptEntry->type == SECTION)
            {
              descriptor* shadowPtEntry = &(context->PT_shadow[y]);
              mapAPBitsSection(y*1024*1024, ptEntry, shadowPtEntry);
#ifdef ADDRESSING_DEBUG
              DEBUG_STRING("changeGuestDomainAccessControl: remapped to ");
              DEBUG_INT(*(u32int*)ptEntry);
              DEBUG_NEWLINE();
#endif
            }
            else if (ptEntry->type == PAGE_TABLE)
            {
              // ignore for now?
#ifdef ADDRESSING_DEBUG
              DEBUG_STRING("changeGuestDomainAccessControl: remap AP for page table entry ");
              DEBUG_NEWLINE();
              DIE_NOW(context, "changeGuestDomainAccessControl unimplemented.");
#endif
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

