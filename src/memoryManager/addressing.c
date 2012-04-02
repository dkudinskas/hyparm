#include "common/debug.h"
#include "common/markers.h"
#include "common/stddef.h"
#include "common/string.h"

#include "cpuArch/constants.h"

#include "drivers/beagle/memoryMap.h"

#include "guestManager/blockCache.h"

#include "memoryManager/addressing.h"
#include "memoryManager/memoryConstants.h"
#include "memoryManager/memoryProtection.h"
#include "memoryManager/mmu.h"
#include "memoryManager/pageTable.h"
#include "memoryManager/shadowMap.h"


void initVirtualAddressing(GCONTXT *context)
{
  //alloc some space for our 1st Level page table
  context->pageTables->hypervisor = (simpleEntry *)newLevelOnePageTable();

  setupHypervisorPageTable(context->pageTables->hypervisor);

  DEBUG(MM_ADDRESSING, "initVirtualAddressing: new hypervisor page table %p" EOL, context->pageTables->hypervisor);

  mmuInit();
  mmuSetTTBR0(context->pageTables->hypervisor, 0);
  mmuEnableVirtAddr();

  DEBUG(MM_ADDRESSING, "initVirtualAddressing: done" EOL);
}


void setupHypervisorPageTable(simpleEntry *pageTablePtr)
{
  DEBUG(MM_ADDRESSING, "setupHypervisorPageTable: new PT at %p" EOL, pageTablePtr);

  memset(pageTablePtr, 0, PT1_SIZE);

  //map in the hypervisor
  mapHypervisorMemory(pageTablePtr);

  // 1:1 Map the entire of physical memory
  u32int hypervisorStart = HYPERVISOR_IMAGE_START_ADDRESS;
  u32int memStart = MEMORY_START_ADDR;
  while (memStart < hypervisorStart) 
  {
    mapSection(pageTablePtr, memStart, memStart, 
               GUEST_ACCESS_DOMAIN, GUEST_ACCESS_BITS, 1, 0, 0b000);
    memStart += SECTION_SIZE;
  }

  //set the domain access control for the hypervisor and guest domains
  mmuSetDomain(HYPERVISOR_ACCESS_DOMAIN, client);
  mmuSetDomain(GUEST_ACCESS_DOMAIN, client);

  //serial (UART3)
  mapSmallPage(pageTablePtr, BE_UART3, BE_UART3,
               HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0b000, 0);

  // clock manager
  mapSmallPage(pageTablePtr, BE_CLOCK_MANAGER, BE_CLOCK_MANAGER,
               HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0b000, 0);
  mapSmallPage(pageTablePtr, BE_CLOCK_MANAGER + SMALL_PAGE_SIZE, BE_CLOCK_MANAGER + SMALL_PAGE_SIZE,
               HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0b000, 0);

  // interrupt controller
  mapSmallPage(pageTablePtr, BE_IRQ_CONTROLLER, BE_IRQ_CONTROLLER,
               HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0b000, 0);

  // gptimer1
  mapSmallPage(pageTablePtr, BE_GPTIMER1, BE_GPTIMER1,
               HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0b000, 0);

  // gptimer2
  mapSmallPage(pageTablePtr, BE_GPTIMER2, BE_GPTIMER2,
               HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0b000, 0);

  // 32kHz synchronized timer
  mapSmallPage(pageTablePtr, BE_TIMER32K, BE_TIMER32K,
               HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0b000, 0);

  // MMC1 interface
  mapSmallPage(pageTablePtr, BE_MMCHS1, BE_MMCHS1,
               HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0b000, 0);

  //add section mapping for 0x14000 (base exception vectors)
  const u32int exceptionHandlerAddr = 0x14000;
  mapSmallPage(pageTablePtr, exceptionHandlerAddr, exceptionHandlerAddr,
                 HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0b000, 0);

  // We will want to use the exception handler remap feature
  // to put the page tables in the 0xffff0000 address space later
  const u32int excHdlrSramStart = 0x4020ffd0;
  mapSmallPage(pageTablePtr, excHdlrSramStart, excHdlrSramStart,
                 HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0b000, 0);

  DEBUG(MM_ADDRESSING, "setupHypervisorPageTable: ... done" EOL);
}


