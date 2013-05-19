#include "cpuArch/constants.h"
#include "guestManager/guestConstants.h"


.text

/*
 * Entry point of the hypervisor.
 */
.global _start
_start:
  /*
   * Fix U-Shit: some smartass decided it was good to turn on MMU in the bootloader. WTF?
   * Also, when using GCC >= 4.7, unaligned memory accesses may be generated unless
   * CONFIG_BUILD_NO_UNALIGNED_ACCESS is set. Make sure alignment checking is disabled in that
   * case. All this must happen as early as possible to avoid madness later on.
   */
  MRC     p15, 0, r2, c1, c0, 0
  BIC     r2, r2, #(SCTLR_TE | SCTLR_VE)
  BIC     r2, r2, #SCTLR_V
#ifdef __GNUC__
#if !defined(CONFIG_BUILD_NO_UNALIGNED_ACCESS) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 7)))
  BIC     r2, r2, #SCTLR_A
#endif
#endif /* __GNUC__ */
  BIC     r2, r2, #SCTLR_M
  MCR     p15, 0, r2, c1, c0, 0
  ISB

  /*
   * The hypervisor is started through U-Boot, any command line arguments are passed as following:
   * - R0 contains the number of arguments;
   * - R1 is a pointer to an array of null-terminated C strings.
   * These arguments will be used by main and will be preserved if we don't touch R0 and R1.
   *
   * Zero-fill BSS section. Since the BSS section is aligned to a 32-byte boundary, we can safely
   * store 32 bytes at once.
   */
  LDR     r2, =__BSS_BEGIN__
  LDR     r3, =__BSS_END__
  MOV     r4, #0
  MOV     r5, #0
  MOV     r6, #0
  MOV     r7, #0
  MOV     r8, #0
  MOV     r9, #0
  MOV     r10, #0
  MOV     r11, #0
zeroFillBSS$:
  STMIA   r2!, {r4-r11}
  CMP     r2, r3
  BCC     zeroFillBSS$
  /* Set up stack pointer for all modes (SVC,FIQ,IRQ,ABT,UND,SYS/USR) and switch back to SVC mode.
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
    LDR     R2, =svcVector
    MCR     P15, 0, R2, C12, C0, 0
#else
  .err @ Architectures without the security extensions are not supported
#endif /* CONFIG_ARCH_EXT_SECURITY */
  /*
   * Set up SSP if enabled before we use any stacks...
   * TODO: make the guard value a random number.
   */
#ifdef CONFIG_BUILD_SSP
  MOVW    R3, #0x7ACC
  MOVT    R3, #0xBAD5
  MOV     R2, R3
  LDR     R3, =__stack_chk_guard
  STR     R2, [R3]
#endif /* CONFIG_BUILD_SSP */

#ifdef CONFIG_PROFILER
  CPSIE   F
#endif

  /*
   * Branch to main.
   * This function must never return.
   */
branchToMain$:
  B       main


/*
 * Provide a signal handler for libgcc
 */
.global raise
.func raise
raise:
  MOV     r1, lr
  B       dieFromSignal
.endfunc


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
  B       pabtHandler
  B       dabtHandler
  B       monHandler
  B       irqHandler
  B       fiqHandler

  .balign 0x20
  .global abortVector
abortVector:
  B       abortReset
  B       abortUndefined
  B       abortSvc
  B       abortPrefetch
  B       abortDataAbort
  B       abortMonitor
  B       abortIrq
  B       abortFiq

  .balign 0x20
  .global svcVector
svcVector:
  B       svcReset
  B       svcUndefined
  B       svcSvc
  B       svcPrefetch
  B       svcDataAbort
  B       svcMonitor
  B       svcIrq
  B       svcFiq

  .balign 0x20
  .global irqVector
irqVector:
  B       irqReset
  B       irqUndefined
  B       irqSvc
  B       irqPrefetch
  B       irqDataAbort
  B       irqMonitor
  B       irqIrq
  B       irqFiq

  .balign 0x20
  .global usrVector
usrVector:
  B       usrReset
  B       usrUndefined
  B       usrSvc
  B       usrPrefetch
  B       usrDataAbort
  B       usrMonitor
  B       usrIrq
  B       usrFiq

  .balign 0x20
  .global undVector
undVector:
  B       undReset
  B       undUndefined
  B       undSvc
  B       undPrefetch
  B       undDataAbort
  B       undMonitor
  B       undIrq
  B       undFiq
  .ltorg


