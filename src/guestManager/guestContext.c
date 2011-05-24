#include "common/debug.h"
#include "common/memFunctions.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/sdram.h"

#include "instructionEmu/commonInstrFunctions.h"
#include "instructionEmu/miscInstructions.h"

GCONTXT * guest;


GCONTXT * allocateGuest(void)
{
  GCONTXT * context = (GCONTXT*)mallocBytes(sizeof(GCONTXT));
  if (context == 0)
  {
    DIE_NOW(0, "allocateGuest: Failed to allocate guest context.");
  }
#ifdef GUEST_CONTEXT_DBG
  else
  {
    printf("allocateGuest: Guest context at%x\n", (u32int)context);
  }
#endif
  guest = context;
  initGuestContext(context);

  /* initialise coprocessor register bank */
  CREG * coprocRegBank = (CREG*)mallocBytes(MAX_CRB_SIZE * sizeof(CREG));
  if (coprocRegBank == 0)
  {
    DIE_NOW(0, "Failed to allocate coprocessor register bank.");
  }
  else
  {
    memset((void*)coprocRegBank, 0x0, MAX_CRB_SIZE * sizeof(CREG));
  }
  initCRB(coprocRegBank);
#ifdef GUEST_CONTEXT_DBG
  printf("allocateGuest: Coprocessor register bank at %x\n", (u32int)coprocRegBank);
#endif
  context->coprocRegBank = coprocRegBank;


  /* initialise block cache */
  BCENTRY * blockCache = (BCENTRY*)mallocBytes(BLOCK_CACHE_SIZE * sizeof(BCENTRY));
  if (blockCache == 0)
  {
    DIE_NOW(0, "Failed to allocate basic block cache.\n");
  }
  else
  {
    memset((void*)blockCache, 0x0, BLOCK_CACHE_SIZE * sizeof(BCENTRY));
  }
  initialiseBlockCache(blockCache);
#ifdef GUEST_CONTEXT_DBG
  printf("allocateGuest: basic block cache at %x\n", (u32int)blockCache);
#endif
  context->blockCache = blockCache;


  /* initialise virtual hardware devices */
  device * libraryPtr;
  if ((libraryPtr = initialiseHardwareLibrary()) != 0)
  {
    /* great success. register with guest context */
    context->hardwareLibrary = libraryPtr;
  }
  else
  {
    DIE_NOW(0, "Hardware library initialisation failed.");
  }

  /* Setup guest memory protection */
  context->memProt = initialiseMemoryProtection();


  return context;
}

