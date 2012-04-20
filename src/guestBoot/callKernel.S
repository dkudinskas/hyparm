#include "cpuArch/constants.h"

/*
 * callKernel
 *
 * 4 arguments are passed in r0-r3.
 * r0-r2 are unused here and must be passed as is to the guest.
 */
.global callKernel
callKernel:
  /*
   * Modify the SPSR: set USR mode, disable asynchronous aborts and FIQs
   */
  MRS    r4, SPSR
  BIC    r4, #PSR_MODE
  ORR    r4, #(PSR_USR_MODE | PSR_A_BIT | PSR_F_BIT)

#ifdef CONFIG_GUEST_TEST
  /*
   * If the LSB of the entry point is set, set the Thumb flag in the SPSR.
   */
  TST    r3, #0x1
  BICNE  r3, r3, #0x1
  ORRNE  r4, #PSR_T_BIT
#endif

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