#include "exceptions/macros.S.inc"
#include "exceptions/abortVector.S.inc"
  .ltorg
#include "exceptions/svcVector.S.inc"
#include "exceptions/irqVector.S.inc"
#include "exceptions/usrVector.S.inc"
#include "exceptions/undVector.S.inc"

  .ltorg

#endif /* CONFIG_ARCH_EXT_SECURITY */


/*
 * Emergency exception vector (depends on CONFIG_ARCH_EXT_SECURITY).
 */
#ifdef CONFIG_EMERGENCY_EXCEPTION_VECTOR

  .balign 0x20
  .global emergencyExceptionVectorBase
emergencyExceptionVectorBase:
#ifdef CONFIG_BKPT
  BKPT    #0xDEAD
#else
  B       emergencyExceptionVectorBase
#endif
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
  BX      LR

#endif /* CONFIG_EMERGENCY_EXCEPTION_VECTOR */

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
 * - R6: 0x1DF
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
   * Save SPSR into guest context CPSR field; exclude AIF bits and mode
   */
  MOVW    R6, #0x1DF
  AND     R2, R6
  MRS     R5, SPSR
  BIC     R5, R6
  ORR     R2, R2, R5
  STR     R2, [R1]
  /*
   * extract userm ode values of banked R13 and R14
   */
  STMIA   R4, {R13, R14}^

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
  LDMIA   R4, {SP, LR}
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
 * spsr = (context->cpsr & ~(I_BIT | MODE)) | A_BIT | F_BIT | USR_MODE;
 * guest_pc = context->pc;
 * if (spsr & PSR_T_BIT)
 * {
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
  /*
   * Get value of PC saved in guest context into LR
   */
  ADD     LR, LR, #GC_R15_OFFS
  LDR     LR, [LR]
  /*
   * Construct SPSR in R0: set user mode, disable asynchronous aborts and FIQs, but enable IRQs, and
   * restore all other fields from the CPSR
   */
#ifdef CONFIG_PROFILER
  BIC     R0, R0, #(PSR_I_BIT | PSR_MODE | PSR_F_BIT)
  ORR     R0, R0, #(PSR_A_BIT | PSR_USR_MODE)
#else
  BIC     R0, R0, #(PSR_I_BIT | PSR_MODE)
  ORR     R0, R0, #(PSR_A_BIT | PSR_F_BIT | PSR_USR_MODE)
#endif

#ifdef CONFIG_THUMB2
  /*
   * Also update guest PC when guest is in Thumb mode, then save SPSR
   */
  TST     R0, #PSR_T_BIT
  SUBNE   LR, LR, #2
#endif
  MSR     SPSR_cxsf, R0
  POP     {R0}
  /*
   * Save guest PC on stack, then perform exception return (pop PC and restore SPSR to CPSR)
   */
  STMDB   SP, {LR}
  LDMDB   SP, {PC}^
.endm


/*
 * restore_cpsr_pc_usr_mode
 *
 * Restore the guest CPSR and switch to guest.
 *
 * Pseudocode:
 * spsr = (context->cpsr & ~MODE) | USR_MODE;
 * guest_pc = context->pc;
 */
.macro restore_cpsr_pc_usr_mode
  /* FIXME: this can be done better */
  PUSH    {R0}
  /*
   * Get value of CPSR saved in guest context into R1
   */
  ADD     R0, LR, #GC_CPSR_OFFS
  LDR     R0, [R0]
  /*
   * Patch CPSR value (set user mode) and store to SPSR
   */
#ifdef CONFIG_PROFILER
  BIC     R0, R0, #(PSR_MODE | PSR_F_BIT)
#else
  BIC     R0, R0, #PSR_MODE
#endif
  ORR     R0, R0, #PSR_USR_MODE
  MSR     SPSR_cxsf, R0
  POP     {R0}
  /*
   * Get value of PC saved in guest context into LR
   */
  ADD     LR, LR, #GC_R15_OFFS
  LDR     LR, [LR]
  /*
   * Save guest PC on stack, then perform exception return (pop PC and restore SPSR to CPSR)
   */
  STMDB   SP, {LR}
  LDMDB   SP, {PC}^
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
     * Handle abort in SVC mode.
     */
    CPS    PSR_SVC_MODE
    BL     dataAbort
    CPS    PSR_ABT_MODE

    /* We came from usr mode (emulation or not of guest state) lets restore it and try that faulting instr again*/
    restore_r0_r14
    restore_cpsr_pc_usr_mode


.global dabtHandlerPriv
dabtHandlerPriv:
  /* we backed up the LR to hold the SPSR */
  POP    {LR}
  /* link register is last pc+8 */
  SUB    LR, LR, #8
  /* save common registers and return address. */
  PUSH    {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, LR}
  /* PC of the aborting instruction in @param1 of C handler */
  MOV    R0, LR
  MOV    R1, SP
  MRS    R2, SPSR
  AND    R2, R2, #PSR_MODE
  BL     dataAbortPrivileged
  POP    {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, LR}
  MOVS   PC, LR


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


.global pabtHandler
pabtHandler:
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
  save_pc

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
  /* we backed up the LR to hold the SPSR */
  POP     {LR}
  /* link register is last pc+4. */
  SUB     LR, LR, #4
  /* save common registers and return address. */
  PUSH    {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, LR}
  BL      irqPrivileged
  POP     {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, LR}
  MOVS    PC, LR


.global fiqHandler
fiqHandler:
  push    {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
  SUB    r0, lr, #4
  BL fiq
  pop     {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
  // TODO: Check if this works correctly in Thumb mode
  SUBS PC, LR, #4


/*
 * Space allocated in the BSS section is initialized to zero in _start.
 */
.bss

#ifdef CONFIG_STATS
.global timerStartSvc
timerStartSvc:
  .space 4
.global timerTotalSvc
timerTotalSvc:
  .space 4
.global timerNumberSvc
timerNumberSvc:
  .space 4

.global timerStartDabt
timerStartDabt:
  .space 4
.global timerTotalDabt
timerTotalDabt:
  .space 4
.global timerNumberDabt
timerNumberDabt:
  .space 4

.global timerStartIrq
timerStartIrq:
  .space 4
.global timerTotalIrq
timerTotalIrq:
  .space 4
.global timerNumberIrq
timerNumberIrq:
  .space 4

.global timerStartPabt
timerStartPabt:
  .space 4
.global timerTotalPabt
timerTotalPabt:
  .space 4
.global timerNumberPabt
timerNumberPabt:
  .space 4
#endif

/*
 * Allocate stack space. Note that stacks grown down, i.e., every PUSH lowers the value of SP, and
 * every POP increases the value of SP. Hence, labels have to be put after the reserved space.
 *
 * WARNING: EABI requires 8-byte aligned stacks (on public interfaces)!
 * WARNING: all stacks aligned on 4k boundaries for memory protection!
 */
.balign 4096
.global svcStack
.global svcStackEnd
.global svcStackGap
.global abtStack
.global abtStackEnd
.global abtStackGap
.global undStack
.global undStackEnd
.global undStackGap
.global irqStack
.global irqStackEnd
.global irqStackGap
.global fiqStack
.global fiqStackEnd
.global fiqStackGap
.global topStackGap

svcStackGap:
  .space 4096 /* SVC gap */
svcStackEnd:
  .space 4096 /* SVC stack */
svcStack:

abtStackGap:
  .space 4096 /* ABT gap */
abtStackEnd:
  .space 4096 /* ABT stack */
abtStack:

undStackGap:
  .space 4096 /* UND gap */
undStackEnd:
  .space 4096 /* UND stack */
undStack:

irqStackGap:
  .space 4096 /* IRQ gap */
irqStackEnd:
  .space 4096 /* IRQ */
irqStack:

fiqStackGap:
  .space 4096
fiqStackEnd:
  .space 4096 /* FIQ */
fiqStack:

topStackGap:
  .space 4096

/*
 * 32-bit space for the guest context pointer. Must be initialized to zero at start!
 */
.global activeGuestContext
activeGuestContext:
guestContextSpace:
  .space 4

/*
 * temporary location for register spilling!
 */
.global spillLocation
spillLocation:
  .space 4


/*
 * Guard value for GCC stack-smashing protection (SSP).
 *
 * WARNING: the placement is important -- we have descending stacks, so the guard value can never be
 * overwritten. Keep it like that!
 */
#ifdef CONFIG_BUILD_SSP

.global __stack_chk_guard
__stack_chk_guard:

  .space 4

#endif /* CONFIG_BUILD_SSP */
