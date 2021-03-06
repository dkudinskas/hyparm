#ifndef __GUEST_MANAGER__GUEST_EXCEPTIONS_H__
#define __GUEST_MANAGER__GUEST_EXCEPTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


#define EXC_VECT_HIGH_OFFS  0xffff0000

#define EXC_VECT_LOW_SVC   0x08
#define EXC_VECT_LOW_PABT  0x0C
#define EXC_VECT_LOW_DABT  0x10
#define EXC_VECT_LOW_IRQ   0x18


// no need to throw a service call to guest context.
// hypercall handler deals with it.
void deliverServiceCall(GCONTXT* context);

void throwInterrupt(GCONTXT* context, u32int irqNumber);
void deliverInterrupt(GCONTXT* context);

void throwDataAbort(GCONTXT* context, u32int address, u32int faultType, bool isWrite, u32int domain);
void deliverDataAbort(GCONTXT* context);

void throwPrefetchAbort(GCONTXT* context, u32int address, u32int faultType);
void deliverPrefetchAbort(GCONTXT* context);

#endif
