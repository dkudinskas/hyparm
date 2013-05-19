#include "constants.h"

.section .text.unlikely

/*
 * callKernel
 *
 * 4 arguments are passed in r0-r3.
 * r0-r2 are unused here and must be passed as is to the guest.
 */
.global callKernel
callKernel:
  MOV    r4, #0
  MOV    r5, #0
  MOV    r6, #0
  MOV    r7, #0
  MOV    r8, #0
  MOV    r9, #0
  MOV    r10, #0
  MOV    r11, #0
  MOV    r12, #0

  MOV    PC, r3
