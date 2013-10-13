#include "common/debug.h"
#include "common/linker.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "cpuArch/constants.h"

#include "drivers/beagle/memoryMap.h"

#include "memoryManager/addressing.h"
#include "memoryManager/memoryConstants.h"
#include "memoryManager/memoryProtection.h"
#include "memoryManager/mmu.h"
#include "memoryManager/pageTable.h"
#include "memoryManager/shadowMap.h"


typedef enum pageTableTarget
{
  PT_TARGET_HYPERVISOR,
  PT_TARGET_GUEST_SHADOW_PRIVILEGED,
  PT_TARGET_GUEST_SHADOW_UNPRIVILEGED
} PageTableTarget;


extern const u32int exceptionVectorBase;
extern const u32int abortVector;
extern const u32int svcVector;
extern const u32int irqVector;
extern const u32int usrVector;
extern const u32int undVector;


static void setupPageTable(GCONTXT *context, PageTableTarget target);


void initVirtualAddressing(GCONTXT *context)
{
  //alloc some space for our 1st Level page table
  context->hypervisorPageTable = (simpleEntry *)newLevelOnePageTable();

  setupPageTable(context, PT_TARGET_HYPERVISOR);

  DEBUG(MM_ADDRESSING, "initVirtualAddressing: new hypervisor page table %p" EOL, context->hypervisorPageTable);

  mmuInit();
  mmuSetDomain(HYPERVISOR_ACCESS_DOMAIN, client);
  mmuSetTTBR0(context->hypervisorPageTable, 0);
  mmuEnableVirtAddr();

  DEBUG(MM_ADDRESSING, "initVirtualAddressing: done" EOL);
}


static void setupPageTable(GCONTXT *context, PageTableTarget target)
{
  simpleEntry *pageTablePtr;
  switch (target)
  {
    case PT_TARGET_HYPERVISOR:
    {
      pageTablePtr = context->hypervisorPageTable;
      break;
    }
    case PT_TARGET_GUEST_SHADOW_PRIVILEGED:
    {
      pageTablePtr = context->pageTables->shadowPriv;
      break;
    }
    case PT_TARGET_GUEST_SHADOW_UNPRIVILEGED:
    {
      pageTablePtr = context->pageTables->shadowUser;
      break;
    }
    default:
    {
      DIE_NOW(context, "bad target");
    }
  }

  // map in the hypervisor
  mapHypervisorMemory(pageTablePtr);

  if (target == PT_TARGET_HYPERVISOR)
  {
    // 1:1 Map the entire of physical memory
    mapRegion(pageTablePtr, MEMORY_START_ADDR, MEMORY_START_ADDR, HYPERVISOR_BEGIN_ADDRESS,
              HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, TRUE, FALSE, 0, FALSE);

    // 32kHz synchronized timer
    mapSmallPage(pageTablePtr, BE_TIMER32K, BE_TIMER32K,
                 HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0b000, 1);

    // map in static RAM 1-2-1
    const u32int staticRamStart = 0x40200000;
    mapSection(pageTablePtr, staticRamStart, staticRamStart, HYPERVISOR_ACCESS_DOMAIN,
               HYPERVISOR_ACCESS_BITS, 0, 0, 0, 1);

  }
#ifndef CONFIG_HW_PASSTHROUGH
  else
  {
    // no need to add other mappings for RAM addresses: if guest needs any,
    // we will on-demand shadow map them.

    // All connected GPIOs must be mapped here!
    mapSmallPage(pageTablePtr, BE_GPIO5, BE_GPIO5,
                 HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0, 1);
  }

  // clock manager
  mapSmallPage(pageTablePtr, BE_CLOCK_MANAGER, BE_CLOCK_MANAGER,
               HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0b000, 1);
  mapSmallPage(pageTablePtr, BE_CLOCK_MANAGER + SMALL_PAGE_SIZE, BE_CLOCK_MANAGER + SMALL_PAGE_SIZE,
               HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0b000, 1);

  // interrupt controller
  mapSmallPage(pageTablePtr, BE_IRQ_CONTROLLER, BE_IRQ_CONTROLLER,
               HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0, 1);

  // gptimer1
  mapSmallPage(pageTablePtr, BE_GPTIMER1, BE_GPTIMER1,
               HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0, 1);

  // MMC1 interface
  mapSmallPage(pageTablePtr, BE_MMCHS1, BE_MMCHS1,
               HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0, 1);
#endif

#ifdef CONFIG_PROFILER
  // gptimer3
  mapSmallPage(pageTablePtr, BE_GPTIMER3, BE_GPTIMER3,
               HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0, 1);
#endif

  //serial (UART3)
  mapSmallPage(pageTablePtr, BE_UART3, BE_UART3,
               HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0, 1);

  if (target != PT_TARGET_GUEST_SHADOW_UNPRIVILEGED)
  {
    mapRegion(pageTablePtr, RAM_CODE_CACHE_POOL_BEGIN, RAM_CODE_CACHE_POOL_BEGIN, RAM_CODE_CACHE_POOL_END,
              HYPERVISOR_ACCESS_DOMAIN, PRIV_RW_USR_RO, TRUE, FALSE, 0, FALSE);
  }
}

