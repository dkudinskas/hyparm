#ifndef __CPU_ARCH__CONSTANTS_H__
#define __CPU_ARCH__CONSTANTS_H__

/*
 * This file is suitable for inclusion by both C and assembly sources.
 *
 * DO NOT include ANY other file from here. Whatever you want to do, take it elsewhere.
 */


#define PSR_A_BIT            0x100
#define PSR_I_BIT             0x80
#define PSR_F_BIT             0x40
#define PSR_T_BIT             0x20

#define PSR_MODE              0x1f
#define PSR_NZCV        0xf0000000

#define PSR_USR_MODE          0x10
#define PSR_FIQ_MODE          0x11
#define PSR_IRQ_MODE          0x12
#define PSR_SVC_MODE          0x13
#define PSR_ABT_MODE          0x17
#define PSR_UND_MODE          0x1b
#define PSR_SYS_MODE          0x1f





/****************** all defines for CPSR (spsr's) ************/
/* random flags.. */
#define CPSR_CUM_SAT_FLAG     0x08000000
#define CPSR_IF_THEN_0_1      0x06000000
#define CPSR_JAZELLE_MODE     0x01000000
#define CPSR_RESERVED         0x00f00000
#define CPSR_GTE_FLAGS_SIMD   0x000f0000
#define CPSR_IF_THEN_2_7      0x0000fc00
#define CPSR_ENDIANNESS       0x00000200
/* ARM instruction condition field / values */
#define CPSR_CC_FIELD      0xF0000000
#define CC_EQ   0x0  // equals
#define CC_NE   0x1  // not equals
#define CC_HS   0x2  // carry set / unsigned higher or same
#define CC_LO   0x3  // carry not set / unsigned lower
#define CC_MI   0x4  // minus / negative / N set
#define CC_PL   0x5  // plus / positive or zero / N clear
#define CC_VS   0x6  // overflow / V set
#define CC_VC   0x7  // no overflow / V clear
#define CC_HI   0x8  // unsigned higher
#define CC_LS   0x9  // unsigned lower or same
#define CC_GE   0xA  // signed greater than or equals
#define CC_LT   0xB  // signed less than
#define CC_GT   0xC  // signed greater than
#define CC_LE   0xD  // signed less than or equal
#define CC_AL   0xE  // always
#define CC_NV   0xF  // never - should not be used! only special uncond instr
/* flag bits in CPSR */
#define CC_N_FLAG  0x8  // negative flag
#define CC_Z_FLAG  0x4  // zero flag
#define CC_C_FLAG  0x2  // carry flag
#define CC_V_FLAG  0x1  // oVerflow flag

#define SHIFT_TYPE_LSL    0x0
#define SHIFT_TYPE_LSR    0x1
#define SHIFT_TYPE_ASR    0x2
#define SHIFT_TYPE_RRX    0x3
#define SHIFT_TYPE_ROR    0x4



#endif
