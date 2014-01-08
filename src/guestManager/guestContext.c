#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "cpuArch/constants.h"

#include "guestManager/guestContext.h"

#include "memoryManager/addressing.h"

#include "perf/contextSwitchCounters.h"

#include "vm/omap35xx/sdram.h"


#ifdef CONFIG_STATS
extern u32int timerTotalSvc;
extern u32int timerNumberSvc;
extern u32int timerTotalDabt;
extern u32int timerNumberDabt;
extern u32int timerTotalIrq;
extern u32int timerNumberIrq;

u32int getPerfCounter1(void);
u32int getPerfCounter2(void);
u32int getCycleCount(void);
#endif



GCONTXT *createGuestContext(void)
{
  // Allocate guest context
  GCONTXT *context = (GCONTXT *)calloc(1, sizeof(GCONTXT));
  if (context == 0)
  {
    DIE_NOW(NULL, "Failed to allocate guest context.");
  }
  DEBUG(GUEST_CONTEXT, "createGuestContext: @ %p; initialising..." EOL, context);

  // Set initial values
  context->CPSR.bits.F = 1;
  context->CPSR.bits.I = 1;
  context->CPSR.bits.mode = SVC_MODE;

  // Initialise coprocessor register bank
  context->coprocRegBank = createCRB();
  if (context->coprocRegBank == NULL)
  {
    DIE_NOW(context, "Failed to allocate coprocessor register bank.");
  }
  DEBUG(GUEST_CONTEXT, "createGuestContext: coprocessor register bank @ %p" EOL,
      context->coprocRegBank);

  // Initialise translator
  context->translationStore = (TranslationStore*)malloc(sizeof(TranslationStore));
  if (context->translationStore == NULL)
  {
    DIE_NOW(context, "Failed to allocate translation store");
  }
  DEBUG(GUEST_CONTEXT, "createGuestContext: translation store @ %p" EOL, context->translationStore);
  initialiseTranslationStore(context->translationStore);

  // virtual machine page table structs
  context->pageTables = (pageTablesVM *)calloc(1, sizeof(pageTablesVM));
  if (context->pageTables == NULL)
  {
    DIE_NOW(context, "Failed to allocate page tables struct");
  }
  DEBUG(GUEST_CONTEXT, "createGuestContext: page tables @ %p" EOL, context->pageTables);

  // virtual machine struct
  DEBUG(GUEST_CONTEXT, "createGuestContext: virtual machine @ %p" EOL, &context->vm);

  // Initialise virtual hardware devices
  context->hardwareLibrary = createHardwareLibrary(context);
  if (context->hardwareLibrary == NULL)
  {
    DIE_NOW(context, "Hardware library initialisation failed.");
  }

#ifdef CONFIG_GUEST_CONTEXT_BLOCK_TRACE
  // Print the address of the block trace, it may come in handy when debugging...
  DEBUG(GUEST_CONTEXT, "allocateGuestContext: block trace @ %p" EOL, &(context->blockTrace));
#endif

  // Execution bitmaps
  context->execBitmap = (u8int*)malloc(SIZE_BITMAP1);
  if (context->execBitmap == NULL)
  {
    DIE_NOW(context, "Failed to allocate page tables struct");
  }
  // STARFIX: remove most memsets and put one memset in naive malloc. memory isn't reused.
  memset((void*)context->execBitmap, 0, SIZE_BITMAP1);
  DEBUG(GUEST_CONTEXT, "createGuestContext: execBitmap @ %p size %x" EOL, context->execBitmap, SIZE_BITMAP1);

#ifdef CONFIG_STATS
  timerTotalSvc = 0;
  timerNumberSvc = 0;
  timerTotalDabt = 0;
  timerNumberDabt = 0;
#endif
  return context;
}


