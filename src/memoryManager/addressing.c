#include "addressing.h"
#include "serial.h"
#include "pageTable.h"
#include "memoryConstants.h"
#include "mmu.h"
#include "memoryProtection.h"
#include "blockCache.h"
#include "memoryConstants.h"
#include "debug.h"

extern GCONTXT * getGuestContext(void); //from main.c
extern void setGuestPhysicalPt(GCONTXT* gc);

void initialiseVirtualAddressing()
{
  serial_putstring("Initializing Virtual Addressing.");
  descriptor* ptAddr = createHypervisorPageTable();

  serial_putstring(" Page Table @ 0x");
  serial_putint((u32int)ptAddr);
  serial_putstring("...");

  mmuInit();
  mmuInsertPt0(ptAddr); //Map Hypervisor PT into TTBR0
  mmuEnableVirtAddr();

  serial_putstring("done");
  serial_newline();
}

/* virtual machine startup, need to add a new guestPhysical to ReadPhysical address map */
/* NOT used in current deliverable */
void createVirtualMachineGPAtoRPA(GCONTXT* gc)
{
#ifdef ADDRESSING_DEBUG
  serial_putstring("createVirtualMachineGPAtoRPA: TODO createVirtualMachineGPAtoRPA (addressing.c)");
  serial_newline();
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
  serial_putstring("initialiseGuestShadowPageTable: new pt addr ");
  serial_putint(guestPtAddr);
  serial_newline();
#endif

  GCONTXT* context = getGuestContext();

  //This needs changing to, or similar
  //if(gc->decompressionDone)
  if(guestPtAddr == 0x80004000)
  {
#ifdef ADDRESSING_DEBUG
    serial_putstring("initialiseGuestShadowPageTable: TTBR0 Linux identity mapping bootstrap, ignoring.");
    serial_newline();
#endif
    return;
  }

  //This was really annoying to find, linux set the bottom bits for some reason?!
  //Perhaps to check that the table ptr has been updated and is not still the identity map?
  guestPtAddr = guestPtAddr &  0xFFFFC000;

#ifdef ADDRESSING_DEBUG
  serial_putstring("initialiseGuestShadowPageTable: Dumping guest page table @ 0x");
  serial_putint(guestPtAddr);
  serial_newline();

  descriptor* ptd = (descriptor*)guestPtAddr;
  dumpPageTable(ptd);

  serial_newline();
  serial_putstring("Guest pt dump finished");
  serial_newline();
#endif

  if(context->virtAddrEnabled)
  {
    // 1. explode block cache
    explodeCache(context->blockCache);
    
    // 2. create a new shadow page table. Mapping in hypervisor address space
    descriptor* newShadowPt = createGuestOSPageTable();
    serial_putstring("new shadow page table @ ");
    serial_putint((u32int)newShadowPt);
    serial_newline();
  
    // 3a: remove metadata about old sPT1
    removePT2Metadata();

    // 3b: map new page table address to a 1-2-1 virtual address 
    // (now we can safely tamper with sPT, as it will be discarded soon)
    sectionMapMemory(context->PT_shadow, (guestPtAddr & 0xFFF00000),
                 (guestPtAddr & 0xFFF00000)+(SECTION_SIZE-1), 
                 HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0b000);
    // 3c: copy new gPT1 entries to new sPT1
    copyPageTable((descriptor*)guestPtAddr, newShadowPt);
#ifdef ADDRESSING_DEBUG
    serial_putstring("initialiseGuestShadowPageTable: Copy PT done.");
    serial_newline();
    serial_putstring("initialiseGuestShadowPageTable: About to switch to sPT");
    serial_newline();
#endif
    
    // 4. anything in caches needs to be written back now
    dataBarrier();
    
    // 5. tell CP15 of this new base PT
    setTTBCR((u32int)newShadowPt);
    serial_putstring("initialiseGuestShadowPageTable: set new ttbcr to ");
    serial_putint((u32int)newShadowPt);
    serial_newline();
    
    // 6. clean tlb and cache entries
    clearTLB();
    clearCache();
   
    // 7. update guest context entries
    // 7a. update 1st level shadow page table pointer
    descriptor* oldShadowPt = context->PT_shadow; 
    context->PT_shadow = newShadowPt;
    // 7b. in this new 1st level sPT, find VA for 1st lvl gPT
    u32int ptGuestVirtual = findVAforPA(guestPtAddr);
    serial_putstring("initialiseGuestShadowPageTable: VA for guestPtAddr 0x");
    serial_putint(ptGuestVirtual);
    serial_newline();
    context->PT_os_real = (descriptor*)guestPtAddr;
    context->PT_os = (descriptor*)ptGuestVirtual;
    serial_putstring("initialiseGuestShadowPageTable: updated guest context");
    serial_newline();
  
    // 8. add protection to guest page table.  
    u32int guestPtVirtualEndAddr = ptGuestVirtual + PAGE_TABLE_SIZE - 1;
    //function ptr to the routine that handler gOS edits to its PT
    addProtection(ptGuestVirtual, guestPtVirtualEndAddr, &pageTableEdit, PRIV_RW_USR_RO);
    serial_putstring("initialiseGuestShadowPageTable: protected guest page table");
    serial_newline();
  
    // 9. clean old shadow page table
    invalidateSPT1(oldShadowPt);

    //now running with new shadow page tables
  }
  else
  {
#ifdef ADDRESSING_DEBUG
    serial_putstring("initialiseGuestShadowPageTable: set gPT ptr in gContext");
    serial_newline();
#endif
    //guest virtual addressing is not active, no need to spend time faulting on PT that the OS is going to add entries to before it activates
    context->PT_os = (descriptor*)guestPtAddr;
  }
}


