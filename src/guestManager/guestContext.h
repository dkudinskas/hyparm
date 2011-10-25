#ifndef __GUEST_MANAGER__GUEST_CONTEXT_H__
#define __GUEST_MANAGER__GUEST_CONTEXT_H__

#include "common/compiler.h"
#include "common/types.h"

#include "guestManager/blockCache.h"

#include "vm/omap35xx/hardwareLibrary.h"

#include "memoryManager/cp15coproc.h"
#include "memoryManager/memoryProtection.h"


struct guestContext;
typedef struct guestContext GCONTXT;

typedef u32int (*instructionHandler)(GCONTXT *context, u32int instruction);


enum guestOSType
{
  GUEST_OS_NOT_SET = 0,
  GUEST_OS_LINUX,
  GUEST_OS_FREERTOS
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
#ifdef CONFIG_GUEST_CONTEXT_BLOCK_TRACE
  u32int blockTrace[CONFIG_GUEST_CONTEXT_BLOCK_TRACE_SIZE];
  u32int blockTraceIndex;
#endif
#ifdef CONFIG_LOOP_DETECTOR
  s32int loopDetectorLoopCount;
  s32int loopDetectorNextTreshold;
#endif
  /* Virtual Addressing */
  descriptor* PT_physical; // guest physical to real physical PT
  descriptor* PT_os;       // guest OS to guest Physical PT
  descriptor* PT_os_real;  // physical addr of PT_os for H/W
  descriptor* PT_shadow;   // guest OS to real physical PT
  bool virtAddrEnabled;
  /* Virtual Addressing end */
  MEMPROT* memProt;
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


#ifdef CONFIG_GUEST_CONTEXT_BLOCK_TRACE

__macro__ void traceBlock(GCONTXT *context, u32int startAddress)
{
  context->blockTraceIndex++;
  if (context->blockTraceIndex >= CONFIG_GUEST_CONTEXT_BLOCK_TRACE_SIZE)
  {
    context->blockTraceIndex = 0;
  }
  context->blockTrace[context->blockTraceIndex] = startAddress;
}

#else

#define traceBlock(context, startAddress)

#endif /* CONFIG_GUEST_CONTEXT_BLOCK_TRACE */


GCONTXT *createGuestContext(void);

void dumpGuestContext(GCONTXT * gc);

/*
 * Gets the guest context pointer.
 * Defined in startup.s!
 */
extern GCONTXT *getGuestContext(void);

#endif
