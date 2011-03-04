#ifndef __EXCEPTION_HANDLERS_H__
#define __EXCEPTION_HANDLERS_H__

#include "types.h"
#include "guestContext.h"

#define SWI_END_OF_BLOCK      0x1

// uncomment me to enable exception handler debug : #define EXC_HDLR_DBG

void softwareInterrupt(u32int code);

void dataAbort(void);
void dataAbortPrivileged(void);
void undefined(void);
void undefinedPrivileged(void);
void prefetchAbort(void);
void prefetchAbortPrivileged(void);
void monitorMode(void);
void monitorModePrivileged(void);
void irq(void);
void irqPrivileged(void);
void fiq(void);

#endif

