#ifndef __EXCEPTIONS__EXCEPTION_HANDLERS_H__
#define __EXCEPTIONS__EXCEPTION_HANDLERS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"

#include "memoryManager/mmu.h"


GCONTXT *softwareInterrupt(GCONTXT *context, u32int code);

GCONTXT *dataAbort(GCONTXT *context);
void dataAbortPrivileged(u32int pc, u32int sp, u32int spsr);

GCONTXT *undefined(GCONTXT *context);
void undefinedPrivileged(void);

GCONTXT *prefetchAbort(GCONTXT *context);
void prefetchAbortPrivileged(u32int pc, u32int sp, u32int spsr);

GCONTXT *monitorMode(GCONTXT *context);
void monitorModePrivileged(void);

GCONTXT *irq(GCONTXT *context);
void irqPrivileged(void);

void fiq(u32int addr);

void dabtTranslationFault(GCONTXT * gc, DFSR dfsr, u32int dfar);
void dabtPermissionFault(GCONTXT * gc, DFSR dfsr, u32int dfar);

void iabtTranslationFault(GCONTXT * gc, IFSR ifsr, u32int ifar);

#endif

