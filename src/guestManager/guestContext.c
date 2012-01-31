#include "common/debug.h"
#include "common/memFunctions.h"
#include "common/stddef.h"

#include "cpuArch/constants.h"

#include "guestManager/guestContext.h"

#include "instructionEmu/commonInstrFunctions.h"
#include "instructionEmu/miscInstructions.h"

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
  DEBUG(GUEST_CONTEXT, "allocateGuestContext: @ %p; block trace @ %p; initializing..." EOL,
      context, &(context->blockHistory));
  /*
   * Fill entire guest context with zero bytes
   */
  memset(context, 0, sizeof(GCONTXT));
  /*
   * Set initial values
   */
  context->CPSR = (PSR_F_BIT | PSR_I_BIT | PSR_SVC_MODE);
  /*
   * Initialise coprocessor register bank
   */
  context->coprocRegBank = (CREG *)mallocBytes(MAX_CRB_SIZE * sizeof(CREG));
  if (context->coprocRegBank == NULL)
  {
    DIE_NOW(0, "Failed to allocate coprocessor register bank.");
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
    DIE_NOW(0, "Failed to allocate basic block cache");
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
    DIE_NOW(0, "Hardware library initialisation failed.");
  }
  /*
   * Setup guest memory protection
   */
  context->memProt = initialiseMemoryProtection();
  return context;
}

void dumpGuestContext(GCONTXT *context)
{
  printf("============== DUMP GUEST CONTEXT ===============" EOL);

  const char *modeString;
  u32int *r8 = &(context->R8);
  u32int *r13 = NULL;
  u32int *spsr = NULL;
  switch (context->CPSR & 0x1F)
  {
    case 0x10: // user
    case 0x1F: // system
      modeString = "USR";
      r13 = &(context->R13_USR);
      break;
    case 0x11: // fiq
      modeString = "FIQ";
      r8 = &(context->R8_FIQ);
      r13 = &(context->R13_FIQ);
      spsr = &(context->SPSR_FIQ);
      break;
    case 0x12: // irq
      modeString = "IRQ";
      r13 = &(context->R13_IRQ);
      spsr = &(context->SPSR_IRQ);
      break;
    case 0x13: // svc
      modeString = "SVC";
      r13 = &(context->R13_SVC);
      spsr = &(context->SPSR_SVC);
      break;
    case 0x17: // abort
      modeString = "ABT";
      r13 = &(context->R13_ABT);
      spsr = &(context->SPSR_ABT);
      break;
    case 0x1B: // undef
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
      "R4:   0x%.8x     R5:   0x%.8x     R6:   0x%.8x     R7:   0x%.8x" EOL
      "R8:   0x%.8x     R9:   0x%.8x     R10:  0x%.8x     R11:  0x%.8x" EOL
      "R12:  0x%.8x     SP:   0x%.8x     LR:   0x%.8x     PC:   0x%.8x" EOL,
      context->R0, context->R1, context->R2, context->R3,
      context->R4, context->R5, context->R6, context->R7,
      *r8, *(r8 + 1), *(r8 + 2), *(r8 + 3),
      *(r8 + 4), r13 ? *r13 : 0, r13 ? *(r13 + 1) : 0, context->R15
      );

  printf("Mode: %s     CPSR: 0x%.8x", modeString, context->CPSR);
  if (spsr)
  {
    printf("     SPSR: 0x%.8x", *spsr);
  }
  printf(EOL);

  printf("endOfBlockInstr: %08x\n", context->endOfBlockInstr);
#ifdef CONFIG_THUMB2
  printf("endOfBlockHalfInstr: %08x\n", context->endOfBlockHalfInstr);
#endif
  printf("handler function addr: %08x\n", (u32int)context->hdlFunct);

  /* Virtual Memory */
  printf("guest OS virtual addressing enabled: %x\n", context->virtAddrEnabled);
  printf("guest OS Page Table: %08x\n", (u32int)context->PT_os);
  printf("guest OS Page Table (real): %08x\n", (u32int)context->PT_os_real);
  printf("guest OS shadow Page Table: %08x\n", (u32int)context->PT_shadow);
  printf("guest physical Page Table: %08x\n", (u32int)context->PT_physical);
  printf("high exception vector flag: %x\n", context->guestHighVectorSet);
  printf("registered exception vector:\n");
  printf("Und: %08x\n", context->guestUndefinedHandler);
  printf("Swi: %08x\n", context->guestSwiHandler);
  printf("Pabt: %08x\n", context->guestPrefAbortHandler);
  printf("Dabt: %08x\n", context->guestDataAbortHandler);
  printf("Unused: %08x\n", context->guestUnusedHandler);
  printf("IRQ: %08x\n", context->guestIrqHandler);
  printf("FIQ: %08x\n", context->guestFiqHandler);
  printf("Hardware library: not core dumping just yet\n");
  printf("Interrupt pending: %x\n", context->guestIrqPending);
  printf("Data abort pending: %x\n", context->guestDataAbtPending);
  printf("Prefetch abort pending: %x\n", context->guestPrefetchAbtPending);
  printf("Guest idle: %x\n", context->guestIdle);
  printf("Block cache at: %08x\n", (u32int)context->blockCache);

  int i = 0;
  printf("Block Trace:\n");
  for (i = BLOCK_HISOTRY_SIZE-1; i >= 0; i--)
  {
    printf("%x: %08x\n", i, context->blockHistory[i]);

  }
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
