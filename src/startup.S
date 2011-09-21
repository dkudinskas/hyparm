#include "cpuArch/constants.h"

/* Standard definitions */
.equ GC_R0_OFFS,        0x00
.equ GC_R1_OFFS,        0x04
.equ GC_R2_OFFS,        0x08
.equ GC_R3_OFFS,        0x0C
.equ GC_R4_OFFS,        0x10
.equ GC_R5_OFFS,        0x14
.equ GC_R6_OFFS,        0x18
.equ GC_R7_OFFS,        0x1C
.equ GC_R8_OFFS,        0x20
.equ GC_R9_OFFS,        0x24
.equ GC_R10_OFFS,       0x28
.equ GC_R11_OFFS,       0x2C
.equ GC_R12_OFFS,       0x30
.equ GC_R13_OFFS,       0x34
.equ GC_R14_OFFS,       0x38
.equ GC_R15_OFFS,       0x3C
.equ GC_CPSR_OFFS,      0x40
.equ GC_R8_FIQ_OFFS,    0x44
.equ GC_R9_FIQ_OFFS,    0x48
.equ GC_R10_FIQ_OFFS,   0x4C
.equ GC_R11_FIQ_OFFS,   0x50
.equ GC_R12_FIQ_OFFS,   0x54
.equ GC_R13_FIQ_OFFS,   0x58
.equ GC_R14_FIQ_OFFS,   0x5C
.equ GC_SPSR_FIQ_OFFS,  0x60
.equ GC_R13_SVC_OFFS,   0x64
.equ GC_R14_SVC_OFFS,   0x68
.equ GC_SPSR_SVC_OFFS,  0x6C
.equ GC_R13_ABT_OFFS,   0x70
.equ GC_R14_ABT_OFFS,   0x74
.equ GC_SPSR_ABT_OFFS,  0x78
.equ GC_R13_IRQ_OFFS,   0x7C
.equ GC_R14_IRQ_OFFS,   0x80
.equ GC_SPSR_IRQ_OFFS,  0x84
.equ GC_R13_UND_OFFS,   0x88
.equ GC_R14_UND_OFFS,   0x8C
.equ GC_SPSR_UND_OFFS,  0x90


.text

/*
 * Entry point of the hypervisor.
 */
.global _start
_start:

  /*
   * The hypervisor is started through U-Boot, and any command line arguments are passed as following:
   * - R0 contains the number of arguments;
   * - R1 is a pointer to an array of null-terminated C strings.
   * These arguments will be used by main and will be preserved if we don't touch R0 and R1.
   *
   * Set up stack pointer for all modes (SVC,FIQ,IRQ,ABT,UND,SYS/USR) and switch back to SVC mode.
   * We do not make use of secure modes, so we do not set up a stack pointer for MON mode.
   * Hence, the hypervisor and its guests must never execute the SMC instruction.
   */
  CPSID   if, #PSR_FIQ_MODE
  LDR     SP, =fiqStack
  CPS     #PSR_IRQ_MODE
  LDR     SP, =irqStack
  CPS     #PSR_ABT_MODE
  LDR     SP, =abtStack
  CPS     #PSR_UND_MODE
  LDR     SP, =undStack
  CPS     #PSR_SYS_MODE
  MOV     SP, #0
  CPS     #PSR_SVC_MODE
  LDR     SP, =svcStack
  /*
   * Install exception handlers
   */
#ifdef CONFIG_ARCH_EXT_SECURITY
    /*
     * The security extensions introduce a register to set the exception vector base address (VBAR).
     * On the TI OMAP 35xx, this eliminates a few branches through the NAND and the SRAM.
     */
    LDR     R2, =exceptionVectorBase
    MCR     P15, 0, R2, C12, C0, 0
