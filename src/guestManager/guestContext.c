#include "common/debug.h"
#include "common/memFunctions.h"
#include "common/stddef.h"

#include "cpuArch/constants.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/sdram.h"


GCONTXT *createGuestContext(void)
{
  /*
   * Allocate guest context
   */
  GCONTXT *context = (GCONTXT *)mallocBytes(sizeof(GCONTXT));
  if (context == 0)
  {
    DIE_NOW(NULL, "allocateGuest: Failed to allocate guest context.");
  }
  DEBUG(GUEST_CONTEXT, "allocateGuestContext: @ %p; initialising..." EOL, context);
  /*
   * Set initial values
   */
  memset(context, 0, sizeof(GCONTXT));
  context->CPSR = (PSR_F_BIT | PSR_I_BIT | PSR_SVC_MODE);
  /*
   * Initialise coprocessor register bank
   */
  context->coprocRegBank = (CREG *)mallocBytes(MAX_CRB_SIZE * sizeof(CREG));
  if (context->coprocRegBank == NULL)
  {
    DIE_NOW(context, "Failed to allocate coprocessor register bank.");
  }
  DEBUG(GUEST_CONTEXT, "allocateGuestContext: coprocessor register bank @ %p" EOL,
      context->coprocRegBank);
  initCRB(context->coprocRegBank);
  /*
   * Initialise block cache
   */
  context->blockCache = (BCENTRY *)mallocBytes(BLOCK_CACHE_SIZE * sizeof(BCENTRY));
  if (context->blockCache == NULL)
  {
    DIE_NOW(context, "Failed to allocate basic block cache");
  }
  DEBUG(GUEST_CONTEXT, "allocateGuestContext: block cache @ %p" EOL, context->blockCache);
  memset(context->blockCache, 0, BLOCK_CACHE_SIZE * sizeof(BCENTRY));
  initialiseBlockCache(context->blockCache);
  /*
   * Initialise virtual hardware devices
   */
  context->hardwareLibrary = initialiseHardwareLibrary();
  if (context->hardwareLibrary == NULL)
  {
    DIE_NOW(context, "Hardware library initialisation failed.");
  }
  /*
   * Setup guest memory protection
   */
  context->memProt = initialiseMemoryProtection();
  /*
   * Print the address of the block trace, it may come in handy when debugging...
   */
#ifdef CONFIG_GUEST_CONTEXT_BLOCK_TRACE
  DEBUG(GUEST_CONTEXT, "allocateGuestContext: block trace @ %p" EOL, &(context->blockTrace));
#endif
  return context;
}

void dumpGuestContext(GCONTXT *context)
{
  printf("============== DUMP GUEST CONTEXT ===============" EOL);

  const char *modeString;
  u32int *r8 = &(context->R8);
  u32int *r13 = NULL;
  u32int *spsr = NULL;
  switch (context->CPSR & PSR_MODE)
  {
    case PSR_USR_MODE:
    case PSR_SYS_MODE:
      modeString = "USR";
      r13 = &(context->R13_USR);
      break;
    case PSR_FIQ_MODE:
      modeString = "FIQ";
      r8 = &(context->R8_FIQ);
      r13 = &(context->R13_FIQ);
      spsr = &(context->SPSR_FIQ);
      break;
    case PSR_IRQ_MODE:
      modeString = "IRQ";
      r13 = &(context->R13_IRQ);
      spsr = &(context->SPSR_IRQ);
      break;
    case PSR_SVC_MODE:
      modeString = "SVC";
      r13 = &(context->R13_SVC);
      spsr = &(context->SPSR_SVC);
      break;
    case PSR_ABT_MODE:
      modeString = "ABT";
      r13 = &(context->R13_ABT);
      spsr = &(context->SPSR_ABT);
      break;
    case PSR_UND_MODE:
      modeString = "UND";
      r13 = &(context->R13_UND);
      spsr = &(context->SPSR_UND);
      break;
    default:
      modeString = "???";
      return;
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

  printf("Mode: %-36s CPSR: 0x%.8x     SPSR: ", modeString, context->CPSR);
  spsr = 0;
  if (spsr)
  {
    printf("0x%.8x" EOL, *spsr);
  }
  else
  {
    printf("----------" EOL);
  }

  printf("endOfBlockInstr: %#.8x" EOL, context->endOfBlockInstr);
  printf("handler function addr: %#.8x" EOL, (u32int)context->hdlFunct);

  /* Virtual Memory */
  printf("guest OS virtual addressing enabled: %x" EOL, context->virtAddrEnabled);
  printf("guest OS Page Table: %#.8x" EOL, (u32int)context->PT_os);
  printf("guest OS Page Table (real): %#.8x" EOL, (u32int)context->PT_os_real);
  printf("guest OS shadow Page Table: %#.8x" EOL, (u32int)context->PT_shadow);
  printf("guest physical Page Table: %#.8x" EOL, (u32int)context->PT_physical);
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
  printf("Interrupt pending: %x" EOL, context->guestIrqPending);
  printf("Data abort pending: %x" EOL, context->guestDataAbtPending);
  printf("Prefetch abort pending: %x" EOL, context->guestPrefetchAbtPending);
  printf("Guest idle: %x" EOL, context->guestIdle);
  printf("Block cache at: %#.8x" EOL, (u32int)context->blockCache);

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

#ifdef CONFIG_BLOCK_COPY
  /* BlockCache with copied code */
  printf("gc blockCopyCache: %08x\n", (u32int)context->blockCopyCache);
  printf("gc blockCopyCacheEnd: %08x\n", (u32int)context->blockCopyCacheEnd);
  printf("gc blockCopyCacheLastUsedLine: %08x\n", (u32int)context->blockCopyCacheLastUsedLine);
  printf("gc PCOfLastInstruction: %08x\n", (u32int)context->PCOfLastInstruction);
#endif

  dumpSdramStats();
}

#ifdef CONFIG_BLOCK_COPY
void registerBlockCopyCache(GCONTXT *gc, u32int * blockCopyCache, u32int size)
{
  gc->blockCopyCache = (u32int)blockCopyCache;
  gc->blockCopyCacheEnd = (u32int)(blockCopyCache + size - 1);  // !pointer arithmetic size is size in number of u32ints
  gc->blockCopyCacheLastUsedLine = (u32int)(blockCopyCache-1);
}
#endif
