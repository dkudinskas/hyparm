#ifndef __COMMON__THUMB_DEFS_H__
#define __COMMON__THUMB_DEFS_H__


#define T_BIT  0x20 // 5th bit of CPSR

// 3 different thumb-32 bit encodings.
#define THUMB32              0xF800
#define  THUMB32_1           0xE800
#define THUMB32_2            0xF000
#define THUMB32_3            0xF800

// markers to be used by endOfHalfBlock variable
#define THUMB32_LOW          0x1
#define THUMB32_HIGH         0x2
#define THUMB16              0x3

// used by scanner
#define INSTR_SWI            0xEF000000
#define INSTR_SWI_THUMB_MIX  0xDF00BF00 // Create a 32-bit instruction combining a SVC|NOP
#define INSTR_SWI_THUMB      0xDF00
#define INSTR_NOP_THUMB      0xBF00


#endif