void setupShadowPageTable(simpleEntry* pageTablePtr)
{
  memset(pageTablePtr, 0, PT1_SIZE);

  //map in the hypervisor
  mapHypervisorMemory(pageTablePtr);

  // no need to add other mappings for RAM addresses: if guest needs any,
  // we will on-demand shadow map them.

  //serial (UART3)
  mapSmallPage(pageTablePtr, BE_UART3, BE_UART3,
               HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0b000, 0);

  //add section mapping for 0x14000 (base exception vectors)
  const u32int exceptionHandlerAddr = 0x14000;
  mapSmallPage(pageTablePtr, exceptionHandlerAddr, exceptionHandlerAddr,
                 HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0b000, 0);

  // We will want to use the exception handler remap feature
  // to put the page tables in the 0xffff0000 address space later
  const u32int excHdlrSramStart = 0x4020ffd0;
  mapSmallPage(pageTablePtr, excHdlrSramStart, excHdlrSramStart,
                 HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0b000, 0);

  // interrupt controller
  mapSmallPage(pageTablePtr, BE_IRQ_CONTROLLER, BE_IRQ_CONTROLLER,
               HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0b000, 0);

  // gptimer1
  mapSmallPage(pageTablePtr, BE_GPTIMER1, BE_GPTIMER1,
               HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0b000, 0);

  // gptimer2
  mapSmallPage(pageTablePtr, BE_GPTIMER2, BE_GPTIMER2,
               HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0b000, 0);

  // MMC1 interface
  mapSmallPage(pageTablePtr, BE_MMCHS1, BE_MMCHS1,
               HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0b000, 0);

  // All connected GPIOs must be mapped here!
  mapSmallPage(pageTablePtr, BE_GPIO5, BE_GPIO5,
               HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0, 0);
}


/**
 * we intercept the guest setting Translation Table Base Register
 * this address is PHYSICAL!
 **/
void guestSetPageTableBase(u32int ttbr)
{
  GCONTXT *gc = getGuestContext();

  DEBUG(MM_ADDRESSING, "guestSetPageTableBase: ttbr %#.8x @ pc %#.8x" EOL, ttbr, gc->R15);

  gc->pageTables->guestPhysical = (simpleEntry *)ttbr;
  gc->pageTables->guestVirtual = NULL;

  if (gc->virtAddrEnabled)
  {
    initialiseShadowPageTables(gc);
  }
}


/**
 * guest is turning on the MMU. this means, virtual memory is being turned on!
 * lots of work to do!
 **/
void guestEnableMMU()
{
  DEBUG(MM_ADDRESSING, "guestEnableMMU: guest turning on virtual memory" EOL);

  GCONTXT *context = getGuestContext();

  if (context->pageTables->guestPhysical == 0)
  {
    DIE_NOW(context, "guestEnableMMU: guest TTBR is not set! Guest cannot enable virtual memory!" EOL);
  }
  if (context->virtAddrEnabled)
  {
    DIE_NOW(context, "guestEnableMMU: guest virtual addressing is already enabled, mustn't happen!" EOL);
  }
  
  // initialise double-shadow page tables now
  initialiseShadowPageTables(context);
  // if initializing vmem, realy must clean and invalidate the TLB!
  mmuInvalidateUTLB();
}


/**
 * guest is turning OFF the MMU.
 * what?
 **/
void guestDisableMMU()
{
  DIE_NOW(NULL, "guestDisableMMU: unimplemented.");
}


void guestSetContextID(u32int contextid)
{
  DEBUG(MM_ADDRESSING, "guestSetContextID: value %x" EOL, contextid);

  GCONTXT* context = getGuestContext();
  context->pageTables->contextID = (contextid & 0xFF);
}

/**
 * switching guest addressing from privileged to user mode
 **/
void privToUserAddressing()
{
  GCONTXT *gc = getGuestContext();

  DEBUG(MM_ADDRESSING, "privToUserAddressing: set shadowActive to %p" EOL, gc->pageTables->shadowUser);

  // invalidate the whole block cache
  clearBlockCache(gc->blockCache);

  gc->pageTables->shadowActive = gc->pageTables->shadowUser;
  
  if (gc->pageTables->guestVirtual != 0)
  {
    // shadow map gpt1 address if not shadow mapped yet
    simpleEntry* entry = getEntryFirst(gc->pageTables->shadowActive, (u32int)gc->pageTables->guestVirtual);
    if (entry->type == FAULT)
    {
      bool mapped = shadowMap((u32int)gc->pageTables->guestVirtual);
      if (!mapped)
      {
        DIE_NOW(gc, "privToUserAddressing: couldn't shadow map guest page table." EOL);
      }
    }
  }

  //anything in caches needs to be written back now
  mmuDataMemoryBarrier();

  // set translation table base register in the physical MMU!
  mmuSetTTBR0(gc->pageTables->shadowActive, (0x100 | gc->pageTables->contextID));

  // clean out all TLB entries - may have conflicting entries
  mmuInvalidateUTLB();

  //just to make sure
  mmuInstructionSync();
}