/**
 * we intercept the guest setting Translation Table Base Register
 * this address is PHYSICAL!
 **/
void guestSetPageTableBase(GCONTXT *gc, u32int ttbr)
{
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
void guestEnableMMU(GCONTXT *context)
{
  DEBUG(MM_ADDRESSING, "guestEnableMMU: guest turning on virtual memory" EOL);

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
  // if initializing vmem, really must clean and invalidate the TLB!
  mmuInvalidateUTLB();
}

/**
 * guest is turning OFF the MMU.
 * what?
 **/
void guestDisableMMU(GCONTXT *context)
{
  DEBUG(MM_ADDRESSING, "guestDisableMMU: guest turning off virtual memory" EOL);

  mmuDisableVirtAddr();

  // reset all the shadow stuff
  context->pageTables->guestPhysical = 0;
  context->pageTables->guestVirtual = 0;
  context->pageTables->shadowPriv = 0;
  context->pageTables->shadowUser = 0;
  context->pageTables->shadowActive = 0;
  context->pageTables->contextID = 0;

  invalidatePageTableInfo(context);
  context->pageTables->sptInfo = 0;
  context->pageTables->gptInfo = 0;
  context->virtAddrEnabled = FALSE;
  
  // reset hypervisor page table
  // STARFIX: remove memsets!
  memset((void*)context->hypervisorPageTable, 0, PT1_SIZE);

  // and set it up again
  setupPageTable(context, PT_TARGET_HYPERVISOR);

  DEBUG(MM_ADDRESSING, "guestDisableMMU: hypervisor page table %p reset" EOL, context->hypervisorPageTable);

  mmuInit();
  mmuSetDomain(HYPERVISOR_ACCESS_DOMAIN, client);
  mmuSetTTBR0(context->hypervisorPageTable, 0);

  // turn vmem back on
  mmuEnableVirtAddr();
  
  // get rid of all translations
  clearTranslationsAll(context->translationStore);

  // sync everything to make sure.
  mmuDataMemoryBarrier();
  mmuInvalidateUTLB();
  mmuClearInstructionCache();
  DEBUG(MM_ADDRESSING, "guestDisableMMU: done" EOL);
}


void guestSetContextID(GCONTXT *context, u32int contextid)
{
  DEBUG(MM_ADDRESSING, "guestSetContextID: value %x" EOL, contextid);

  context->pageTables->contextID = (contextid & 0xFF);
}


/**
 * switching guest addressing from privileged to user mode
 **/
void privToUserAddressing(GCONTXT *context)
{
  DEBUG(MM_ADDRESSING, "privToUserAddressing: set shadowActive to %p" EOL, context->pageTables->shadowUser);

  context->pageTables->shadowActive = context->pageTables->shadowUser;
  
  if (context->pageTables->guestVirtual != 0)
  {
    // shadow map gpt1 address if not shadow mapped yet
    simpleEntry* entry = getEntryFirst(context->pageTables->shadowActive, (u32int)context->pageTables->guestVirtual);
    if (entry->type == FAULT)
    {
      bool mapped = shadowMap(context, (u32int)context->pageTables->guestVirtual);
      if (!mapped)
      {
        DIE_NOW(context, "privToUserAddressing: couldn't shadow map guest page table." EOL);
      }
    }
  }

  //anything in caches needs to be written back now
  mmuDataMemoryBarrier();

  // set translation table base register in the physical MMU!
  mmuSetTTBR0(context->pageTables->shadowActive, (0x100 | context->pageTables->contextID));

  // clean out all TLB entries - may have conflicting entries
  mmuInvalidateUTLB();

  //just to make sure
  mmuInstructionSync();
}

/**
 * switching guest addressing from user to privileged mode
 **/
void userToPrivAddressing(GCONTXT *context)
{
  DEBUG(MM_ADDRESSING, "userToPrivAddressing: set shadowActive to %p" EOL, context->pageTables->shadowPriv);

  context->pageTables->shadowActive = context->pageTables->shadowPriv;
  if (context->pageTables->guestVirtual != 0)
  {
    // shadow map gpt1 address if not shadow mapped yet
    simpleEntry* entry = getEntryFirst(context->pageTables->shadowActive, (u32int)context->pageTables->guestVirtual);
    if (entry->type == FAULT)
    {
      bool mapped = shadowMap(context, (u32int)context->pageTables->guestVirtual);
      if (!mapped)
      {
        DIE_NOW(context, "privToUserAddressing: couldn't shadow map guest page table." EOL);
      }
    }
  }

  //anything in caches needs to be written back now
  mmuDataMemoryBarrier();

  // set translation table base register in the physical MMU!
  mmuSetTTBR0(context->pageTables->shadowActive, (0x100 | context->pageTables->contextID));

  // clean out all TLB entries - may have conflicting entries
  mmuInvalidateUTLB();

  //just to make sure
  mmuInstructionSync();
}

/**
 * allocate and set up double shadow page tables for the guest
 **/
void initialiseShadowPageTables(GCONTXT *gc)
{
  DEBUG(MM_ADDRESSING, "initialiseShadowPageTables: create double-shadows!" EOL);

  mmuClearDataCache();
  mmuDataMemoryBarrier();

  invalidatePageTableInfo(gc);

  DEBUG(MM_ADDRESSING, "initialiseShadowPageTables: invalidatePageTableInfo() done." EOL);

  // allocate two shadow page tables and prepare the minimum for operation
  gc->pageTables->shadowPriv = (simpleEntry *)newLevelOnePageTable();
  gc->pageTables->shadowUser = (simpleEntry *)newLevelOnePageTable();
  setupPageTable(gc, PT_TARGET_GUEST_SHADOW_PRIVILEGED);
  setupPageTable(gc, PT_TARGET_GUEST_SHADOW_UNPRIVILEGED);
  DEBUG(MM_ADDRESSING, "initialiseShadowPageTables: allocated spt priv %p; spt usr %p" EOL,
        gc->pageTables->shadowPriv, gc->pageTables->shadowUser);

  // which shadow PT will be in use depends on guest mode
  gc->pageTables->shadowActive = isGuestInPrivMode(gc) ?
     gc->pageTables->shadowPriv : gc->pageTables->shadowUser;

  // mark guest virtual addressing as now enabled
  gc->virtAddrEnabled = TRUE;

  DEBUG(MM_ADDRESSING, "initialiseShadowPageTables: gPT phys %p virt %p" EOL,
            gc->pageTables->guestPhysical, gc->pageTables->guestVirtual);

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
 * guest has modified domain access control register.
 * make sure it isnt twiddling with our reserved domain
 **/
void changeGuestDACR(GCONTXT *context, DACR old, DACR new)
{
  if (old.fields.dom15 != new.fields.dom15)
  {
    DIE_NOW(0, "changeGuestDACR: changing hypervisor domain bits\n");
  }

  if (new.fields.dom0 == manager) {new.fields.dom0 = client; }
  if (new.fields.dom1 == manager) {new.fields.dom1 = client; }
  if (new.fields.dom2 == manager) {new.fields.dom2 = client; }
  if (new.fields.dom3 == manager) {new.fields.dom3 = client; }
  if (new.fields.dom4 == manager) {new.fields.dom4 = client; }
  if (new.fields.dom5 == manager) {new.fields.dom5 = client; }
  if (new.fields.dom6 == manager) {new.fields.dom6 = client; }
  if (new.fields.dom7 == manager) {new.fields.dom7 = client; }
  if (new.fields.dom8 == manager) {new.fields.dom8 = client; }
  if (new.fields.dom9 == manager) {new.fields.dom9 = client; }
  if (new.fields.dom10 == manager) {new.fields.dom10 = client; }
  if (new.fields.dom11 == manager) {new.fields.dom11 = client; }
  if (new.fields.dom12 == manager) {new.fields.dom12 = client; }
  if (new.fields.dom13 == manager) {new.fields.dom13 = client; }
  if (new.fields.dom14 == manager) {new.fields.dom14 = client; }

  // set hypervisor domain bits to 'client'
  new.fields.dom15 = client;
  __asm__ __volatile__("mcr p15, 0, %0, c3, c0, 0"
  :
  : "r"(new.value)
  : "memory"
     );

  return;
}


void setExceptionVector(u32int guestMode)
{
  DEBUG(MM_ADDRESSING, "setExceptionVector: mode %02x" EOL, guestMode);
  switch (guestMode)
  {
    case USR_MODE:
    {
      DEBUG(MM_ADDRESSING, "setExceptionVector: USR, vector %08x" EOL, (u32int)&usrVector);
      mmuSetExceptionVector((u32int)&usrVector);
      break;
    }
    case IRQ_MODE:
    {
      DEBUG(MM_ADDRESSING, "setExceptionVector: IRQ, vector %08x" EOL, (u32int)&irqVector);
      mmuSetExceptionVector((u32int)&irqVector);
      break;
    }
    case SVC_MODE:
    {
      DEBUG(MM_ADDRESSING, "setExceptionVector: SVC, vector %08x" EOL, (u32int)&svcVector);
      mmuSetExceptionVector((u32int)&svcVector);
      break;
    }
    case ABT_MODE:
    {
      DEBUG(MM_ADDRESSING, "setExceptionVector: ABORT, vector %08x" EOL, (u32int)&abortVector);
      mmuSetExceptionVector((u32int)&abortVector);
      break;
    }
    case UND_MODE:
    {
      DEBUG(MM_ADDRESSING, "setExceptionVector: UNDEFINED, vector %08x" EOL, (u32int)&undVector);
      mmuSetExceptionVector((u32int)&undVector);
      break;
    }
    case FIQ_MODE:
    case MON_MODE:
    case SYS_MODE:
    default:
    {
      printf("setExceptionVector: guestMode %03x\n", guestMode);
      DIE_NOW(0, "setExceptionVector: unimplemented\n");
    }
  } // switch end
}
