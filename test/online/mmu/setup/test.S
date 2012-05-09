#include "sctlr.h"
#include "translationTable.h"

.syntax unified

.text
.arm

_start:
B start

mapSelf:
/*
 * Map a 1 MB section for ourselves...
 */
LDR    r0, =_start
BFC    r0, #0, #20
ORR    r0, r0, #(L1_SECTION_ID | L1_SECTION_CACHEABLE)
ORR    r0, r0, #L1_SECTION_NON_SECURE
LDR    r1, =l1Table
ADD    r1, #(TODO depending on base addr * 4)
STR    r0, [r1]
MOV    pc, lr

setupTTBR0:
/*
 * Set TTBR0, we have N=0 in TTBCR and a 16 kB table so we take bits[31:14] of the table address
 */
LDR    r0, =l1Table
/* BFC    r0, #0, #14 */
MCR    p15, 0, r0, c2, c0, 0
MOV    pc, lr

setupTTBCR:
/*
 * Set TTBCR to perform table walks and not use TTBR1
 */
MOV    r0, #0
MCR    p15, 0, r0, c2, c0, 2
MOV    pc, lr

start:
/*
 * Set up exception vector
 */
CPSID  if
LDR    r0, =exceptionVector
MCR    p15, 0, r0, c12, c0, 0
/*
 * The next three operations can be performed in *any* order!
 */
BL     mapSelf
BL     setupTTBR0
BL     setupTTBCR
/*
 * Enable MMU in SCTLR
 */
MRC    p15, 0, r0, c1, c0, 0
MOVW   r1, #(SCTLR_ALIGNMENT_CHECK_ENABLE | SCTLR_CP15_BARRIER_ENABLE | SCTLR_SW_ENABLE | SCTLR_HIVECS_ENABLE)
MOVT   r1, #((SCTLR_FI_LOW_LATENCY_CONFIGURATION | SCTLR_EXCEPTION_ENDIANNESS | SCTLR_TEX_REMAP_ENABLE | SCTLR_ACCESS_FLAG_ENABLE | SCTLR_THUMB_EXCEPTION_ENABLE) >> 16)
BIC    r0, r0, r1
MOVW   r1, #(SCTLR_MMU_ENABLE | SCTLR_BRANCH_PREDICTION_ENABLE | SCTLR_CACHE_ENABLE | SCTLR_ICACHE_ENABLE)
ORR    r0, r0, r1
MCR    p15, 0, r0, c1, c0, 0
ISB    SY
/*
 * Did we survive?
 */
success:
BKPT   #0
B      success


.balign 0x20
.global exceptionVector
exceptionVector:
B      exceptionVector
B      exceptionVector
B      exceptionVector
BKPT   #0xA1
BKPT   #0xA2
B      exceptionVector
B      exceptionVector
B      exceptionVector


.data

/*
 * 16 kB L1 short-descriptor translation table
 */
.balign (4 * 4096), 0xFF
.global l1Table
l1Table:
.space (4 * 4096), 0