/**
 * switching guest addressing from user to privileged mode
 **/
void userToPrivAddressing()
{
  GCONTXT* gc = getGuestContext();

  DEBUG(MM_ADDRESSING, "userToPrivAddressing: set shadowActive to %p" EOL, gc->pageTables->shadowPriv);

  // invalidate the whole block cache
  clearBlockCache(gc->blockCache);

  gc->pageTables->shadowActive = gc->pageTables->shadowPriv;
  if (gc->pageTables->guestVirtual != 0)
  {
    // shadow map gpt1 address if not shadow mapped yet
    simpleEntry* entry = getEntryFirst(gc->pageTables->shadowActive, (u32int)gc->pageTables->guestVirtual);
    if (entry->type == FAULT)
    {
      bool mapped = shadowMap((u32int)gc->pageTables->guestVirtual);
      if (!mapped)
      {
        DIE_NOW(gc, "privToUserAddressing: couldn't shadow map guest page table." EOL);
      }
    }
  }

  //anything in caches needs to be written back now
  mmuDataMemoryBarrier();

  // set translation table base register in the physical MMU!
  mmuSetTTBR0(gc->pageTables->shadowActive, (0x100 | gc->pageTables->contextID) );

  // clean out all TLB entries - may have conflicting entries
  mmuInvalidateUTLB();

  //just to make sure
  mmuInstructionSync();
}


/**
 * allocate and set up double shadow page tables for the guest
 **/
void initialiseShadowPageTables(GCONTXT* gc)
{
  DEBUG(MM_ADDRESSING, "initialiseShadowPageTables: create double-shadows!" EOL);
  mmuClearDataCache();
  mmuDataMemoryBarrier();

  invalidatePageTableInfo();
  DEBUG(MM_ADDRESSING, "initialiseShadowPageTables: invalidatePageTableInfo() done." EOL);

  // allocate two shadow page tables and prepare the minimum for operation
  gc->pageTables->shadowPriv = (simpleEntry*)newLevelOnePageTable();
  gc->pageTables->shadowUser = (simpleEntry*)newLevelOnePageTable();
  setupShadowPageTable(gc->pageTables->shadowPriv);
  setupShadowPageTable(gc->pageTables->shadowUser);
  DEBUG(MM_ADDRESSING, "initialiseShadowPageTables: allocated spt priv %p; spt usr %p" EOL,
        gc->pageTables->shadowPriv, gc->pageTables->shadowUser);

  // which shadow PT will be in use depends on guest mode
  gc->pageTables->shadowActive = isGuestInPrivMode(gc) ?
     gc->pageTables->shadowPriv : gc->pageTables->shadowUser;

  // mark guest virtual addressing as now enabled
  gc->virtAddrEnabled = TRUE;

  DEBUG(MM_ADDRESSING, "initialiseShadowPageTables: gPT phys %p virt %p" EOL,
            gc->pageTables->guestPhysical, gc->pageTables->guestVirtual);

  // invalidate the whole block cache
  clearBlockCache(gc->blockCache);

  //anything in caches needs to be written back now
  mmuDataMemoryBarrier();

  // set translation table base register in the physical MMU!
//  mmuSetTTBR0(gc->pageTables->shadowActive, (0x100 | (gc->pageTables->contextID * 2)) );
  mmuSetTTBR0(gc->pageTables->shadowActive, (0x100 | gc->pageTables->contextID) );

  // clean out all TLB entries - may have conflicting entries
  mmuInvalidateUTLB();

  //just to make sure
  mmuInstructionSync();
}


/**
 * guest has modified domain access control register.
 * look through any shadowed entries we must update now
 **/
