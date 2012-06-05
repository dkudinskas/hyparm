#include "common/debug.h"
#include "common/linker.h"
#include "common/stddef.h"
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
  context->pageTables->hypervisor = (simpleEntry *)newLevelOnePageTable();

  setupPageTable(context, PT_TARGET_HYPERVISOR);

  DEBUG(MM_ADDRESSING, "initVirtualAddressing: new hypervisor page table %p" EOL, context->pageTables->hypervisor);

  mmuInit();
  mmuSetDomain(HYPERVISOR_ACCESS_DOMAIN, client);
  mmuSetDomain(GUEST_ACCESS_DOMAIN, client);
  mmuSetTTBR0(context->pageTables->hypervisor, 0);
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
      pageTablePtr = context->pageTables->hypervisor;
      break;
    }
    case PT_TARGET_GUEST_SHADOW_PRIVILEGED:
    {
      pageTablePtr = context->pageTables->shadowPriv;
      break;
    }
    case PT_TARGET_GUEST_SHADOW_UNPRIVILEGED:
      pageTablePtr = context->pageTables->shadowUser;
      break;
    default:
      DIE_NOW(context, "bad target");
  }

  // map in the hypervisor
  mapHypervisorMemory(pageTablePtr);

  if (target == PT_TARGET_HYPERVISOR)
  {
    // 1:1 Map the entire of physical memory
    mapRegion(pageTablePtr, MEMORY_START_ADDR, MEMORY_START_ADDR, HYPERVISOR_BEGIN_ADDRESS,
              GUEST_ACCESS_DOMAIN, GUEST_ACCESS_BITS, TRUE, FALSE, 0, FALSE);

    // clock manager
    mapSmallPage(pageTablePtr, BE_CLOCK_MANAGER, BE_CLOCK_MANAGER,
                 HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0b000, 1);
    mapSmallPage(pageTablePtr, BE_CLOCK_MANAGER + SMALL_PAGE_SIZE, BE_CLOCK_MANAGER + SMALL_PAGE_SIZE,
                 HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0b000, 1);

    // 32kHz synchronized timer
    mapSmallPage(pageTablePtr, BE_TIMER32K, BE_TIMER32K,
                 HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0b000, 1);
  }
  else
  {
    // no need to add other mappings for RAM addresses: if guest needs any,
    // we will on-demand shadow map them.

    // All connected GPIOs must be mapped here!
    mapSmallPage(pageTablePtr, BE_GPIO5, BE_GPIO5,
                 HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0, 1);
  }

  // interrupt controller
  mapSmallPage(pageTablePtr, BE_IRQ_CONTROLLER, BE_IRQ_CONTROLLER,
               HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0, 1);

  // gptimer1
  mapSmallPage(pageTablePtr, BE_GPTIMER1, BE_GPTIMER1,
               HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0, 1);

  // gptimer2
  mapSmallPage(pageTablePtr, BE_GPTIMER2, BE_GPTIMER2,
               HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0, 1);

  // MMC1 interface
  mapSmallPage(pageTablePtr, BE_MMCHS1, BE_MMCHS1,
               HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0, 1);

  //serial (UART3)
  mapSmallPage(pageTablePtr, BE_UART3, BE_UART3,
               HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0, 1);

#ifdef CONFIG_BLOCK_COPY
  if (target != PT_TARGET_GUEST_SHADOW_UNPRIVILEGED)
  {
    const u32int codeAddress = (u32int)context->translationCache.codeCache;
    const u32int spillAddress = (u32int)context->translationCache.spillPage;
    mapRegion(pageTablePtr, codeAddress, codeAddress, codeAddress + SMALL_PAGE_SIZE,
              GUEST_ACCESS_DOMAIN, PRIV_RW_USR_RO, FALSE, FALSE, 0, FALSE);
    mapRegion(pageTablePtr, spillAddress, spillAddress, spillAddress + SMALL_PAGE_SIZE,
              GUEST_ACCESS_DOMAIN, PRIV_RW_USR_RW, FALSE, FALSE, 0, TRUE);
  }
#endif
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
  DEBUG(MM_ADDRESSING, "guestEnableMMU: guest turning off virtual memory" EOL);

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
  
  mmuInvalidateUTLB();
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

  // invalidate the whole block cache
  clearTranslationCache(&context->translationCache);

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

  // invalidate the whole block cache
  clearTranslationCache(&context->translationCache);

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

  // invalidate the whole block cache
  clearTranslationCache(&gc->translationCache);

  mmuClearDataCache();
  mmuDataMemoryBarrier();

  //FIXME: Henri: Why should these structures be invalidated?
#ifndef CONFIG_GUEST_ANDROID
  invalidatePageTableInfo(gc);
#endif /* CONFIG_GUEST_ANDROID */

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
 * look through any shadowed entries we must update now
 **/
