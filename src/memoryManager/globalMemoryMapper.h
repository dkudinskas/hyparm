#ifndef __GLOBAL_MEMORY_MAPPER_H__
#define __GLOBAL_MEMORY_MAPPER_H__

#include "types.h"
#include "serial.h"
#include "guestContext.h"

#define STRH_IMM_MASK   0x0e5000f0
#define STRH_IMM_MASKED 0x004000b0
#define STRH_REG_MASK   0x0e500ff0
#define STRH_REG_MASKED 0x000000b0
// byte store
#define STRB_IMM_MASK   0x0e500000
#define STRB_IMM_MASKED 0x04400000
#define STRB_REG_MASK   0x0e500010
#define STRB_REG_MASKED 0x06400000
// word store
#define STR_REG_MASK    0x0e500010
#define STR_REG_MASKED  0x06000000
#define STR_IMM_MASK    0x0e500000
#define STR_IMM_MASKED  0x04000000
// dual word store
#define STRD_REG_MASK   0x0e500ff0
#define STRD_REG_MASKED 0x000000f0
#define STRD_IMM_MASK   0x0e5000f0
#define STRD_IMM_MASKED 0x004000f0
// multiple word store
#define STM_MASK        0x0e100000
#define STM_MASKED      0x08000000


// halfword load
#define LDRH_IMM_MASK   0x0e500090
#define LDRH_IMM_MASKED 0x00500090
#define LDRH_REG_MASK   0x0e500f90
#define LDRH_REG_MASKED 0x00100090
// byte load
#define LDRB_MASK       0x0c500000
#define LDRB_MASKED     0x04500000
// word load
#define LDR_MASK        0x0c500000
#define LDR_MASKED      0x04100000
// dual word load
#define LDRD_REG_MASK   0x0e500ff0
#define LDRD_REG_MASKED 0x000000d0
#define LDRD_IMM_MASK   0x0e5000f0
#define LDRD_IMM_MASKED 0x004000d0
// load multiple
#define LDM_MASK        0x0e100000
#define LDM_MASKED      0x08100000
// load exclusive
#define LDREX_MASK      0x0ff00fff
#define LDREX_MASKED    0x01900f9f


/* generic load store instruction emulation  *
 * called when we permission fault on memory *
 * access to a protected area - must emulate */
void emulateLoadStoreGeneric(GCONTXT * context, u32int address);

#endif