void dumpGuestContext(const GCONTXT *context)
{
  printf("====== DUMP GUEST CONTEXT @ %p ==============" EOL, context);

  const char *modeString;
  const u32int *r8 = &(context->R8);
  const u32int *r13 = NULL;
  const u32int *spsr = NULL;
  switch (context->CPSR.bits.mode)
  {
    case USR_MODE:
    case SYS_MODE:
    {
      modeString = "USR";
      r13 = &(context->R13_USR);
      spsr = 0;
      break;
    }
    case FIQ_MODE:
    {
      modeString = "FIQ";
      r8 = &(context->R8_FIQ);
      r13 = &(context->R13_FIQ);
      spsr = &(context->SPSR_FIQ.value);
      break;
    }
    case IRQ_MODE:
    {
      modeString = "IRQ";
      r13 = &(context->R13_IRQ);
      spsr = &(context->SPSR_IRQ.value);
      break;
    }
    case SVC_MODE:
    {
      modeString = "SVC";
      r13 = &(context->R13_SVC);
      spsr = &(context->SPSR_SVC.value);
      break;
    }
    case ABT_MODE:
    {
      modeString = "ABT";
      r13 = &(context->R13_ABT);
      spsr = &(context->SPSR_ABT.value);
      break;
    }
    case UND_MODE:
    {
      modeString = "UND";
      r13 = &(context->R13_UND);
      spsr = &(context->SPSR_UND.value);
      break;
    }
    default:
    {
      modeString = "???";
      spsr = 0;
      return;
    }
  }

  printf(
      "R0:   0x%.8x     R1:   0x%.8x     R2:   0x%.8x     R3:   0x%.8x" EOL
      "R4:   0x%.8x     R5:   0x%.8x     R6:   0x%.8x     R7:   0x%.8x" EOL,
      context->R0, context->R1, context->R2, context->R3,
      context->R4, context->R5, context->R6, context->R7
      );
  printf(
      "R8:   0x%.8x     R9:   0x%.8x     R10:  0x%.8x     R11:  0x%.8x" EOL
      "R12:  0x%.8x     SP:   0x%.8x     LR:   0x%.8x     PC:   0x%.8x" EOL,
      *r8, *(r8 + 1), *(r8 + 2), *(r8 + 3),
      *(r8 + 4), r13 ? *r13 : 0, r13 ? *(r13 + 1) : 0, context->R15
      );

  printf("Mode: %-35s CPSR: 0x%.8x     SPSR: ", modeString, context->CPSR.value);
  if (spsr)
  {
    printf("0x%.8x" EOL, *spsr);
  }
  else
  {
    printf("----------" EOL);
  }

  /* Virtual Memory */
  pageTablesVM *ptVM = context->pageTables;
  printf("virtual machine page table struct at %p" EOL, ptVM);
  printf("guest OS virtual addressing enabled: %x" EOL, context->virtAddrEnabled);
  printf("guest OS Page Table (VA): %p" EOL, ptVM->guestVirtual);
  printf("guest OS Page Table (PA): %p" EOL, ptVM->guestPhysical);
  printf("Shadow Page Table Priv: %p, User %p" EOL, ptVM->shadowPriv, ptVM->shadowUser);
  printf("Current active shadow PT: %p" EOL, ptVM->shadowActive);
  /* .. thats it with virtual memory stuff */
  printf("Hypervisor page table: %p" EOL, context->hypervisorPageTable);
  printf("high exception vector flag: %x" EOL, context->guestHighVectorSet);
  printf("registered exception vector:" EOL);
  printf("Und: %#.8x" EOL, context->guestUndefinedHandler);
  printf("Swi: %#.8x" EOL, context->guestSwiHandler);
  printf("Pabt: %#.8x" EOL, context->guestPrefAbortHandler);
  printf("Dabt: %#.8x" EOL, context->guestDataAbortHandler);
  printf("Unused: %#.8x" EOL, context->guestUnusedHandler);
  printf("IRQ: %#.8x" EOL, context->guestIrqHandler);
  printf("FIQ: %#.8x" EOL, context->guestFiqHandler);
  printf("Hardware library: not core dumping just yet" EOL);
  printf("Virtual Machine structure at %p" EOL, &context->vm);
  printf("Interrupt pending: %x" EOL, context->guestIrqPending);
  printf("Data abort pending: %x" EOL, context->guestDataAbtPending);
  printf("Prefetch abort pending: %x" EOL, context->guestPrefetchAbtPending);
  printf("Guest idle: %x" EOL, context->guestIdle);

#ifdef CONFIG_GUEST_CONTEXT_BLOCK_TRACE
  printf("Block trace:" EOL);
  s32int traceIndex;
  u32int printIndex = 0;
  for (traceIndex = context->blockTraceIndex; traceIndex >= 0 && context->blockTrace[traceIndex];
      traceIndex--, printIndex++)
  {
    printf("%3u: %#.8x" EOL, printIndex, context->blockTrace[traceIndex]);
  }
  if (traceIndex < 0)
  {
    for (traceIndex = CONFIG_GUEST_CONTEXT_BLOCK_TRACE_SIZE - 1;
        traceIndex > (s32int)context->blockTraceIndex && context->blockTrace[traceIndex];
        traceIndex--, printIndex++)
    {
      printf("%3u: %#.8x" EOL, printIndex, context->blockTrace[traceIndex]);
    }
  }
#endif

  /* BlockCache with copied code */
  printf("Translation store at: %p" EOL, context->translationStore);
  printf("translationStore->basicBlockStore: %p\n", context->translationStore->basicBlockStore);
  printf("translationStore->codeStore next: %p\n", context->translationStore->codeStore);
  printf("translationStore->codeStore next free word: %p\n", context->translationStore->codeStoreFreePtr);
  printf("last entry block index: %08x\n", context->lastEntryBlockIndex);
  dumpSdramStats(context->vm.sdram);

  /* context switch counters */
  dumpCounters(context);

#ifdef CONFIG_STATS
  printf("timerTotalSvc:     %08x\n", timerTotalSvc);
  printf("timerNumberSvc:    %08x\n", timerNumberSvc);
  printf("timerTotalDabt:    %08x\n", timerTotalDabt);
  printf("timerNumberDabt:   %08x\n", timerNumberDabt);
  printf("timerTotalIrq:     %08x\n", timerTotalIrq);
  printf("timerNumberIrq:    %08x\n", timerNumberIrq);
  printf("total cycle count: %08x\n", getCycleCount());
  printf("perf counter 1:    %08x\n", getPerfCounter1());
  printf("perf counter 2:    %08x\n", getPerfCounter2());
#endif
}

