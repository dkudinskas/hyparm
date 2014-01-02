#ifndef __VM__TYPES_H__
#define __VM__TYPES_H__

#include "common/types.h"


struct guestContext;

typedef u32int (*LOAD_FUNCTION)(struct guestContext *, ACCESS_SIZE, u32int, u32int);
typedef void (*STORE_FUNCTION)(struct guestContext *, ACCESS_SIZE, u32int, u32int, u32int);

typedef struct EmulatedVirtualMachine virtualMachine;

#endif /* __VM__TYPES_H__ */
