#ifndef __EXCEPTIONS__EXCEPTION_HANDLERS_H__
#define __EXCEPTIONS__EXCEPTION_HANDLERS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


#define SWI_END_OF_BLOCK      0x1

// uncomment me to enable exception handler debug : #define EXC_HDLR_DBG

GCONTXT *softwareInterrupt(GCONTXT *context, u32int code);

void dataAbort(GCONTXT *context);
void dataAbortPrivileged(u32int pc);
void undefined(void);
void undefinedPrivileged(void);
void prefetchAbort(GCONTXT *context);
void prefetchAbortPrivileged(void);
void monitorMode(void);
void monitorModePrivileged(void);
void irq(void);
void irqPrivileged(void);
void fiq(void);

#endif