void changeGuestDACR(u32int oldVal, u32int newVal)
{
  GCONTXT *context = getGuestContext();
  if (context->virtAddrEnabled)
  {
    u32int backupEntry = 0;
    bool backedUp = FALSE;
    simpleEntry* tempFirst = 0;
    simpleEntry* gpt = 0;
    if (context->pageTables->guestVirtual == 0)
    {
      DEBUG(MM_ADDRESSING, "changeGuestDACR: guestVirtual PT not set. hack a 1-2-1 of %p" EOL,
            context->pageTables->guestPhysical);
      tempFirst = getEntryFirst(context->pageTables->shadowActive, (u32int)context->pageTables->guestPhysical);
      backupEntry = *(u32int*)tempFirst;
      DEBUG(MM_ADDRESSING, "changeGuestDACR: backed up entry %08x @ %p" EOL, backupEntry, tempFirst);
      mapSection(context->pageTables->shadowActive, (u32int)context->pageTables->guestPhysical, 
                (u32int)context->pageTables->guestPhysical, HYPERVISOR_ACCESS_DOMAIN,
                HYPERVISOR_ACCESS_BITS, TRUE, FALSE, 0);
      mmuInvalidateUTLBbyMVA((u32int)context->pageTables->guestPhysical);
      gpt = context->pageTables->guestPhysical;
      DEBUG(MM_ADDRESSING, "changeGuestDACR: gpt now set to %p" EOL, gpt);
      backedUp = TRUE;
    }
    else
    {
      gpt = context->pageTables->guestVirtual;
    }

    // if changing DACR, may have to change access permission bits of shadowed entries
    // this requires us to force the guest into an appropriate mode
    u32int cpsrPriv = PSR_SVC_MODE;
    u32int cpsrUser = PSR_USR_MODE;
    u32int cpsrBackup = context->CPSR; 

    // for every entry changed, loop through all page table entries
    u32int y = 0;
    for (y = 0; y < PT1_ENTRIES; y++)
    {
      context->CPSR = cpsrPriv;
      simpleEntry* shadowPriv = (simpleEntry*)&context->pageTables->shadowPriv[y];
      // only check guest domain if the entry is shadow mapped.
      if ((shadowPriv->type != FAULT) && (shadowPriv->type != RESERVED))
      {
        simpleEntry* guest = &(gpt[y]);
        // look for domains that had their configuration changed.
        if ( ((oldVal >> (guest->domain*2)) & 0x3) != 
             ((newVal >> (guest->domain*2)) & 0x3) )
        {
          DEBUG(MM_ADDRESSING, "changeGuestDACR: %x: sPTE %08x gPTE %08x needs AP bits remapped" EOL, y, *(u32int *)shadowPriv, *(u32int *)guest);
          if (guest->type == SECTION)
          {
            mapAPBitsSection((sectionEntry*)guest, shadowPriv, (y << 20));
            DEBUG(MM_ADDRESSING, "changeGuestDACR: remapped to %08x" EOL, *(u32int*)shadowPriv);
          }
          else if (guest->type == PAGE_TABLE)
          {
            DEBUG(MM_ADDRESSING, "changeGuestDACR: remap AP for page table entry" EOL);
            mapAPBitsPageTable((pageTableEntry*)guest, (pageTableEntry*)shadowPriv);
          }
        } // if DACR for PT entry domain changed ends 
      } // shadowUser


      context->CPSR = cpsrUser;
      simpleEntry* shadowUser = (simpleEntry*)&context->pageTables->shadowUser[y];
      // only check guest domain if the entry is shadow mapped.
      if ((shadowUser->type != FAULT) && (shadowUser->type != RESERVED))
      {
        simpleEntry* guest = &(gpt[y]);
        // look for domains that had their configuration changed.
        if ( ((oldVal >> (guest->domain*2)) & 0x3) != 
             ((newVal >> (guest->domain*2)) & 0x3) )
        {
          DEBUG(MM_ADDRESSING, "changeGuestDACR: %x: sPTE %08x gPTE %08x needs AP bits remapped" EOL, y, *(u32int*)shadowPriv, *(u32int*)guest);
          if (guest->type == SECTION)
          {
            mapAPBitsSection((sectionEntry*)guest, shadowUser, (y << 20));
            DEBUG(MM_ADDRESSING, "changeGuestDACR: remapped to %08x" EOL, *(u32int *)shadowUser);
          }
          else if (guest->type == PAGE_TABLE)
          {
            DEBUG(MM_ADDRESSING, "changeGuestDACR: remap AP for page table entry" EOL);
            mapAPBitsPageTable((pageTableEntry*)guest, (pageTableEntry*)shadowUser);
          }
        } // if DACR for PT entry domain changed ends 
      } // shadowUser
    } // loop all PT1 entries
    context->CPSR = cpsrBackup; 

    // exception case: we backed up the entry, but shadowed that exact entry. 
    // dont want to restore old entry then!!
    if (backedUp)
    {
      // if we dont have gPT1 VA we must have backed up the lvl1 entry. restore now
      *(u32int*)tempFirst = backupEntry;
      mmuInvalidateUTLBbyMVA((u32int)context->pageTables->guestPhysical);
      DEBUG(MM_ADDRESSING, "shadowMap: restore backed up entry %08x @ %p" EOL, backupEntry, tempFirst);
    }
    mmuInvalidateUTLB();
  }
}