void guestEnableVirtMem()
{
  GCONTXT* gc = getGuestContext();

  if(0 == gc->PT_os)
  {
#ifdef ADDRESSING_DEBUG
    serial_putstring("guestEnableVirtMem: No entry in gc. Must be identity mapping bootstrap, ignore hypervised. Continuing boot...");
    serial_newline();
#endif
    return;
  }

#ifdef ADDRESSING_DEBUG
  dumpGuestContext(gc);
  serial_putstring("guestEnableVirtMem: Dumping guest page table from addr in gc->PT_os @ 0x");
  serial_putint((u32int)gc->PT_os);
  serial_newline();

  dumpPageTable(gc->PT_os);

  serial_putstring("guestEnableVirtMem: PT dump done");
  serial_newline();
#endif

  if(gc->virtAddrEnabled)
  {
#ifdef ADDRESSING_DEBUG
    DIE_NOW(0, "guest virtual addressing is already enabled");
#endif
  }

#ifdef ADDRESSING_DEBUG
  serial_putstring("guestEnableVirtMem: Enabling guest virtual / shadow page tables");
  serial_newline();
#endif

  //create a new shadow page table. Mapping in hypervisor address space
  descriptor* sPT =  createGuestOSPageTable();
  gc->PT_shadow = sPT;

  //map all the guest OS pt entries into the shadow PT, using the GuestPhysical to ReadPhysical PT map
  copyPageTable(gc->PT_os, sPT);

#ifdef ADDRESSING_DEBUG
  serial_putstring("guestEnableVirtMem: Copy PT done. Dumping shadow PT");
  serial_newline();
  dumpPageTable(sPT);
  serial_putstring("guestEnableVirtMem: shadow PT dump done.");
  serial_newline();
  serial_putstring("guestEnableVirtMem: About to switch to sPT");
  serial_newline();
#endif


  /* Quick prototyping code for now. needs doing properly at some point */
  explodeCache(gc->blockCache);

  dataBarrier();  //anything in caches needs to be written back now
  setTTBCR((u32int)sPT); //swap shadow PT
  clearTLB(); //clean out all TLB entries - may have conflicting entries
  clearCache(); //just to make sure

#ifdef ADDRESSING_DEBUG
  serial_putstring("guestEnableVirtMem: Using sPT. Continuing");
  serial_newline();
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

  //function ptr to the routine that handler gOS edits to its PT
  addProtection(guestPtAddr, guestPtEndAddr, &pageTableEdit, PRIV_RW_USR_RO);
}

