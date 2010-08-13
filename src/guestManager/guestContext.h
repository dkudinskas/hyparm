#ifndef __GUEST_CONTEXT_H__
#define __GUEST_CONTEXT_H__

#include "types.h"
#include "serial.h"
#include "cp15coproc.h"
#include "blockCache.h"
#include "pageTable.h" // for descriptor type
#include "memoryProtection.h"
#include "hardwareLibrary.h"

struct guestContext;
typedef struct guestContext GCONTXT;

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
  u32int (*hdlFunct)(GCONTXT * context);
  CREG * coprocRegBank;
  BCENTRY * blockCache;
  /* Virtual Addressing */
  descriptor* PT_physical; //guest physical to real physical PT
  descriptor* PT_os; //guest OS to guest Physical PT
  descriptor* PT_os_real; //physical addr of PT_os for H/W
  descriptor* PT_shadow; //guest OS to real physical PT
  bool virtAddrEnabled;
  /* Virtual Addressing end */
  MEMPROT* memProt;
  /* exception vector */
  u32int guestUndefinedHandler;
  u32int guestSwiHandler;
  u32int guestPrefAbortHandler;
  u32int guestDataAbortHandler;
  u32int guestUnusedHandler;
  u32int guestIrqHandler;
  u32int guestFiqHandler;
  device * hardwareLibrary;
  /* interrupt handling */
  bool guestIrqPending;
};



void dumpGuestContext(GCONTXT * gc);

void initGuestContext(GCONTXT * gContext);

void registerCrb(GCONTXT * gc, CREG * coprocRegBank);

void registerBlockCache(GCONTXT * gc, BCENTRY * blockCacheStart);

void registerHardwareLibrary(GCONTXT * gc, device * libraryPtr);

void registerMemoryProtection(GCONTXT * gc);

#endif
