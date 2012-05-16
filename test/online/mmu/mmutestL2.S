#include "sctlr.h"
#include "translationTable.h"

.syntax unified

.text
.arm

.global _start
_start:
B start

.global accessSecondPage
.global accessSmallPages
.global addL1Descriptors
.global addL2Descriptor
.global enableMMU
.global fail
.global l1Table
.global l2Table
.global mapSelf
.global remapWithSection
.global removeL1Descriptor
.global setupDACR
.global setupTTBR0
.global setupTTBCR
.global start
.global success
.global testSection

mapSelf:
/*
 * Map a 1 MB section for ourselves...
 */
LDR    r0, =_start
BFC    r0, #0, #20
ORR    r0, r0, #(L1_SECTION_ID | L1_SECTION_CACHEABLE)
ORR    r0, r0, #L1_SECTION_NON_SECURE
ORR    r0, r0, #0x400
LDR    r1, =l1Table
ADD    r1, #(2048*4)
STR    r0, [r1]
MOV    pc, lr

setupDACR:
/*
 * Set DACR to 0x1
 */
MOVW   r2, #1
MCR    p15, 0, r2, c3, c0, 0
BX     lr

setupTTBR0:
/*
 * Set TTBR0, we have N=0 in TTBCR and a 16 kB table so we take bits[31:14] of the table address
 */
LDR    r0, =l1Table
/* BFC    r0, #0, #14 */
MCR    p15, 0, r0, c2, c0, 0
BX     lr

setupTTBCR:
/*
 * Set TTBCR to perform table walks and not use TTBR1
 */
MOV    r0, #0
MCR    p15, 0, r0, c2, c0, 2
BX     lr

start:
/*
 * Set up exception vector
 */
CPSID  if
LDR    r0, =exceptionVector
MCR    p15, 0, r0, c12, c0, 0
/*
 * The next four operations can be performed in *any* order!
 */
BL     mapSelf
BL     setupDACR
BL     setupTTBR0
BL     setupTTBCR

enableMMU:
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

addL1Descriptors:
/*
 * Now start mapping the L2 table (full of fault entries) in all entries of the first half of the L1 table (2048 entries)
 */
LDR    r0, =l2Table
ORR    r1, r0, #9
LDR    r2, =l1Table
ADD    r3, r2, (2048 * 4)
fill$:
STR    r1, [r2], #4
CMP    r2, r3
BCC    fill$

addL2Descriptor:
/*
 * Now add a valid entry in the L2 table, PA = 0x80000000, privileged R/W, user no access
 */
MOV    r1, #0x80000000
ORR    r1, #0x12
STR    r1, [r0]
DSB

accessSmallPages:
/*
 * Shadow mapping is lazy so now force shadow mapping by accessing the one page mapped 2048 times above once in all mapped VA ranges
 */
MOV    r0, #0
MOV    r1, #0x80000000
access$:
LDR    r2, [r0], #0
ADD    r0, #(1024 * 1024)
CMP    r0, r1
BCC    access$

removeL1Descriptor:
/*
 * If we survived this, remove the first descriptor in the L1 table.
 */
LDR    r0, =l1Table
MOV    r1, #0
MCR    p15, 0, r1, c7, c11, 1
DSB
MCR    p15, 0, r1, c8, c7, 1
DSB
STR    r1, [r0], #0

accessSecondPage:
/*
 * Now access the second one (at 1 MB)
 */
MOV    r0, (1024 * 1024)
MOV    r1, #5
STR    r1, [r0], #0
ADD    r0, (1024 * 1024)
LDR    r2, [r0]
CMP    r1, r2
BEQ    readOk$
readFail$:
B      readFail$
readOk$:

remapWithSection:
/*
 * Remap first MB as section
 */
MOV    r0, #0x80000000
ORR    r0, r0, #0x2
ORR    r0, r0, #0x400
LDR    r1, =l1Table
STR    r0, [r1]
MOV    r0, 0
LDR    r0, [r0]
CMP    r0, #5
BEQ    remapOk$
remapFail$:
B      remapFail$
remapOk$:

testSection:
/*
 * Make sure first MB is readable beyond the first 4k
 */
MOV    r0, #(1024 * 8)
LDR    r0, [r0]

/*
 * Did we survive?
 */
success:
BKPT   #0
B      success
fail:
BKPT   #0xBAD
B      fail

.balign 0x20
.global exceptionVector
exceptionVector:
B      exceptionVector
B      exceptionVector
B      exceptionVector
BKPT   #0xA1
BKPT   #0xA2
/*prefetchAbort:
B      prefetchAbort
dataAbort:
B      dataAbort*/
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

/*
 * 1 kB L2 short-descriptor translation table
 */
.balign (4 * 256), 0xFF
.global l2Table
l2Table:
.space (4 * 256), 0
