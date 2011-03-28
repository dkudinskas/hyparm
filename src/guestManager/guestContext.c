#include "guestManager/guestContext.h"

#include "vm/omap35xx/serial.h"
#include "vm/omap35xx/sdram.h"

#include "instructionEmu/commonInstrFunctions.h"
#include "instructionEmu/miscInstructions.h"

extern GCONTXT * getGuestContext( void );

void dumpGuestContext(GCONTXT * gc)
{
  u32int mode = gc->CPSR & 0x1F;
  serial_putstring("gc r0: ");
  serial_putint(gc->R0);
  serial_newline();
  serial_putstring("gc r1: ");
  serial_putint(gc->R1);
  serial_newline();
  serial_putstring("gc r2: ");
  serial_putint(gc->R2);
  serial_newline();
  serial_putstring("gc r3: ");
  serial_putint(gc->R3);
  serial_newline();
  serial_putstring("gc r4: ");
  serial_putint(gc->R4);
  serial_newline();
  serial_putstring("gc r5: ");
  serial_putint(gc->R5);
  serial_newline();
  serial_putstring("gc r6: ");
  serial_putint(gc->R6);
  serial_newline();
  serial_putstring("gc r7: ");
  serial_putint(gc->R7);
  serial_newline();
  if ( mode == 0x11 )
  {
    serial_putstring("gc R8_FIQ: ");
    serial_putint(gc->R8_FIQ);
    serial_newline();
    serial_putstring("gc R9_FIQ: ");
    serial_putint(gc->R9_FIQ);
    serial_newline();
    serial_putstring("gc R10_FIQ: ");
    serial_putint(gc->R10_FIQ);
    serial_newline();
    serial_putstring("gc R11_FIQ: ");
    serial_putint(gc->R11_FIQ);
    serial_newline();
    serial_putstring("gc R12_FIQ: ");
    serial_putint(gc->R12_FIQ);
    serial_newline();
  }
  else
  {
    serial_putstring("gc r8: ");
    serial_putint(gc->R8);
    serial_newline();
    serial_putstring("gc r9: ");
    serial_putint(gc->R9);
    serial_newline();
    serial_putstring("gc r10: ");
    serial_putint(gc->R10);
    serial_newline();
    serial_putstring("gc r11: ");
    serial_putint(gc->R11);
    serial_newline();
    serial_putstring("gc r12: ");
    serial_putint(gc->R12);
    serial_newline();
  }

  switch(mode)
  {
    case 0x10: // user
    case 0x1F: // system
      serial_putstring("gc R13_USR: ");
      serial_putint(gc->R13_USR);
      serial_newline();
      serial_putstring("gc R14_USR: ");
      serial_putint(gc->R14_USR);
      serial_newline();
      break;
    case 0x11: // fiq
      serial_putstring("gc R13_FIQ: ");
      serial_putint(gc->R13_FIQ);
      serial_newline();
      serial_putstring("gc R14_FIQ: ");
      serial_putint(gc->R14_FIQ);
      serial_newline();
      serial_putstring("gc SPSR_FIQ: ");
      serial_putint(gc->SPSR_FIQ);
      serial_newline();
      break;
    case 0x12: // irq
      serial_putstring("gc R13_IRQ: ");
      serial_putint(gc->R13_IRQ);
      serial_newline();
      serial_putstring("gc R14_IRQ: ");
      serial_putint(gc->R14_IRQ);
      serial_newline();
      serial_putstring("gc SPSR_IRQ: ");
      serial_putint(gc->SPSR_IRQ);
      serial_newline();
      break;
    case 0x13: // svc
      serial_putstring("gc R13_SVC: ");
      serial_putint(gc->R13_SVC);
      serial_newline();
      serial_putstring("gc R14_SVC: ");
      serial_putint(gc->R14_SVC);
      serial_newline();
      serial_putstring("gc SPSR_SVC: ");
      serial_putint(gc->SPSR_SVC);
      serial_newline();
      break;
    case 0x17: // abort
      serial_putstring("gc R13_ABT: ");
      serial_putint(gc->R13_ABT);
      serial_newline();
      serial_putstring("gc R14_ABT: ");
      serial_putint(gc->R14_ABT);
      serial_newline();
      serial_putstring("gc SPSR_ABT: ");
      serial_putint(gc->SPSR_ABT);
      serial_newline();
      break;
    case 0x1B: // undef
      serial_putstring("gc R13_UND: ");
      serial_putint(gc->R13_UND);
      serial_newline();
      serial_putstring("gc R14_UND: ");
      serial_putint(gc->R14_UND);
      serial_newline();
      serial_putstring("gc SPSR_UND: ");
      serial_putint(gc->SPSR_UND);
      serial_newline();
      break;
    default:
      serial_putstring("gc dump ARB: invalid mode in CPSR!");
      serial_newline();
      return;
  }

  serial_putstring("gc R15: ");
  serial_putint(gc->R15);
  serial_newline();
  serial_putstring("gc CPSR: ");
  serial_putint(gc->CPSR);
  serial_newline();


  serial_putstring("gc endOfBlockInstr: ");
  serial_putint(gc->endOfBlockInstr);
  serial_newline();
  serial_putstring("gc handler function addr: ");
  serial_putint((u32int)gc->hdlFunct);
  serial_newline();

  /* Virtual Memory */
  serial_putstring("gc  guest OS virtual addressing enabled: ");
  serial_putint_nozeros(gc->virtAddrEnabled);
  serial_newline();
  serial_putstring("gc guest OS Page Table: 0x");
  serial_putint((u32int)gc->PT_os);
  serial_newline();
  serial_putstring("gc guest OS Page Table (real): 0x");
  serial_putint((u32int)gc->PT_os_real);
  serial_newline();
  serial_putstring("gc guest OS shadow Page Table: 0x");
  serial_putint((u32int)gc->PT_shadow);
  serial_newline();
  serial_putstring("gc guest physical Page Table: 0x");
  serial_putint((u32int)gc->PT_physical);
  serial_newline();

  serial_putstring("gc high exception vector flag: ");
  serial_putint(gc->guestHighVectorSet);
  serial_newline();

  serial_putstring("gc  registered exception vector: ");
  serial_newline();

  serial_putstring("Und: ");
  serial_putint(gc->guestUndefinedHandler);
  serial_newline();

  serial_putstring("Swi: ");
  serial_putint(gc->guestSwiHandler);
  serial_newline();

  serial_putstring("Pabt: ");
  serial_putint(gc->guestPrefAbortHandler);
  serial_newline();

  serial_putstring("Dabt: ");
  serial_putint(gc->guestDataAbortHandler);
  serial_newline();

  serial_putstring("Unused: ");
  serial_putint(gc->guestUnusedHandler);
  serial_newline();

  serial_putstring("IRQ: ");
  serial_putint(gc->guestIrqHandler);
  serial_newline();

  serial_putstring("FIQ: ");
  serial_putint(gc->guestFiqHandler);
  serial_newline();

  serial_putstring("Hardware library: not core dumping just yet");
  serial_newline();

  serial_putstring("Interrupt pending: ");
  serial_putint(gc->guestIrqPending);
  serial_newline();

  serial_putstring("Data abort pending: ");
  serial_putint(gc->guestDataAbtPending);
  serial_newline();

  serial_putstring("Prefetch abort pending: ");
  serial_putint(gc->guestPrefetchAbtPending);
  serial_newline();

  serial_putstring("Guest idle: ");
  serial_putint(gc->guestIdle);
  serial_newline();

  serial_putstring("Block cache at: ");
  serial_putint((u32int)gc->blockCache);
  serial_newline();
  int i = 0;
  serial_putstring("Block Trace: ");
  serial_newline();
  for (i = BLOCK_HISOTRY_SIZE-1; i >= 0; i--)
  {
    serial_putint_nozeros(i);
    serial_putstring(": ");
    serial_putint(gc->blockHistory[i]);
    serial_newline();
  }
  dumpSdramStats();
}