void initGuestContext(GCONTXT * gContext)
{
#ifdef GUEST_CONTEXT_DBG
  printf("initGuestContext: Initializing guest context @ %x\n", (u32int)gContext);
#endif

  /* zero context!!! */
  gContext->R0 = 0;
  gContext->R1 = 0;
  gContext->R2 = 0;
  gContext->R3 = 0;
  gContext->R4 = 0;
  gContext->R5 = 0;
  gContext->R6 = 0;
  gContext->R7 = 0;
  gContext->R8 = 0;
  gContext->R9 = 0;
  gContext->R10 = 0;
  gContext->R11 = 0;
  gContext->R12 = 0;
  gContext->R13_USR = (u32int)mallocBytes(GUEST_STACK_SIZE);
  if (gContext->R13_USR == 0)
  {
    DIE_NOW(0, "initGuestContext: Failed to allocate guest USR stack.");
  }
  gContext->R14_USR = 0;
  gContext->R8_FIQ = 0;
  gContext->R9_FIQ = 0;
  gContext->R10_FIQ = 0;
  gContext->R11_FIQ = 0;
  gContext->R12_FIQ = 0;
  gContext->R13_FIQ = (u32int)mallocBytes(GUEST_STACK_SIZE);
  if (gContext->R13_FIQ == 0)
  {
    DIE_NOW(0, "initGuestContext: Failed to allocate guest FIQ stack.");
  }
  gContext->R14_FIQ = 0;
  gContext->SPSR_FIQ = 0;
  gContext->R13_SVC = (u32int)mallocBytes(GUEST_STACK_SIZE);
  if (gContext->R13_SVC == 0)
  {
    DIE_NOW(0, "initGuestContext: Failed to allocate guest SVC stack.");
  }
  gContext->R14_SVC = 0;
  gContext->SPSR_SVC = 0;
  gContext->R13_ABT = (u32int)mallocBytes(GUEST_STACK_SIZE);
  if (gContext->R13_ABT == 0)
  {
    DIE_NOW(0, "initGuestContext: Failed to allocate guest ABT stack.");
  }
  gContext->R14_ABT = 0;
  gContext->SPSR_ABT = 0;
  gContext->R13_IRQ = (u32int)mallocBytes(GUEST_STACK_SIZE);
  if (gContext->R13_IRQ == 0)
  {
    DIE_NOW(0, "initGuestContext: Failed to allocate guest IRQ stack.");
  }
  gContext->R14_IRQ = 0;
  gContext->SPSR_IRQ = 0;
  gContext->R13_UND = 0;
  gContext->R13_UND = (u32int)mallocBytes(GUEST_STACK_SIZE);
  if (gContext->R13_UND == 0)
  {
    DIE_NOW(0, "initGuestContext: Failed to allocate guest UND stack.");
  }
  gContext->R14_UND = 0;
  gContext->SPSR_UND = 0;
  gContext->R15 = 0;
  gContext->CPSR = (CPSR_FIQ_BIT | CPSR_IRQ_BIT | CPSR_MODE_SVC);
  gContext->endOfBlockInstr = 0;
  gContext->hdlFunct = 0;
  gContext->coprocRegBank = 0;
  gContext->blockCache = 0;
  gContext->virtAddrEnabled = FALSE;
  gContext->PT_physical = 0;
  gContext->PT_os = 0;
  gContext->PT_shadow = 0;
  gContext->memProt = 0;
  gContext->guestHighVectorSet = FALSE;
  gContext->guestUndefinedHandler = 0;
  gContext->guestSwiHandler = 0;
  gContext->guestPrefAbortHandler = 0;
  gContext->guestDataAbortHandler = 0;
  gContext->guestUnusedHandler = 0;
  gContext->guestIrqHandler = 0;
  gContext->guestFiqHandler = 0;
  gContext->hardwareLibrary = 0;
  gContext->guestIrqPending = FALSE;
  gContext->guestDataAbtPending = FALSE;
  gContext->guestPrefetchAbtPending = FALSE;
  gContext->guestIdle = FALSE;
#ifdef GUEST_CONTEXT_DBG
  printf("initGuestContext: Block Trace @ address %08x\n", (u32int)&(gContext->blockHistory));
#endif
  int i = 0;
  for (i = 0; i < BLOCK_HISOTRY_SIZE; i++)
  {
    gContext->blockHistory[i] = 0;
  }
  gContext->debugFlag = FALSE;
}


