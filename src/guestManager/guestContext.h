#ifndef __GUEST_MANAGER__GUEST_CONTEXT_H__
#define __GUEST_MANAGER__GUEST_CONTEXT_H__

#include "cpuArch/state.h"

#include "common/compiler.h"
#include "common/types.h"

#include "guestManager/translationStore.h"
#include "guestManager/types.h"

#include "instructionEmu/decoder.h"

#include "memoryManager/pageTableInfo.h"

#include "vm/omap35xx/cp15coproc.h"
#include "vm/omap35xx/hardwareLibrary.h"
#include "vm/omap35xx/omap35xx.h"


#define NUMBER_OF_SECTIONS  4096
#define NUMBER_OF_SMALL_PAGES 256
#define SIZE_BITMAP1        (NUMBER_OF_SECTIONS * NUMBER_OF_SMALL_PAGES)/ 8

struct VirtualMachinePageTables;
typedef struct VirtualMachinePageTables pageTablesVM;


enum guestOSType
{
  GUEST_OS_NOT_SET = 0,
  GUEST_OS_LINUX,
  GUEST_OS_FREERTOS,
  GUEST_OS_TEST
};

struct VirtualMachinePageTables
{
  simpleEntry* guestVirtual;
  simpleEntry* guestPhysical;
  simpleEntry* shadowPriv;
  simpleEntry* shadowUser;
  simpleEntry* shadowActive;
  u32int contextID;
  ptInfo* sptInfo;
  ptInfo* gptInfo;
};


struct guestContext
{
  u32int R0;
  u32int R1;
  u32int R2;
  u32int R3;
  u32int R4;
  u32int R5;
  u32int R6;
  u32int R7;
  u32int R8;
  u32int R9;
  u32int R10;
  u32int R11;
  u32int R12;
  u32int R13_USR;
  u32int R14_USR;
  u32int R15;
  CPSRreg CPSR;
  u32int R8_FIQ;
  u32int R9_FIQ;
  u32int R10_FIQ;
  u32int R11_FIQ;
  u32int R12_FIQ;
  u32int R13_FIQ;
  u32int R14_FIQ;
  CPSRreg SPSR_FIQ;
  u32int R13_SVC;
  u32int R14_SVC;
  CPSRreg SPSR_SVC;
  u32int R13_ABT;
  u32int R14_ABT;
  CPSRreg SPSR_ABT;
  u32int R13_IRQ;
  u32int R14_IRQ;
  CPSRreg SPSR_IRQ;
  u32int R13_UND;
  u32int R14_UND;
  CPSRreg SPSR_UND;
  CREG * coprocRegBank;
  TranslationStore* translationStore;
#ifdef CONFIG_GUEST_CONTEXT_BLOCK_TRACE
  u32int blockTrace[CONFIG_GUEST_CONTEXT_BLOCK_TRACE_SIZE];
  u32int blockTraceIndex;
#endif
#ifdef CONFIG_LOOP_DETECTOR
  s32int loopDetectorLoopCount;
  s32int loopDetectorNextTreshold;
#endif
  /* Virtual Addressing */
  pageTablesVM* pageTables;
  simpleEntry* hypervisorPageTable;
  bool virtAddrEnabled;
  virtualMachine vm;
  /* vector address in vmem */
  bool guestHighVectorSet;
  /* exception vector */
  u32int guestUndefinedHandler;
  u32int guestSwiHandler;
  u32int guestPrefAbortHandler;
  u32int guestDataAbortHandler;
  u32int guestUnusedHandler;
  u32int guestIrqHandler;
  u32int guestFiqHandler;
  device * hardwareLibrary;
  /* exception flags */
  bool guestIrqPending;
  bool guestDataAbtPending;
  bool guestPrefetchAbtPending;
  bool guestIdle;
  /* for OS-specific quirks */
  enum guestOSType os;

  /* This will contain the guest PC of the last instruction in active BB*/
  u32int lastEntryBlockIndex;

  u8int *execBitmap;

#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
  u32int svcCount;

  u32int branchLink;
  u32int branchNonlink;
  u32int branchConditional;
  u32int branchNonconditional;
  u32int branchImmediate;
  u32int branchRegister;

