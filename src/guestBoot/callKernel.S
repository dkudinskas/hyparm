#include "cpuArch/constants.h"

.section .text.unlikely

/*
 * callKernel
 *
 * 4 arguments are passed in r0-r3.
 * r0-r2 are unused here and must be passed as is to the guest.
 */
.global callKernel
callKernel:

#ifdef CONFIG_STATS
  MOV     R8, #1
  MCR     p15, 0, R8, c9, c12, 5  /* select performance counter 1  */
  MOV     R8, #1
  MCR     p15, 0, R8, c9, c13, 1  /* get it to count i-cache l1 misses   */

  MOV     R8, #2
  MCR     p15, 0, R8, c9, c12, 5  /* select performance counter 2  */
  MOV     R8, #3
  MCR     p15, 0, R8, c9, c13, 1  /* get it to count d-cache l1 misses   */

  MOV     R8, #3
  MCR     p15, 0, R8, c9, c12, 5  /* select performance counter 3  */
  MOV     R8, #0x44
  MCR     p15, 0, R8, c9, c13, 1  /* get it to count l2 misses   */

  MOV     R8, #0
  ORR     R8, R8, #1              /* enable all counters */
  ORR     R8, R8, #2              /* reset all performance counters to 0 */
  ORR     R8, R8, #4              /* reset cycle counter to 0 */
  ORR     R8, R8, #8              /* ENABLE the divider (64)*/
  ORR     R8, R8, #16             /* not sure: enable export events? */
  MCR     p15, 0, R8, c9, c12, 0  /* Write PMNC Register */

  MOVW    R8, #0x000f
  MOVT    R8, #0x8000
  MCR     p15, 0, R8, c9, c12, 1  /* enable all counters */
  MOV     R7, #0
  MCR     p15, 0, R7, c9, c12, 2  /* enable all counters */
  MCR     p15, 0, R8, c9, c12, 3  /* clear all overflow flags */
#endif

  /* make sure counter 0 is disabled */
  MOV     R8, #1
  MCR     p15, 0, R8, c9, c12, 2

  /* select performance counter 0 */
  MOV     R8, #0
  MCR     p15, 0, R8, c9, c12, 5

  /* reset counter zero */
  MCR     p15, 0, R8, c9, c13, 2

  /* user enable peft counters */
  MOV     R8, #1
  MCR     p15, 0, R8, c9, c14, 0

  /* performance counter 0 reserved for spilling */
  MOV     R8, #0
  MCR     p15, 0, R8, c9, c12, 5


  /*
   * Modify the SPSR: set USR mode, disable asynchronous aborts and FIQs
   */
  MRS    r4, SPSR
#ifdef CONFIG_PROFILER
  BIC    r4, #(PSR_MODE | PSR_F_BIT)
  ORR    r4, #(PSR_USR_MODE | PSR_A_BIT)
#else
   BIC    r4, #PSR_MODE
   ORR    r4, #(PSR_USR_MODE | PSR_A_BIT | PSR_F_BIT)
#endif

#ifdef CONFIG_THUMB2
  /*
   * If the LSB of the entry point is set, set the Thumb flag in the SPSR.
   */
  TST    r3, #0x1
  BICNE  r3, r3, #0x1
  ORRNE  r4, #PSR_T_BIT
#endif /* CONFIG_THUMB2 */

  MSR    SPSR, r4

  /* Load the entry point onto the stack, then use the Load PC + copy SPSR to CPSR to jump into USR mode */
  LDR    SP, =svcStack
  STMDB  SP, {r3}

  /*
   * Prevent leaking hypervisor data to guest (this also improves determinism)
   */
  MOV    r4, #0
  MOV    r5, #0
  MOV    r6, #0
  MOV    r7, #0
  MOV    r8, #0
  MOV    r9, #0
  MOV    r10, #0
  MOV    r11, #0
  MOV    r12, #0

  LDMDB  SP, {PC}^
