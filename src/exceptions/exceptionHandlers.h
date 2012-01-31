#ifndef __EXCEPTIONS__EXCEPTION_HANDLERS_H__
#define __EXCEPTIONS__EXCEPTION_HANDLERS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


#define SWI_END_OF_BLOCK      0x1


GCONTXT *softwareInterrupt(GCONTXT *context, u32int code);

GCONTXT *dataAbort(GCONTXT *context);
void dataAbortPrivileged(u32int pc);

GCONTXT *undefined(GCONTXT *context);
void undefinedPrivileged(void);

GCONTXT *prefetchAbort(GCONTXT *context);
void prefetchAbortPrivileged(void);

GCONTXT *monitorMode(GCONTXT *context);
void monitorModePrivileged(void);

GCONTXT *irq(GCONTXT *context);
void irqPrivileged(void);

void fiq(void);

#endif

