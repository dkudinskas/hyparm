#ifndef __MEMORY_MANAGER__GLOBAL_MEMORY_MAPPER_H__
#define __MEMORY_MANAGER__GLOBAL_MEMORY_MAPPER_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


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
// store exclusive
#define STREX_MASK      0x0ff00ff0
#define STREX_MASKED    0x01800f90

// halfword load
#define LDRH_IMM_MASK   0x0e500090
#define LDRH_IMM_MASKED 0x00500090
#define LDRH_REG_MASK   0x0e500f90
#define LDRH_REG_MASKED 0x00100090
// byte load
#define LDRB_IMM_MASK   0x0e500000
#define LDRB_IMM_MASKED 0x04500000
#define LDRB_REG_MASK   0x0e500010
#define LDRB_REG_MASKED 0x06500000
// word load
#define LDR_IMM_MASK    0x0e500000
#define LDR_IMM_MASKED  0x04100000
#define LDR_REG_MASK    0x0e500000
#define LDR_REG_MASKED  0x06100000
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


#ifdef CONFIG_THUMB2

/*
 * FIXME -- all bit patterns with C++ style comments need checking
 *
 * BEWARE - major bit and naming errors
 */

// ------------------Thumb 16 bit----------------- //

//STR Instructions
#define THUMB16_STR_IMM5_MASK      0xF800
#define THUMB16_STR_IMM5           0x6000
#define THUMB16_STR_IMM8_MASK      0xF800
#define THUMB16_STR_IMM8           0x9000

// STRB Instructions
#define THUMB16_STRB_IMM5_MASK     0xF800
#define THUMB16_STRB_IMM5          0x7000
#define THUMB16_STRB_REG_MASK      0xFE00
#define THUMB16_STRB_REG           0xA400

// STRH Instructions
#define THUMB16_STRH_IMM5_MASK     0xF800
#define THUMB16_STRH_IMM5          0x8000
#define THUMB16_STRH_REG_MASK      0xFE00
#define THUMB16_STRH_REG           0x5200

// STM, PUSH Instructions
#define THUMB16_PUSH_MASK          0xFE00
#define THUMB16_PUSH               0xB400
#define THUMB16_STM_MASK           0xF800
#define THUMB16_STM                0xC000

//LDR Instructions
#define THUMB16_LDR_IMM5_MASK      0xF800
#define THUMB16_LDR_IMM5           0x6800
#define THUMB16_LDR_IMM8_MASK      0xF800
#define THUMB16_LDR_IMM8           0x9800
#define THUMB16_LDR_IMM8_LIT_MASK  0xF800
#define THUMB16_LDR_IMM8_LIT       0x4800
#define THUMB16_LDR_REG_MASK       0xFE00
#define THUMB16_LDR_REG            0x5800

//LDRB Instructions
#define THUMB16_LDRB_IMM5_MASK     0xF800
#define THUMB16_LDRB_IMM5          0x7800
#define THUMB16_LDRB_REG_MASK      0xFE00
#define THUMB16_LDRB_REG           0xAC00

//LDRH Instructions
#define THUMB16_LDRH_IMM5_MASK     0xF800
#define THUMB16_LDRH_IMM5          0x8800
#define THUMB16_REG_MASK           0xFE00
#define THUMB16_REG                0x6A00

//LDRSH Instructions
#define  THUMB16_LDRSH_REG_MASK    0xFE00
#define THUMB16_LDRSH_REG          0xAE00


// --------------- Thumb 32 bit-------------------- //

//STRB Instructions
#define THUMB32_STRB_IMM12_MASK        0xFFF00000
#define THUMB32_STRB_IMM12             0xF8800000
#define THUMB32_STRB_IMM8_MASK         0xFFF08000
#define THUMB32_STRB_IMM8              0xF8008000
#define THUMB32_STRB_REG_MASK          0xFFF00FC0
#define THUMB32_STRB_REG               0xF8000000

//STRH
#define THUMB32_STRH_REG_IMM5_MASK     0xFFF00000
#define THUMB32_STRH_REG_IMM5          0xF8A00000
#define THUMB32_STRH_REG_IMM8_MASK     0xFFF00800
#define THUMB32_STRH_REG_IMM8          0xF8200800
#define THUMB32_STRH_REG_MASK          0xFFF00FC0
#define THUMB32_STRH_REG               0xF8200000

//STRD
#define THUMB32_STRD_IMM8_MASK         0xFE500000
#define THUMB32_STRD_IMM8              0xE8400000


/* LDRH immediate (8) T2 */
#define THUMB32_LDRH_REG_IMM12_MASK    0xFFF00000
#define THUMB32_LDRH_REG_IMM12         0xF8B00000

/* LDRH literal T1 */
#define THUMB32_LDRH_IMM12_MASK        0xFF7F0000
#define THUMB32_LDRH_IMM12             0xF83F0000

/* LDRH register T2 */
#define THUMB32_LDRH_REG_MASK          0xFFF00FC0
#define THUMB32_LDRH_REG               0xF8300000

/* LDRHT (unused?) */
#define THUMB32_LDRHT_REG_IMM8_MASK    0xFFF00F00
#define THUMB32_LDRHT_REG_IMM8         0xF8300E00

/*
 * LDRSH immediate (12) T1
 */
#define THUMB32_LDRSH_REG_IMM8_MASK    0xFFF00000
#define THUMB32_LDRSH_REG_IMM8         0xF9B00000

/*
 * LDRSH immediate (8) T2
 */
#define THUMB32_LDRSH_REG_IMM12_MASK   0xFFF00800
#define THUMB32_LDRSH_REG_IMM12        0xF9300800

/*
 * LDRSH literal T1
 */
#define THUMB32_LDRSH_IMM12_MASK       0xFF7F0000
#define THUMB32_LDRSH_IMM12            0xF93F0000

/*
 * LDRSH register T2
 */
#define THUMB32_LDRSH_REG_MASK         0xFFF00FC0
#define THUMB32_LDRSH_REG              0xF9300000


/*
 * LDRSHT T1 --unused--
 */
#define THUMB32_LDRSHT_MASK            0xFFF00F00
#define THUMB32_LDRSHT                 0xF9300E00

#endif /* CONFIG_THUMB2 */


/* generic load store instruction emulation  *
 * called when we permission fault on memory *
 * access to a protected area - must emulate */
void emulateLoadStoreGeneric(GCONTXT *context, u32int address);

#endif