void dumpGuestContext(GCONTXT * gc)
{
  u32int mode = gc->CPSR & 0x1F;
  printf("============== DUMP GUEST CONTEXT ===============\n"); 
  printf("R0: %08x\n", gc->R0);
  printf("R1: %08x\n", gc->R1);
  printf("R2: %08x\n", gc->R2);
  printf("R3: %08x\n", gc->R3);
  printf("R4: %08x\n", gc->R4);
  printf("R5: %08x\n", gc->R5);
  printf("R6: %08x\n", gc->R6);
  printf("R7: %08x\n", gc->R7);
  if ( mode == 0x11 )
  {
    printf("R8_FIQ: %08x\n", gc->R8_FIQ);
    printf("R9_FIQ: %08x\n", gc->R9_FIQ);
    printf("R10_FIQ: %08x\n", gc->R10_FIQ);
    printf("R11_FIQ: %08x\n", gc->R11_FIQ);
    printf("R12_FIQ: %08x\n", gc->R12_FIQ);
  }
  else
  {
    printf("R8: %08x\n", gc->R8);
    printf("R9: %08x\n", gc->R9);
    printf("R10: %08x\n", gc->R10);
    printf("R11: %08x\n", gc->R11);
    printf("R12: %08x\n", gc->R12);
  }

  switch(mode)
  {
    case 0x10: // user
    case 0x1F: // system
      printf("R13_USR: %08x\n", gc->R13_USR);
      printf("R14_USR: %08x\n", gc->R14_USR);
      break;
    case 0x11: // fiq
      printf("R13_FIQ: %08x\n", gc->R13_FIQ);
      printf("R14_FIQ: %08x\n", gc->R14_FIQ);
      printf("SPSR_FIQ: %08x\n", gc->SPSR_FIQ);
      break;
    case 0x12: // irq
      printf("R13_IRQ: %08x\n", gc->R13_IRQ);
      printf("R14_IRQ: %08x\n", gc->R14_IRQ);
      printf("SPSR_IRQ: %08x\n", gc->SPSR_IRQ);
      break;
    case 0x13: // svc
      printf("R13_SVC: %08x\n", gc->R13_SVC);
      printf("R14_SVC: %08x\n", gc->R14_SVC);
      printf("SPSR_SVC: %08x\n", gc->SPSR_SVC);
      break;
    case 0x17: // abort
      printf("R13_ABT: %08x\n", gc->R13_ABT);
      printf("R14_ABT: %08x\n", gc->R14_ABT);
      printf("SPSR_ABT: %08x\n", gc->SPSR_ABT);
      break;
    case 0x1B: // undef
      printf("R13_UND: %08x\n", gc->R13_UND);
      printf("R14_UND: %08x\n", gc->R14_UND);
      printf("SPSR_UND: %08x\n", gc->SPSR_UND);
      break;
    default:
      printf("dumpGuestContext: invalid mode in CPSR!\n");
      return;
  }

  printf("R15: %08x\n", gc->R15);
  printf("CPSR: %08x\n", gc->CPSR);
  printf("endOfBlockInstr: %08x\n", gc->endOfBlockInstr);
  printf("handler function addr: %08x\n", (u32int)gc->hdlFunct);

  /* Virtual Memory */
  printf("guest OS virtual addressing enabled: %x\n", gc->virtAddrEnabled);
  printf("guest OS Page Table: %08x\n", (u32int)gc->PT_os);
  printf("guest OS Page Table (real): %08x\n", (u32int)gc->PT_os_real);
  printf("guest OS shadow Page Table: %08x\n", (u32int)gc->PT_shadow);
  printf("guest physical Page Table: %08x\n", (u32int)gc->PT_physical);
  printf("high exception vector flag: %x\n", gc->guestHighVectorSet);
  printf("registered exception vector:\n");
  printf("Und: %08x\n", gc->guestUndefinedHandler);
  printf("Swi: %08x\n", gc->guestSwiHandler);
  printf("Pabt: %08x\n", gc->guestPrefAbortHandler);
  printf("Dabt: %08x\n", gc->guestDataAbortHandler);
  printf("Unused: %08x\n", gc->guestUnusedHandler);
  printf("IRQ: %08x\n", gc->guestIrqHandler);
  printf("FIQ: %08x\n", gc->guestFiqHandler);
  printf("Hardware library: not core dumping just yet\n");
  printf("Interrupt pending: %x\n", gc->guestIrqPending);
  printf("Data abort pending: %x\n", gc->guestDataAbtPending);
  printf("Prefetch abort pending: %x\n", gc->guestPrefetchAbtPending);
  printf("Guest idle: %x\n", gc->guestIdle);
  printf("Block cache at: %08x\n", (u32int)gc->blockCache);

  int i = 0;
  printf("Block Trace: %x\n");
  for (i = BLOCK_HISOTRY_SIZE-1; i >= 0; i--)
  {
    printf("%x: %08x\n", i, gc->blockHistory[i]);
  
  }
  dumpSdramStats();
  printf("Debug Flag: %x\n", (u32int)gc->debugFlag);
}

GCONTXT * getGuestContext()
{
  return guest;
}
