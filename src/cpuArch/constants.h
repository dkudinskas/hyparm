#ifndef __CPU_ARCH__CONSTANTS_H__
#define __CPU_ARCH__CONSTANTS_H__

/*
 * This file is suitable for inclusion by both C and assembly sources.
 *
 * DO NOT include ANY other file from here. Whatever you want to do, take it elsewhere.
 */


#define ARM_INSTRUCTION_SIZE           4
#define T16_INSTRUCTION_SIZE           2
#define T32_INSTRUCTION_SIZE           4


/*
 * A Thumb instruction is a 32-bit instruction if, in the first halfword, the bits in the mask
 * THUMB32 are set to one of THUMB32_[123].
 */
#define THUMB32                   0xF800
#define THUMB32_1                 0xE800
#define THUMB32_2                 0xF000
#define THUMB32_3                 0xF800


#define GPR_R0                         0
#define GPR_R1                         1
#define GPR_R2                         2
#define GPR_R3                         3
#define GPR_R4                         4
#define GPR_R5                         5
#define GPR_R6                         6
#define GPR_R7                         7
#define GPR_R8                         8
#define GPR_R9                         9
#define GPR_R10                       10
#define GPR_R11                       11
#define GPR_R12                       12
#define GPR_SP                        13
#define GPR_LR                        14
#define GPR_PC                        15


#define PSR_CC_FLAGS_NZCV     0xf0000000

#define PSR_CC_FLAG_N_BIT     0x80000000
#define PSR_CC_FLAG_Z_BIT     0x40000000
#define PSR_CC_FLAG_C_BIT     0x20000000
#define PSR_CC_FLAG_V_BIT     0x10000000
#define PSR_Q_BIT             0x08000000

#define PSR_ITSTATE_1_0       0x06000000

#define PSR_J_BIT             0x01000000

#define PSR_SIMD_FLAGS_GE     0x000f0000

#define PSR_ITSTATE_7_2       0x0000fc00

#define PSR_E_BIT                  0x200
#define PSR_A_BIT                  0x100
#define PSR_I_BIT                   0x80
#define PSR_F_BIT                   0x40
#define PSR_T_BIT                   0x20

#define PSR_MODE                    0x1f

#define PSR_USR_MODE                0x10
#define PSR_FIQ_MODE                0x11
#define PSR_IRQ_MODE                0x12
#define PSR_SVC_MODE                0x13
#define PSR_MON_MODE                0X16
#define PSR_ABT_MODE                0x17
#define PSR_UND_MODE                0x1b
#define PSR_SYS_MODE                0x1f

#define PSR_APSR         (PSR_CC_FLAGS_NZCV | PSR_Q_BIT | PSR_SIMD_FLAGS_GE)
#define PSR_EXEC_BITS    (PSR_ITSTATE_1_0 | PSR_ITSTATE_7_2 | PSR_J_BIT | PSR_T_BIT)

// System control register
#define SCTLR_TE    0x40000000  // Thumb Exception enable
#define SCTLR_VE    0x01000000  // ARM fiq/irq vs. impl defined vectors (deprecated)
#define SCTLR_V     0x00002000  // Low/high vecs (deprecated)
#define SCTLR_A     0x00000002  // Align check enable
#define SCTLR_M     0x00000001  // MMU enable

#define LR_OFFSET_IRQ             4
#define LR_OFFSET_DATA_ABT        8
#define LR_OFFSET_PREFETCH_ABT    4


#endif
