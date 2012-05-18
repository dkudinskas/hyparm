#ifndef __GUEST_MANAGER__GUEST_CONTEXT_H__
#define __GUEST_MANAGER__GUEST_CONTEXT_H__

#include "common/compiler.h"
#include "common/types.h"

#include "guestManager/blockCache.h"

#include "vm/omap35xx/hardwareLibrary.h"
#include "vm/omap35xx/cp15coproc.h"
#include "vm/omap35xx/omap35xx.h"

#include "memoryManager/memoryProtection.h"


#define GC_R0_OFFS        0x00
#define GC_R1_OFFS        0x04
#define GC_R2_OFFS        0x08
#define GC_R3_OFFS        0x0C
#define GC_R4_OFFS        0x10
#define GC_R5_OFFS        0x14
#define GC_R6_OFFS        0x18
#define GC_R7_OFFS        0x1C
#define GC_R8_OFFS        0x20
#define GC_R9_OFFS        0x24
#define GC_R10_OFFS       0x28
#define GC_R11_OFFS       0x2C
#define GC_R12_OFFS       0x30
#define GC_R13_OFFS       0x34
#define GC_R14_OFFS       0x38
#define GC_R15_OFFS       0x3C
#define GC_CPSR_OFFS      0x40
#define GC_R8_FIQ_OFFS    0x44
#define GC_R9_FIQ_OFFS    0x48
#define GC_R10_FIQ_OFFS   0x4C
#define GC_R11_FIQ_OFFS   0x50
#define GC_R12_FIQ_OFFS   0x54
#define GC_R13_FIQ_OFFS   0x58
#define GC_R14_FIQ_OFFS   0x5C
#define GC_SPSR_FIQ_OFFS  0x60
#define GC_R13_SVC_OFFS   0x64
#define GC_R14_SVC_OFFS   0x68
#define GC_SPSR_SVC_OFFS  0x6C
#define GC_R13_ABT_OFFS   0x70
#define GC_R14_ABT_OFFS   0x74
#define GC_SPSR_ABT_OFFS  0x78
#define GC_R13_IRQ_OFFS   0x7C
#define GC_R14_IRQ_OFFS   0x80
#define GC_SPSR_IRQ_OFFS  0x84
#define GC_R13_UND_OFFS   0x88
#define GC_R14_UND_OFFS   0x8C
#define GC_SPSR_UND_OFFS  0x90


struct VirtualMachinePageTables;
typedef struct VirtualMachinePageTables pageTablesVM;

struct guestContext;
typedef struct guestContext GCONTXT;

typedef u32int (*instructionHandler)(GCONTXT *context, u32int instruction);

enum guestOSType
{
  GUEST_OS_NOT_SET = 0,
  GUEST_OS_LINUX,
  GUEST_OS_FREERTOS
};

struct VirtualMachinePageTables
{
  simpleEntry* hypervisor;
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
  u32int CPSR;
  u32int R8_FIQ;
  u32int R9_FIQ;
  u32int R10_FIQ;
  u32int R11_FIQ;
  u32int R12_FIQ;
  u32int R13_FIQ;
  u32int R14_FIQ;
  u32int SPSR_FIQ;
  u32int R13_SVC;
  u32int R14_SVC;
  u32int SPSR_SVC;
  u32int R13_ABT;
  u32int R14_ABT;
  u32int SPSR_ABT;
  u32int R13_IRQ;
  u32int R14_IRQ;
  u32int SPSR_IRQ;
  u32int R13_UND;
  u32int R14_UND;
  u32int SPSR_UND;
  u32int endOfBlockInstr;
  instructionHandler hdlFunct;
  CREG * coprocRegBank;
  BCENTRY * blockCache;
  u32int * execBitMap;
  
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
  bool virtAddrEnabled;
  virtualMachine* vm;
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
};



/*
 * Gets the guest context pointer.
 * Defined in startup.s!
 */
extern GCONTXT *getGuestContext(void);

GCONTXT *allocateGuestContext(void);
void initGuestContext(GCONTXT* context);

void dumpGuestContext(const GCONTXT* gc);

/* a function to evaluate if guest is in priviledge mode or user mode */
bool isGuestInPrivMode(GCONTXT* context);

/* a function to to switch the guest to user mode */
void guestToUserMode(void);

/* a function to to switch the guest to privileged mode */
void guestToPrivMode(void);

/* function to call when hypervisor changes guest modes. */
void guestChangeMode(u32int guestMode);

__macro__ void traceBlock(GCONTXT *context, u32int startAddress);


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


#endif /* __GUEST_MANAGER__GUEST_CONTEXT_H__ */