void changeGuestDACR(GCONTXT *context, u32int oldVal, u32int newVal)
{
  if (context->virtAddrEnabled)
  {
    simpleEntry* ttbrBackup = mmuGetTTBR0();
    mmuSetTTBR0(context->pageTables->hypervisor, 0x1FF);

    simpleEntry* gpt = context->pageTables->guestPhysical;

    // if changing DACR, may have to change access permission bits of shadowed entries
    // this requires us to force the guest into an appropriate mode
    u32int cpsrPriv = PSR_SVC_MODE;
    u32int cpsrUser = PSR_USR_MODE;
    u32int cpsrBackup = context->CPSR; 

    // loop through all page table entries
    u32int y = 0;
    for (y = 0; y < PT1_ENTRIES; y++)
    {
      context->CPSR = cpsrPriv;
      simpleEntry *shadowPriv = (simpleEntry *)&context->pageTables->shadowPriv[y];
      // only check guest domain if the entry is shadow mapped.
      if ((shadowPriv->type != FAULT) && (shadowPriv->type != RESERVED)
          && (shadowPriv->domain != HYPERVISOR_ACCESS_DOMAIN))
      {
        simpleEntry* guest = &(gpt[y]);
        // look for domains that had their configuration changed.
        if ( ((oldVal >> (guest->domain*2)) & 0x3) != 
             ((newVal >> (guest->domain*2)) & 0x3) )
        {
          DEBUG(MM_ADDRESSING, "changeGuestDACR: %x: sPTE %08x gPTE %08x needs AP bits remapped" EOL, y, *(u32int *)shadowPriv, *(u32int *)guest);
          if (guest->type == SECTION)
          {
            mapAPBitsSection(context, (sectionEntry*)guest, shadowPriv, (y << 20));
            DEBUG(MM_ADDRESSING, "changeGuestDACR: remapped to %08x" EOL, *(u32int*)shadowPriv);
          }
          else if (guest->type == PAGE_TABLE)
          {
            DEBUG(MM_ADDRESSING, "changeGuestDACR: remap AP for page table entry" EOL);
            mapAPBitsPageTable(context, (pageTableEntry*)guest, (pageTableEntry*)shadowPriv);
          }
        } // if DACR for PT entry domain changed ends 
      } // shadowUser


      context->CPSR = cpsrUser;
      simpleEntry* shadowUser = (simpleEntry*)&context->pageTables->shadowUser[y];
      // only check guest domain if the entry is shadow mapped.
      if ((shadowUser->type != FAULT) && (shadowUser->type != RESERVED)
          && (shadowUser->domain != HYPERVISOR_ACCESS_DOMAIN))
      {
        simpleEntry* guest = &(gpt[y]);
        // look for domains that had their configuration changed.
        if ( ((oldVal >> (guest->domain*2)) & 0x3) != 
             ((newVal >> (guest->domain*2)) & 0x3) )
        {
          DEBUG(MM_ADDRESSING, "changeGuestDACR: %x: sPTE %08x gPTE %08x needs AP bits remapped" EOL, y, *(u32int*)shadowPriv, *(u32int*)guest);
          if (guest->type == SECTION)
          {
            mapAPBitsSection(context, (sectionEntry*)guest, shadowUser, (y << 20));
            DEBUG(MM_ADDRESSING, "changeGuestDACR: remapped to %08x" EOL, *(u32int *)shadowUser);
          }
          else if (guest->type == PAGE_TABLE)
          {
            DEBUG(MM_ADDRESSING, "changeGuestDACR: remap AP for page table entry" EOL);
            mapAPBitsPageTable(context, (pageTableEntry*)guest, (pageTableEntry*)shadowUser);
          }
        } // if DACR for PT entry domain changed ends 
      } // shadowUser
    } // loop all PT1 entries
    context->CPSR = cpsrBackup; 

    mmuSetTTBR0(ttbrBackup, 0x100 | context->pageTables->contextID);
  }
}


void setExceptionVector(u32int guestMode)
{
#ifdef ADDRESSING_DEBUG
  printf("setExceptionVector: mode %02x\n", guestMode);
#endif
  switch (guestMode)
  {
    case PSR_USR_MODE:
    {
#ifdef ADDRESSING_DEBUG
      printf("setExceptionVector: USR, vector %08x\n", (u32int)&usrVector);
#endif
      mmuSetExceptionVector((u32int)&usrVector);
      break;
    }
    case PSR_IRQ_MODE:
    {
#ifdef ADDRESSING_DEBUG
      printf("setExceptionVector: IRQ, vector %08x\n", (u32int)&irqVector);
#endif
      mmuSetExceptionVector((u32int)&irqVector);
      break;
    }
    case PSR_SVC_MODE:
    {
#ifdef ADDRESSING_DEBUG
      printf("setExceptionVector: SVC, vector %08x\n", (u32int)&svcVector);
#endif
      mmuSetExceptionVector((u32int)&svcVector);
      break;
    }
    case PSR_ABT_MODE:
    {
#ifdef ADDRESSING_DEBUG
      printf("setExceptionVector: ABORT, vector %08x\n", (u32int)&abortVector);
#endif
      mmuSetExceptionVector((u32int)&abortVector);
      break;
    }
    case PSR_UND_MODE:
    {
#ifdef ADDRESSING_DEBUG
      printf("setExceptionVector: UNDEFINED, vector %08x\n", (u32int)&undVector);
#endif
      mmuSetExceptionVector((u32int)&undVector);
      break;
    }
//    case PSR_USR_MODE:
    case PSR_FIQ_MODE:
//    case PSR_IRQ_MODE:
//    case PSR_SVC_MODE:
    case PSR_MON_MODE:
//    case PSR_ABT_MODE:
//    case PSR_UND_MODE:
    case PSR_SYS_MODE:
    default:
    {
      printf("setExceptionVector: guestMode %03x\n", guestMode);
      DIE_NOW(0, "setExceptionVector: unimplemented\n");
    }
/*    {
#ifdef ADDRESSING_DEBUG
      printf("setExceptionVector: default, vector %08x\n", (u32int)&exceptionVectorBase);
#endif
      mmuSetExceptionVector((u32int)&exceptionVectorBase);
    }*/
  } // switch end
}