#else
# ifdef CONFIG_SOC_TI_OMAP_35XX
      /*
       * On the TI OMAP 35xx, U-Boot overwrites the default RAM exception vectors, which can normally be used as they are.
       * Restore them.
       */
      LDR     R2, [PC, #0x20]
      LDR     R3, [PC, #0x20]
      STR     R3, [R2], #4
      STR     R3, [R2], #4
      STR     R3, [R2], #4
      STR     R3, [R2], #4
      STR     R3, [R2], #4
      STR     R3, [R2], #4
      STR     R3, [R2], #4
      ADD     PC, PC, #4
      /*
       * The next two lines are data; they contain:
       * - the start address of the RAM exception vectors;
       * - the default instruction to be executed for all exceptions as defined in the TI OMAP 35xx TRM.
       */
      .word   0x4020FFC8
      LDR     PC, [PC, #0x14]
# else
    /*
     * Batch install exception handler addresses.
     * Set R2 to the first exception handler address to be written.
     * On the TI OMAP 35xx R2 is already correct.
     */
    LDR     R2, =exception_vector
    LDR     R2, [R2]
# endif /* CONFIG_SOC_TI_OMAP_35XX */
    LDR     R3, =undHandler
    STR     R3, [R2], #4
    LDR     R3, =svcHandler
    STR     R3, [R2], #4
    LDR     R3, =pabthandler
    STR     R3, [R2], #4
    LDR     R3, =dabtHandler
    STR     R3, [R2], #4
    LDR     R3, =monHandler
    STR     R3, [R2], #4
    LDR     R3, =irqHandler
    STR     R3, [R2], #4
    LDR     R3, =fiqHandler
    STR     R3, [R2], #4
#endif /* CONFIG_ARCH_EXT_SECURITY */
  /*
   * Branch to main.
   * This function must never return.
   */
  B       main


/*
 * Exception vector; only used in case security extensions are implemented.
 */
#ifdef CONFIG_ARCH_EXT_SECURITY

  .balign 0x20

  .global exceptionVectorBase
exceptionVectorBase:
  B       exceptionVectorBase
  B       undHandler
  B       svcHandler
  B       pabthandler
  B       dabtHandler
  B       monHandler
  B       irqHandler
  B       fiqHandler

#endif /* CONFIG_ARCH_EXT_SECURITY */


/*
 * Emergency exception vector (depends on CONFIG_ARCH_EXT_SECURITY).
 */
#ifdef CONFIG_EMERGENCY_EXCEPTION_VECTOR

  .balign 0x20
  .global emergencyExceptionVectorBase
emergencyExceptionVectorBase:
  B       emergencyExceptionVectorBase
  B       emergencyExceptionVectorBase
  B       emergencyExceptionVectorBase
  B       emergencyExceptionVectorBase
  B       emergencyExceptionVectorBase
  B       emergencyExceptionVectorBase
  B       emergencyExceptionVectorBase
  B       emergencyExceptionVectorBase

  .global setEmergencyExceptionVector
setEmergencyExceptionVector:
  LDR     R0, =emergencyExceptionVectorBase
  MCR     P15, 0, R0, C12, C0, 0
  MOV     PC, LR

#endif /* CONFIG_EMERGENCY_EXCEPTION_VECTOR */


.global getGuestContext
getGuestContext:
  LDR     R0, =guestContextSpace
  LDR     R0, [R0]
  MOV     PC, LR


.global setGuestContext
setGuestContext:
  LDR     R1, =guestContextSpace
  STR     R0, [R1]
  MOV     PC, LR


/*
 * get_emulated_mode
 *
 * In:
 * - R0: pointer to guest context
 * Out:
 * - R0: pointer to guest context
 * - R1: pointer to CPSR in guest context
 * - R2: value from CPSR in guest context
 * - R3: mode bits from CPSR in guest context
 * - R4: pointer to mode-dependent R13 field in guest-context
 */
.macro get_emulated_mode
  /*
   * Load CPSR from guest context into R1 to figure out emulated mode.
   */
  ADD     R1, R0, #GC_CPSR_OFFS
  LDR     R2, [R1]
  /*
   * Store pointer to mode-dependent R13 field of guest context in R4.
   */
  AND     R3, R2, #0x1F
  CMP     R3, #PSR_SYS_MODE
  ADDEQ   R4, R0, #GC_R13_OFFS
  CMP     R3, #PSR_USR_MODE
  ADDEQ   R4, R0, #GC_R13_OFFS
  CMP     R3, #PSR_FIQ_MODE
  ADDEQ   R4, R0, #GC_R13_FIQ_OFFS
  CMP     R3, #PSR_IRQ_MODE
  ADDEQ   R4, R0, #GC_R13_IRQ_OFFS
  CMP     R3, #PSR_SVC_MODE
  ADDEQ   R4, R0, #GC_R13_SVC_OFFS
  CMP     R3, #PSR_ABT_MODE
  ADDEQ   R4, R0, #GC_R13_ABT_OFFS
  CMP     R3, #PSR_UND_MODE
  ADDEQ   R4, R0, #GC_R13_UND_OFFS
.endm

/*
 * save_cc_r0_to_r14
 *
 * In:
 * - LR must be PUSHed on stack
 * Out:
 * - R0: pointer to guest context
 * - R1: pointer to CPSR in guest context
 * - R2: pointer to mode-dependent R8 field in guest context
 * - R3: mode bits from CPSR in guest context
 * - R4: pointer to mode-dependent R13 field in guest-context
 * - R5: condition flags extracted from SPSR
 */
.macro save_cc_r0_to_r14
  /*
   * Load guest context into LR and store R0--R7 into guest context.
   */
  LDR     LR, =guestContextSpace
  LDR     LR, [LR]
  STMIA   LR, {R0-R7}
  /*
   * R0--R7 is now usable, move guest context in there.
   */
  MOV     R0, LR
  /*
   * Restore the original value of LR (has to be PUSHed earlier)
   */
  POP     {LR}
  /*
   * Load CPSR from guest context and figure out the pointer to the mode-dependent R13 field
   */
  get_emulated_mode
  /*
   * Save condition flags from SPSR into guest context CPSR field; exclude the other bits (AIF)
   */
  AND     R2, #0x0FFFFFFF
  MRS     R5, SPSR
  AND     R5, #0xF0000000
  ORR     R2, R2, R5
  STR     R2, [R1]
  /*
   * Temporarily switch to system mode to extract the values of banked R13 and R14
   */
  MRS     R2, CPSR
  CPS     PSR_SYS_MODE
  STMIA   R4, {R13, R14}
  MSR     CPSR, R2
  /*
   * Store R8--R12; the location depends on whether we were emulating FIQ mode
   */
  CMP     R3, #0x11
  ADDNE   R2, R0, #GC_R8_OFFS
  ADDEQ   R2, R0, #GC_R8_FIQ_OFFS
  STMIA   R2, {R8-R12}
.endm

/*
 * save_pc
 *
 * In:
 * - R0: pointer to guest context
 * - R1: pointer to CPSR in guest context
 * Out:
 * - R0: pointer to guest context
 * - R1: pointer to PC in guest context
 * - R2: value of PC in guest context
 * - R3: value of CPSR in guest context
 */
.macro save_pc
  /*
   * Copy guest PC to R2
   */
  MOV     R2, LR
#ifdef CONFIG_THUMB2
  /*
   * Load CPSR value from guest context
   */
  LDR     R3, [R1]
  TST     R3, #PSR_T_BIT
  SUBEQ   R2, R2, #4 @ARM
  SUBNE   R2, R2, #2 @Thumb
#else
  SUB     R2, R2, #4
#endif
  ADD     R1, R0, #GC_R15_OFFS
  STR     R2, [R1]
.endm

.macro save_pc_abort
  /* store guest PC */
  MOV     R2, LR
  SUB     R2, R2, #8
  ADD     R1, R0, #GC_R15_OFFS
  STR     R2, [R1]
.endm


/*
 * restore_r0_r14
 *
 * In:
 * - R0: pointer to guest context
 * Out:
 * - R0--R12: restored register values from guest context
 * - R14: pointer to guest context
 */
.macro restore_r0_r14
  /* Use guest CPSR to work out which mode we are meant to be emulating */
  get_emulated_mode
  /*
   * Temporarily switch to system mode to restore the values of R13 and R14 to the banked registers
   */
  MRS     R1, CPSR
  CPS     PSR_SYS_MODE
  LDR     SP, [R4]
  LDR     LR, [R4, #4]
  MSR     CPSR, R1
  /*
   * Move some things out of the way:
   * - R8: pointer to guest context
   * - R14: mode bits from CPSR in guest context
   */
  MOV     R8, R3
  MOV     LR, R0
  /*
   * Restore general purpose registers R0--R7 common to all modes
   */
  LDMIA   LR, {R0-R7}
  /*
   * Restore mode-dependent registers R8--R12
   */
  CMP     R8, #PSR_FIQ_MODE
  ADDNE   R8, LR, #GC_R8_OFFS
  ADDEQ   R8, LR, #GC_R8_FIQ_OFFS
  LDMIA   R8, {R8-R12}
.endm


/*
 * restore_cpsr_pc_usr_mode_irq
 *
 * Restore the guest CPSR and switch to guest.
 *
 * Pseudocode:
 * spsr = ((context->cpsr & 0xf0000000) | A_BIT | F_BIT) | USR_MODE;
 * guest_pc = context->pc;
 * if (context->cpsr & PSR_T_BIT)
 * {
 *   spsr |= PSR_T_BIT;
 *   guest_pc -= 2;
 * }
 */
.macro restore_cpsr_pc_usr_mode_irq
  PUSH    {R0}
  /*
   * Get value of CPSR saved in guest context into R0
   */
  ADD     R0, LR, #GC_CPSR_OFFS
  LDR     R0, [R0]
#ifdef CONFIG_THUMB2
  /*
   * Get value of PC saved in guest context into LR
   */
  ADD     LR, LR, #GC_R15_OFFS
  LDR     LR, [LR]
  /*
   * Construct SPSR in R0: restore condition flags from the value of the CPSR saved in the guest
   * context, set user mode, disable asynchronous aborts and FIQs, but enable IRQs, and restore the
   * Thumb bit
   */
  TST     R0, #PSR_T_BIT
  AND     R0, R0, #PSR_NZCV
  ORREQ   R0, R0, #(PSR_USR_MODE | PSR_A_BIT | PSR_F_BIT)
  ORRNE   R0, R0, #(PSR_USR_MODE | PSR_A_BIT | PSR_F_BIT | PSR_T_BIT)
  /*
   * Also update guest PC when guest is in Thumb mode, then save SPSR
   */
  SUBNE   LR, LR, #2
  MSR     SPSR, R0
  POP     {R0}
#else
  /*
   * Construct SPSR in R0: restore condition flags from the value of the CPSR saved in the guest
   * context, set user mode, disable asynchronous aborts and FIQs, but enable IRQs, then save SPSR
   */
  AND     R0, R0, #PSR_NZCV
  ORR     R0, R0, #(PSR_USR_MODE | PSR_A_BIT | PSR_F_BIT)
  MSR     SPSR, R0
  POP     {R0}
  /*
   * Get value of PC saved in guest context into LR
   */
  ADD     LR, LR, #GC_R15_OFFS
  LDR     LR, [LR]
#endif
  /*
   * Save guest PC on stack, then perform exception return (pop PC and restore SPSR to CPSR)
   */
  STM     SP, {LR}
  LDM     SP, {PC}^
.endm


/*
 * restore_cpsr_pc_usr_mode
 *
 * Restore the guest CPSR and switch to guest.
 *
 * Pseudocode:
 * spsr = (context->cpsr & (0xf0000000 | A_BIT | F_BIT)) | USR_MODE;
 * guest_pc = context->pc;
 * if (context->cpsr & PSR_T_BIT)
 * {
 *   spsr |= PSR_T_BIT;
 * }
 */
.macro restore_cpsr_pc_usr_mode
  /* FIXME: this can be done better */
  PUSH    {R0, R1}
  /*
   * Get value of CPSR saved in guest context into R1
   */
  ADD     R1, LR, #GC_CPSR_OFFS
  LDR     R1, [R1]
  /* Extract condition flags into R0 */
  AND     R0, R1, #PSR_NZCV
#ifdef CONFIG_THUMB2
  /* Extract exception flags & Thumb state*/
  AND     R1, R1, #(PSR_A_BIT | PSR_I_BIT | PSR_F_BIT | PSR_T_BIT)
#else
  /* Extract exception flags */
  AND     R1, R1, #(PSR_A_BIT | PSR_I_BIT | PSR_F_BIT)
#endif
  /* R0= exc flags|thumb state|cc */
  ORR     R0, R1, R0
  /* set user mode */
  ORR     R0, R0, #PSR_USR_MODE
  MSR     SPSR, R0
  POP     {R0, R1}
  /*
   * Get value of PC saved in guest context into LR
   */
  ADD     LR, LR, #GC_R15_OFFS
  LDR     LR, [LR]
  /*
   * Save guest PC on stack, then perform exception return (pop PC and restore SPSR to CPSR)
   */
  STM     SP, {LR}
  LDM     SP, {PC}^
.endm


.global svcHandler
svcHandler:
    /* We can NOT assume that the data abort is guest code */
    push   {LR}

    save_cc_r0_to_r14 /* pops LR */
    save_pc

    /*
     * GCONTXT *softwareInterrupt(GCONTXT *gContext, u32int code);
     * Guest context pointer is already in R0!
     * Store SVC code in R1.
     */
#ifdef CONFIG_THUMB2
    ADD     R1, R0, #GC_CPSR_OFFS
    LDR     R1, [R1]
    AND     R2, R1, #PSR_T_BIT
    CMP     R2, #PSR_T_BIT
    LDRNE   R1, [LR, #-4] @Thumb bit = 0
    ANDNE   R1, #0xFFFFFF
    LDREQB  R1, [LR, #-2]
    ANDEQ   R1, #0x00FF
#else
    LDR     R1, [LR, #-4]
    AND     R1, #0xFFFFFF
#endif
    BL      softwareInterrupt

    restore_r0_r14
    restore_cpsr_pc_usr_mode

.global dabtHandler
dabtHandler:
    /* We can NOT assume that the data abort is guest code */
    Push   {LR}
    /* Test SPSR -> are we from USR mode? */
    MRS    LR, SPSR
    AND    LR, LR, #PSR_MODE
    CMP    LR, #PSR_USR_MODE

    BNE    dabtHandlerPriv

    /* We were in USR mode, we must have been running guest code */
    save_cc_r0_to_r14 /* pops LR */
    /* Get the instr that aborted, after we fix up we probably want to re-try it */
    save_pc_abort

    /*
     * Pointer to guest context is already in R0!
     */
    BL     dataAbort

    /* We came from usr mode (emulation or not of guest state) lets restore it and try that faulting instr again*/
    restore_r0_r14
    restore_cpsr_pc_usr_mode

.global dabtHandlerPriv
dabtHandlerPriv:
  POP    {LR}
  /* SEGFAULT in the hypervisor */
  PUSH   {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12}
  /* PC of the aborting instruction in @param1 of C handler */
  SUB    R0, LR, #8
  BL     dataAbortPrivileged
  /* SEGFAULT in hypervisor: should never return from that handler */
dabtPrivLoop:
  B      dabtPrivLoop


.global undHandler
undHandler:
  PUSH   {LR}
  MRS    LR, SPSR
  AND    LR, LR, #PSR_MODE
  CMP    LR, #PSR_USR_MODE
  BNE    undHandlerPriv /* Abort occured in Hypervisor (privileged) code */

  /* We were in USR mode, we must have been running guest code */
  save_cc_r0_to_r14 /* pops LR */
  save_pc_abort

  BL undefined

  /* We came from usr mode (emulation or not of guest state) lets restore it and resume */
  restore_r0_r14
  restore_cpsr_pc_usr_mode
  /* End restore code */

.global undHandlerPriv
undHandlerPriv:
  /* SEGFAULT in the hypervisor */
  PUSH   {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12}
  /* PC of the aborting instruction in @param1 of C handler */
  SUB    R0, LR, #4
  BL     undefinedPrivileged
  /* SEGFAULT in hypervisor: should never return from that handler */
undPrivLoop:
  B      undPrivLoop


.global pabthandler
pabthandler:
  /* We can NOT assume that the abort is guest code */
  PUSH   {LR}
  /* Test SPSR -> are we from USR mode? */
  MRS    LR, SPSR
  AND    LR, LR, #PSR_MODE
  CMP    LR, #PSR_USR_MODE
  BNE    pabtHandlerPriv

  /* We were in USR mode, we must have been running guest code */
  save_cc_r0_to_r14 /* pops LR */
  /* Get the instr that aborted, after we fix up we probably want to re-try it */
  save_pc_abort

  /*
   * Pointer to guest context is already in R0!
   */
  BL     prefetchAbort

  /* We came from usr mode (emulation or not of guest state) lets restore it and try that faulting instr again*/

  restore_r0_r14
  restore_cpsr_pc_usr_mode
  /* End restore code */

.global pabtHandlerPriv
pabtHandlerPriv:
  /* SEGFAULT in the hypervisor */
  PUSH   {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12}
  /* PC of the aborting instruction in @param1 of C handler */
  SUB    R0, LR, #4
  BL     prefetchAbortPrivileged
  /* SEGFAULT in hypervisor: should never return from that handler */
pabtPrivLoop:
  B      pabtPrivLoop

.global monHandler
monHandler:
  PUSH   {LR}
  MRS    LR, SPSR
  AND    LR, LR, #PSR_MODE
  CMP    LR, #PSR_USR_MODE
  BNE    monHandlerPriv /* Call occured in Hypervisor (privileged) code */

  /* We were in USR mode, we must have been running guest code */
  save_cc_r0_to_r14 /* pops LR */
  /* Get the instr that aborted, after we fix up we probably want to re-try it */
  save_pc_abort

  BL monitorMode

  /* We came from usr mode (emulation or not of guest state) lets restore it and try that faulting instr again*/

  restore_r0_r14
  restore_cpsr_pc_usr_mode
  /* End restore code */

.global monHandlerPriv
monHandlerPriv:
  /* SEGFAULT in the hypervisor */
  PUSH   {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12}
  /* PC of the aborting instruction in @param1 of C handler */
  SUB    R0, LR, #4
  BL monitorModePrivileged
  /* SEGFAULT in hypervisor: should never return from that handler */
monPrivLoop:
  B      monPrivLoop


.global irqHandler
irqHandler:
  /* disable further interrupts */
  CPSID  i

  /* need to check if we came from guest mode, or were inside the hypervisor */
  PUSH   {LR}
  MRS    LR, SPSR
  AND    LR, LR, #PSR_MODE
  CMP    LR, #PSR_USR_MODE
  /* interrupt occured whilst running hypervisor */
  BNE    irqHandlerPriv

  /* We were in USR mode running guest code, need to save context */
  save_cc_r0_to_r14 /* pops LR */
  /* save the PC of the guest, during which we got the interrupt */
  save_pc
  BL irq

  restore_r0_r14
  restore_cpsr_pc_usr_mode_irq
  /* End restore code */

irqHandlerPriv:
  pop     {LR} /* we backed up the LR to hold the SPSR */
  /* link register is last pc+8. need to return to the which?? instruction */
  sub     LR, LR, #4
  /* save common registers and return address. */
  push    {lr}
  push    {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12}
  BL      irqPrivileged
  pop     {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12}
  ldm     sp!, {pc}^


.global fiqHandler
fiqHandler:
  /* this is not implemented really */
  STMFD SP!, {LR}
  STMFD SP!, {R0-R12}

  BL fiq

  LDMFD SP!, {R0-R12}
  LDMFD SP!, {PC}^


.data

#ifndef CONFIG_SOC_TI_OMAP_35XX
exception_vector:
  .err @ Unknown target
#endif

/*
 * 32-bit space for the guest context pointer.
 * This space must be in a .data section to make sure it is initialized to zero.
 */
guestContextSpace:
  .space 4


/*
 * Space allocated in the BSS section is *not* initialized to zero.
 */
.bss

/*
 * Allocate stack space. Note that stacks grown down, i.e., every PUSH lowers the value of SP, and
 * every POP increases the value of SP. Hence, labels have to be put after the reserved space.
 *
 * WARNING: EABI requires 8-byte aligned stacks!!!
 */
  .balign 8
stackSpaceEnd:
  .space 1024 /* SVC */
svcStack:
  .space 1024 /* ABT */
abtStack:
  .space 1024 /* UND */
undStack:
  .space 1024 /* IRQ */
irqStack:
  .space 1024 /* FIQ */
fiqStack:
stackSpaceBegin:
