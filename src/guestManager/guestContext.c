#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "cpuArch/constants.h"

#include "guestManager/guestContext.h"

#include "memoryManager/addressing.h"

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
  context->CPSR = (PSR_F_BIT | PSR_I_BIT | PSR_SVC_MODE);

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
  switch (context->CPSR & PSR_MODE)
  {
    case PSR_USR_MODE:
    case PSR_SYS_MODE:
    {
      modeString = "USR";
      r13 = &(context->R13_USR);
      break;
    }
    case PSR_FIQ_MODE:
    {
      modeString = "FIQ";
      r8 = &(context->R8_FIQ);
      r13 = &(context->R13_FIQ);
      spsr = &(context->SPSR_FIQ);
      break;
    }
    case PSR_IRQ_MODE:
    {
      modeString = "IRQ";
      r13 = &(context->R13_IRQ);
      spsr = &(context->SPSR_IRQ);
      break;
    }
    case PSR_SVC_MODE:
    {
      modeString = "SVC";
      r13 = &(context->R13_SVC);
      spsr = &(context->SPSR_SVC);
      break;
    }
    case PSR_ABT_MODE:
    {
      modeString = "ABT";
      r13 = &(context->R13_ABT);
      spsr = &(context->SPSR_ABT);
      break;
    }
    case PSR_UND_MODE:
    {
      modeString = "UND";
      r13 = &(context->R13_UND);
      spsr = &(context->SPSR_UND);
      break;
    }
    default:
    {
      modeString = "???";
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

  printf("Mode: %-35s CPSR: 0x%.8x     SPSR: ", modeString, context->CPSR);
  spsr = 0;
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
  printf("guest PC Of Last guest Instruction: %08x\n", context->lastGuestPC);
  printf("last entry block index: %08x\n", context->lastEntryBlockIndex);
  dumpSdramStats(context->vm.sdram);

#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
  printf("====================================\n");
  printf("svc  count: %08x\n", context->svcCount);
  printf("====================================\n");
  
  printf("svcGuest: %08x\n", context->svcGuest);
  printf("armStmInstruction: %08x\n", context->armStmInstruction);
  printf("armStrbtInstruction: %08x\n", context->armStrbtInstruction);
  printf("armStrhtInstruction: %08x\n", context->armStrhtInstruction);
  printf("armStrtInstruction: %08x\n", context->armStrtInstruction);
  printf("armStrexbInstruction: %08x\n", context->armStrexbInstruction);
  printf("armStrexdInstruction: %08x\n", context->armStrexdInstruction);
  printf("armStrexhInstruction: %08x\n", context->armStrexhInstruction);
  printf("armStrexInstruction: %08x\n", context->armStrexInstruction);

  printf("armLdmInstruction: %08x\n", context->armLdmInstruction);
  printf("armLdrInstruction: %08x\n", context->armLdrInstruction);
  printf("armLdrbtInstruction: %08x\n", context->armLdrbtInstruction);
  printf("armLdrhtInstruction: %08x\n", context->armLdrhtInstruction);
  printf("armLdrtInstruction: %08x\n", context->armLdrtInstruction);
  printf("armLdrexbInstruction: %08x\n", context->armLdrexbInstruction);
  printf("armLdrexdInstruction: %08x\n", context->armLdrexdInstruction);
  printf("armLdrexhInstruction: %08x\n", context->armLdrexhInstruction);
  printf("armLdrexInstruction: %08x\n", context->armLdrexInstruction);

  printf("armBInstruction: %08x\n", context->armBInstruction);
  printf("armBxInstruction: %08x\n", context->armBxInstruction);
  printf("armBxjInstruction: %08x\n", context->armBxjInstruction);
  printf("armBlxRegisterInstruction: %08x\n", context->armBlxRegisterInstruction);
  printf("armBlxImmediateInstruction: %08x\n", context->armBlxImmediateInstruction);
  printf("====== OF THESE, BRANCHES WERE: ==========\n");
  printf("branchLink: %08x\n", context->branchLink);
  printf("branchNonlink: %08x\n", context->branchNonlink);
  printf("branchConditional: %08x\n", context->branchConditional);
  printf("branchNonconditional: %08x\n", context->branchNonconditional);
  printf("branchImmediate: %08x\n", context->branchImmediate);
  printf("branchRegister: %08x\n", context->branchRegister);
  printf("===========================================\n");

  printf("armMsrInstruction: %08x\n", context->armMsrInstruction);
  printf("armMrsInstruction: %08x\n", context->armMrsInstruction);
  printf("armCpsInstruction: %08x\n", context->armCpsInstruction);

  printf("armSwpInstruction: %08x\n", context->armSwpInstruction);
  printf("armYieldInstruction: %08x\n", context->armYieldInstruction);
  printf("armWfeInstruction: %08x\n", context->armWfeInstruction);
  printf("armWfiInstruction: %08x\n", context->armWfiInstruction);
  printf("armSevInstruction: %08x\n", context->armSevInstruction);
  printf("armDbgInstruction: %08x\n", context->armDbgInstruction);

  printf("svcInstruction: %08x\n", context->svcInstruction);

  printf("armBkptInstruction: %08x\n", context->armBkptInstruction);
  printf("armSmcInstruction: %08x\n", context->armSmcInstruction);
  printf("armAndInstruction: %08x\n", context->armAndInstruction);
  printf("armEorInstruction: %08x\n", context->armEorInstruction);
  printf("armSubInstruction: %08x\n", context->armSubInstruction);
  printf("armAddInstruction: %08x\n", context->armAddInstruction);
  printf("armAdcInstruction: %08x\n", context->armAdcInstruction);
  printf("armSbcInstruction: %08x\n", context->armSbcInstruction);
  printf("armRscInstruction: %08x\n", context->armRscInstruction);
  printf("armOrrInstruction: %08x\n", context->armOrrInstruction);
  printf("armMovInstruction: %08x\n", context->armMovInstruction);
  printf("armLslInstruction: %08x\n", context->armLslInstruction);
  printf("armLsrInstruction: %08x\n", context->armLsrInstruction);
  printf("armAsrInstruction: %08x\n", context->armAsrInstruction);
  printf("armRrxInstruction: %08x\n", context->armRrxInstruction);
  printf("armRorInstruction: %08x\n", context->armRorInstruction);
  printf("armBicInstruction: %08x\n", context->armBicInstruction);
  printf("armMvnInstruction: %08x\n", context->armMvnInstruction);

  printf("armMrcInstruction: %08x\n", context->armMrcInstruction);
  printf("armMcrInstruction: %08x\n", context->armMcrInstruction);
  printf("armDmbInstruction: %08x\n", context->armDmbInstruction);
  printf("armDsbInstruction: %08x\n", context->armDsbInstruction);
  printf("armIsbInstruction: %08x\n", context->armIsbInstruction);
  printf("armClrexInstruction: %08x\n", context->armClrexInstruction);

  printf("armRfeInstruction: %08x\n", context->armRfeInstruction);
  printf("armSetendInstruction: %08x\n", context->armSetendInstruction);
  printf("armSrsInstruction: %08x\n", context->armSrsInstruction);
  printf("armPldInstruction: %08x\n", context->armPldInstruction);
  printf("armPliInstruction: %08x\n", context->armPliInstruction);
  printf("====================================\n");
  printf("dabt count: %08x\n", context->dabtCount);
  printf("dabtPriv: %08x\n", context->dabtPriv);
  printf("dabtUser: %08x\n", context->dabtUser);
  printf("====================================\n");
  printf("pabt count: %08x\n", context->pabtCount);
  printf("pabtPriv: %08x\n", context->pabtPriv);
  printf("pabtUser: %08x\n", context->pabtUser);
  printf("====================================\n");
  printf("irq  count: %08x\n", context->irqCount);
  printf("irqPriv: %08x\n", context->irqPriv);
  printf("irqUser: %08x\n", context->irqUser);
  printf("====================================\n");
#endif
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
  u32int modeField = context->CPSR & PSR_MODE;
  return (modeField == PSR_USR_MODE) ? FALSE : TRUE;
}


/**
 * switching from privileged to user mode
 **/
void guestToUserMode(GCONTXT *context)
{
  privToUserAddressing(context);
}


/**
 * switching from user to privileged mode
 **/
void guestToPrivMode(GCONTXT *context)
{
  userToPrivAddressing(context);
}


/**
 * guest is switching modes.
 **/
void guestChangeMode(u32int guestMode)
{
  // we must make sure the correct exception vector is set.
  setExceptionVector(guestMode);
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

  context->svcGuest = 0;
  context->armStmInstruction = 0;
  context->armLdmInstruction = 0;
  context->armLdrInstruction = 0;
  context->armBInstruction = 0;
  context->armSwpInstruction = 0;
  context->armLdrexbInstruction = 0;
  context->armLdrexdInstruction = 0;
  context->armLdrexhInstruction = 0;
  context->armStrexbInstruction = 0;
  context->armStrexdInstruction = 0;
  context->armStrexhInstruction = 0;
  context->armLdrexInstruction = 0;
  context->armStrexInstruction = 0;
  context->armBxInstruction = 0;
  context->armBxjInstruction = 0;
  context->armBkptInstruction = 0;
  context->armSmcInstruction = 0;
  context->armBlxRegisterInstruction = 0;
  context->armAndInstruction = 0;
  context->armEorInstruction = 0;
  context->armSubInstruction = 0;
  context->armAddInstruction = 0;
  context->armAdcInstruction = 0;
  context->armSbcInstruction = 0;
  context->armRscInstruction = 0;
  context->armMsrInstruction = 0;
  context->armMrsInstruction = 0;
  context->armOrrInstruction = 0;
  context->armMovInstruction = 0;
  context->armLslInstruction = 0;
  context->armLsrInstruction = 0;
  context->armAsrInstruction = 0;
  context->armRrxInstruction = 0;
  context->armRorInstruction = 0;
  context->armBicInstruction = 0;
  context->armMvnInstruction = 0;
  context->armYieldInstruction = 0;
  context->armWfeInstruction = 0;
  context->armWfiInstruction = 0;
  context->armSevInstruction = 0;
  context->armDbgInstruction = 0;
  context->svcInstruction = 0;
  context->armMrcInstruction = 0;
  context->armMcrInstruction = 0;
  context->armDmbInstruction = 0;
  context->armDsbInstruction = 0;
  context->armIsbInstruction = 0;
  context->armClrexInstruction = 0;
  context->armCpsInstruction = 0;
  context->armRfeInstruction = 0;
  context->armSetendInstruction = 0;
  context->armSrsInstruction = 0;
  context->armBlxImmediateInstruction = 0;
  context->armPldInstruction = 0;
  context->armPliInstruction = 0;
  context->armStrbtInstruction = 0;
  context->armStrhtInstruction = 0;
  context->armStrtInstruction = 0;
  context->armLdrbtInstruction = 0;
  context->armLdrhtInstruction = 0;
  context->armLdrtInstruction = 0;

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

void dumpCounters()
{
  dumpGuestContext(getActiveGuestContext());
}
#endif