void initGuestContext(GCONTXT * gContext)
{
  serial_putstring("Initializing guest context @ address ");
  //serial_putlong((u32int)&gContext);
  serial_putint( (u32int)getGuestContext() );
  serial_putstring(" blockTrace @ ");
  serial_putint( (u32int)(&gContext->blockHistory[0]) );
  serial_newline();
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
  gContext->R13_USR = 0;
  gContext->R14_USR = 0;
  gContext->R8_FIQ = 0;
  gContext->R9_FIQ = 0;
  gContext->R10_FIQ = 0;
  gContext->R11_FIQ = 0;
  gContext->R12_FIQ = 0;
  gContext->R13_FIQ = 0;
  gContext->R14_FIQ = 0;
  gContext->SPSR_FIQ = 0;
  gContext->R13_SVC = 0;
  gContext->R14_SVC = 0;
  gContext->SPSR_SVC = 0;
  gContext->R13_ABT = 0;
  gContext->R14_ABT = 0;
  gContext->SPSR_ABT = 0;
  gContext->R13_IRQ = 0;
  gContext->R14_IRQ = 0;
  gContext->SPSR_IRQ = 0;
  gContext->R13_UND = 0;
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
  int i = 0;
  for (i = 0; i < BLOCK_HISOTRY_SIZE; i++)
  {
    gContext->blockHistory[i] = 0;
  }
}

void registerCrb(GCONTXT * gc, CREG * coprocRegBank)
{
  gc->coprocRegBank = coprocRegBank;
  initCRB(gc->coprocRegBank);
}

void registerBlockCache(GCONTXT * gc, BCENTRY * blockCacheStart)
{
  serial_putstring("registerBlockCache: ");
  serial_putint((u32int)blockCacheStart);
  serial_newline();
  gc->blockCache = blockCacheStart;
  initialiseBlockCache(gc->blockCache);
}

void registerHardwareLibrary(GCONTXT * gc, device * libraryPtr)
{
  gc->hardwareLibrary = libraryPtr;
}


void registerMemoryProtection(GCONTXT * gc)
{
  gc->memProt = initialiseMemoryProtection();
}