#ifdef CONFIG_STATS
u32int getPerfCounter1()
{
  u32int sel = 0; /* perf coutner select register: 0 */
  u32int value;
  asm volatile ("MCR p15, 0, %0, c9, c12, 5\t\n":: "r"(sel));
  asm volatile ("MRC p15, 0, %0, c9, c13, 2\t\n": "=r"(value));
  return value;
}

u32int getPerfCounter2()
{
  u32int sel = 1; /* perf coutner select register: 1 */
  u32int value;
  asm volatile ("MCR p15, 0, %0, c9, c12, 5\t\n":: "r"(sel));
  asm volatile ("MRC p15, 0, %0, c9, c13, 2\t\n": "=r"(value));
  return value;
}


u32int getCycleCount()
{
  u32int value;
  asm volatile ("MRC p15, 0, %0, c9, c13, 0\t\n": "=r"(value));
  return value;
}
#endif

bool isGuestInPrivMode(GCONTXT * context)
{
  return (context->CPSR.bits.mode == USR_MODE) ? FALSE : TRUE;
}


/**
 * guest is switching modes.
 **/
void guestChangeMode(GCONTXT *context, u32int newMode)
{
  // we must make sure the correct exception vector is set.
  setExceptionVector(newMode);

  // if changing user <-> priv must change shadow page tables
  bool privilegedBefore = isGuestInPrivMode(context);
  context->CPSR.bits.mode = newMode;
  bool privilegedAfter  = isGuestInPrivMode(context);
  if ((!privilegedBefore) && privilegedAfter)
  {
    userToPrivAddressing(context);
  }
  else if (privilegedBefore && (!privilegedAfter))
  {
    privToUserAddressing(context);
  }
}


#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
void resetExceptionCounters(GCONTXT *context)
{
  context->svcCount = 0;

  context->branchLink = 0;
  context->branchNonlink = 0;
  context->branchConditional = 0;
  context->branchNonconditional = 0;
  context->branchImmediate = 0;
  context->branchRegister = 0;

  context->svc = 0;

  context->dabtCount = 0;
  context->dabtPriv = 0;
  context->dabtUser = 0;
  context->pabtCount = 0;
  context->pabtPriv = 0;
  context->pabtUser = 0;
  context->irqCount = 0;
  context->irqPriv = 0;
  context->irqUser = 0;
}
#endif