  u32int svcGuest;
  u32int armStmInstruction;
  u32int armLdmInstruction;
  u32int armLdrInstruction;
  u32int armBInstruction;
  u32int armSwpInstruction;
  u32int armLdrexbInstruction;
  u32int armLdrexdInstruction;
  u32int armLdrexhInstruction;
  u32int armStrexbInstruction;
  u32int armStrexdInstruction;
  u32int armStrexhInstruction;
  u32int armLdrexInstruction;
  u32int armStrexInstruction;
  u32int armBxInstruction;
  u32int armBxjInstruction;
  u32int armBkptInstruction;
  u32int armSmcInstruction;
  u32int armBlxRegisterInstruction;
  u32int armAndInstruction;
  u32int armEorInstruction;
  u32int armSubInstruction;
  u32int armAddInstruction;
  u32int armAdcInstruction;
  u32int armSbcInstruction;
  u32int armRscInstruction;
  u32int armMsrInstruction;
  u32int armMrsInstruction;
  u32int armOrrInstruction;
  u32int armMovInstruction;
  u32int armLslInstruction;
  u32int armLsrInstruction;
  u32int armAsrInstruction;
  u32int armRrxInstruction;
  u32int armRorInstruction;
  u32int armBicInstruction;
  u32int armMvnInstruction;
  u32int armYieldInstruction;
  u32int armWfeInstruction;
  u32int armWfiInstruction;
  u32int armSevInstruction;
  u32int armDbgInstruction;
  u32int svcInstruction;
  u32int armMrcInstruction;
  u32int armMcrInstruction;
  u32int armDmbInstruction;
  u32int armDsbInstruction;
  u32int armIsbInstruction;
  u32int armClrexInstruction;
  u32int armCpsInstruction;
  u32int armRfeInstruction;
  u32int armSetendInstruction;
  u32int armSrsInstruction;
  u32int armBlxImmediateInstruction;
  u32int armPldInstruction;
  u32int armPliInstruction;
  u32int armStrbtInstruction;
  u32int armStrhtInstruction;
  u32int armStrtInstruction;
  u32int armLdrbtInstruction;
  u32int armLdrhtInstruction;
  u32int armLdrtInstruction;
  
  u32int dabtCount;
  u32int dabtPriv;
  u32int dabtUser;
  u32int pabtCount;
  u32int pabtPriv;
  u32int pabtUser;
  u32int irqCount;
  u32int irqPriv;
  u32int irqUser;
#endif
};


/*
 * Active guest context pointer; defined in startup.S
 * WARNING: because it's volatile, EVERY use of this variable will cause a LOAD or STORE. So keep
 * it sane and make a copy as non-volatile when using it. On the other hand, make sure to keep this
 * one volatile to avoid funky compiler optimizations.
 */
extern GCONTXT *volatile activeGuestContext;


GCONTXT *createGuestContext(void) __cold__;

void dumpGuestContext(const GCONTXT * gc) __cold__;

void dumpCounters(void);

__macro__ GCONTXT *getActiveGuestContext(void);

/* a function to evaluate if guest is in priviledge mode or user mode */
bool isGuestInPrivMode(GCONTXT *context);

/* function to call when hypervisor changes guest modes. */
void guestChangeMode(GCONTXT *context, u32int guestMode);

__macro__ void traceBlock(GCONTXT *context, u32int startAddress);


__macro__ GCONTXT *getActiveGuestContext()
{
  return activeGuestContext;
}

__macro__ void traceBlock(GCONTXT *context, u32int startAddress)
{
#ifdef CONFIG_GUEST_CONTEXT_BLOCK_TRACE
  context->blockTraceIndex++;
  if (context->blockTraceIndex >= CONFIG_GUEST_CONTEXT_BLOCK_TRACE_SIZE)
  {
    context->blockTraceIndex = 0;
  }
  context->blockTrace[context->blockTraceIndex] = startAddress;
#endif /* CONFIG_GUEST_CONTEXT_BLOCK_TRACE */
}

#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
void resetExceptionCounters(GCONTXT *context);
#endif

#endif /* __GUEST_MANAGER__GUEST_CONTEXT_H__ */
