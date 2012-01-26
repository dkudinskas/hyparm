/* Standard definitions */

.equ A_BIT,            0x100     /* when set, async aborts are disabled */
.equ I_BIT,             0x80      /* when set, IRQ is disabled */
.equ F_BIT,             0x40      /* when set, FIQ is disabled */

.equ USR_MODE,          0x10
.equ FIQ_MODE,          0x11
.equ IRQ_MODE,          0x12
.equ SVC_MODE,          0x13
.equ ABT_MODE,          0x17
.equ UND_MODE,          0x1B
.equ SYS_MODE,          0x1F

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
  .ifdef CONFIG_ARCH_V7_A
    CPSID   if, #FIQ_MODE
    LDR     SP, =fiqStack
    CPS     #IRQ_MODE
    LDR     SP, =irqStack
    CPS     #ABT_MODE
    LDR     SP, =abtStack
    CPS     #UND_MODE
    LDR     SP, =undStack
    CPS     #SYS_MODE
    LDR     SP, =usrStack
    CPS     #SVC_MODE
  .else
    MSR     CPSR_c, #(FIQ_MODE | I_BIT | F_BIT)
    LDR     SP, =fiqStack
    MSR     CPSR_c, #(IRQ_MODE | I_BIT | F_BIT)
    LDR     SP, =irqStack
    MSR     CPSR_c, #(ABT_MODE | I_BIT | F_BIT)
    LDR     SP, =abtStack
    MSR     CPSR_c, #(UND_MODE | I_BIT | F_BIT)
    LDR     SP, =undStack
    MSR     CPSR_c, #(SYS_MODE | I_BIT | F_BIT)
    LDR     SP, =usrStack
    MSR     CPSR_c, #(SVC_MODE | I_BIT | F_BIT)
  .endif
  LDR     SP, =svcStack
  /*
   * Install exception handlers
   */
  .ifdef CONFIG_ARCH_EXT_SECURITY
    /*
     * The security extensions introduce a register to set the exception vector base address (VBAR).
     * On the TI OMAP 35xx, this eliminates a few branches through the NAND and the SRAM.
     */
    LDR     R2, =exceptionVectorBase
    MCR     P15, 0, R2, C12, C0, 0
  .else
    .ifdef CONFIG_SOC_TI_OMAP_35XX
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
    .endif
    /*
     * Batch install exception handler addresses.
     */
    .ifndef CONFIG_SOC_TI_OMAP_35XX
      /*
       * Set R2 to the first exception handler address to be written.
       * On the TI OMAP 35xx R2 is already correct.
       */
      LDR     R2, =exception_vector
      LDR     R2, [R2]
    .endif
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
  .endif
  /*
   * Branch to main.
   * This function must never return.
   */
  B       main
infiniteLoopAfterMain:
  B       =infiniteLoopAfterMain


/*
 * Exception vector; only used in case security extensions are implemented.
 */
.ifdef CONFIG_ARCH_EXT_SECURITY
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
.endif


.global getGuestContext
getGuestContext:
  LDR     R0, =guestContextSpace
  LDR     R0, [R0]
  MOV     PC, LR

/* address of guest contest in R0 */
.global registerGuestPointer
.func   registerGuestPointer
registerGuestPointer:
  PUSH    {R0, R1}
  LDR     R1, =guestContextSpace
  STR     R0, [R1]

  /* restore dirty registers */
  POP     {R0, R1}
  MOV     PC, LR
.endfunc

/* Loads guest mode into R0, loads addr into R1 */
.macro get_emulated_mode
  LDR     R0, =guestContextSpace
  LDR     R0, [R0]
  MOV     R1, R0
  ADD     R0, R0, #GC_CPSR_OFFS
  LDR     R0, [R0]
  ANDS    R0, R0, #0x1F
  ADDEQ   R1, R1, #GC_R13_OFFS
  CMP     R0, #0x10
  ADDEQ   R1, R1, #GC_R13_OFFS
  CMP     R0, #0x11
  ADDEQ   R1, R1, #GC_R13_FIQ_OFFS
  CMP     R0, #0x12
  ADDEQ   R1, R1, #GC_R13_IRQ_OFFS
  CMP     R0, #0x13
  ADDEQ   R1, R1, #GC_R13_SVC_OFFS
  CMP     R0, #0x17
  ADDEQ   R1, R1, #GC_R13_ABT_OFFS
  CMP     R0, #0x1B
  ADDEQ   R1, R1, #GC_R13_UND_OFFS
.endm

.macro save_r0_to_r14
  LDR     LR, =guestContextSpace
  LDR     LR, [LR]
  STMIA   LR, {R0-R7}
  POP     {LR}
  /* store SP & LR to the correct places.*/
  get_emulated_mode
  /* use system mode to extract guest stack pointer and link register */
  MRS     R2, CPSR
  CPS     SYS_MODE
  STMIA   R1, {R13, R14}
  /* switch back to previous mode */
  MSR     CPSR, R2
  /* store r8-r12, guest mode still in R0 */
  LDR     R1, =guestContextSpace
  LDR     R1, [R1]
  CMP     R0, #0x11
  ADDNE   R1, R1, #GC_R8_OFFS
  ADDEQ   R1, R1, #GC_R8_FIQ_OFFS
  STMIA   R1, {R8-R12}
.endm

.macro save_pc
  /* store guest PC */
  MOV     R0, LR
  SUB     R0, R0, #4
  LDR     R1, =guestContextSpace
  LDR     R1, [R1]
  ADD     R1, R1, #GC_R15_OFFS
  STR     R0, [R1]
.endm

.macro save_pc_abort
  /* store guest PC */
  MOV     R0, LR
  SUB     R0, R0, #8
  LDR     R1, =guestContextSpace
  LDR     R1, [R1]
  ADD     R1, R1, #GC_R15_OFFS
  STR     R0, [R1]
.endm

.macro save_cc_flags
  /* saving condition flags, but no other (AIF) bits */
  LDR     R0, =guestContextSpace
  LDR     R0, [R0]
  ADD     R0, R0, #GC_CPSR_OFFS
  LDR     R1, [R0]
  AND     R1, #0x0FFFFFFF
  MRS     R2, SPSR
  AND     R2, #0xF0000000
  ORR     R1, R1, R2
  STR     R1, [R0]
.endm


.macro restore_r0_to_r12
  /* general purpose registers common to all modes, using LR as scratch */
  LDR     R8, =guestContextSpace
  LDR     R8, [R8]
  LDMIA   R8, {R0-R7}
  /* now either R8-12_FIQ or R8_12_common */
  ADD     LR, R8, #GC_CPSR_OFFS
  LDR     LR, [LR]
  AND     LR, LR, #0x1F
  CMP     LR, #0x11
  ADDNE   R8, R8, #GC_R8_OFFS
  ADDEQ   R8, R8, #GC_R8_FIQ_OFFS
  LDMIA   R8, {R8-R12}
  /* now r0-r7, r8-r12 have been restored */
.endm


/* Uses R0,R1,R2 as scratch registers */
.macro restore_r13_r14
  /* Use guest CPSR to work out which mode we are meant to be emulating */
  get_emulated_mode
  /* switch to system mode to restore SP & LR */
  MRS     R2, CPSR
  CPS     SYS_MODE
  LDR     SP, [R1]
  LDR     LR, [R1, #4]
  /* switch back to previous mode */
  MSR     CPSR, R2
.endm


/* Restores the cpsr to USR mode (& cc flags) then restore pc */
.macro restore_cpsr_pc_usr_mode_irq
  /* fixup spsr first */
  LDR     LR, =guestContextSpace
  LDR     LR, [LR]
  ADD     LR, LR, #GC_CPSR_OFFS
  LDR     LR, [LR]
  /* Preserve condition flags */
  AND     LR, LR, #0xf0000000
  /* set user mode, disable async abts and fiqs, but enable irqs */
.ifdef CONFIG_BLOCK_COPY_NO_IRQ
  ORR     LR, LR, #(USR_MODE | A_BIT | F_BIT | I_BIT)
.else
  ORR     LR, LR, #(USR_MODE | A_BIT | F_BIT)
.endif
  MSR     SPSR, LR
  /* get PC and save on stack */
  LDR     LR, =guestContextSpace
  LDR     LR, [LR]
  ADD     LR, LR, #GC_R15_OFFS
  LDR     LR, [LR]
  STM     SP, {LR}
  LDM     SP, {PC}^
.endm


/* Restores the cpsr to USR mode (& cc flags) then restore pc */
.macro restore_cpsr_pc_usr_mode
  /* fixup spsr first */
  PUSH    {R0}
  LDR     LR, =guestContextSpace
  LDR     LR, [LR]
  ADD     LR, LR, #GC_CPSR_OFFS
  /* Preserve condition flags */
  LDR     LR, [LR]
  AND     R0, LR, #0xf0000000
  /* Preserve exception flags */
  AND     LR, LR, #0x1C0
  ORR     R0, LR, R0
  /* set user mode */
  ORR     R0, R0, #(USR_MODE)
  MSR     SPSR, R0
  POP     {R0}
  /* get PC and save on stack */
  LDR     LR, =guestContextSpace
  LDR     LR, [LR]
  ADD     LR, LR, #GC_R15_OFFS
  LDR     LR, [LR]
  STM     SP, {LR} /* Store the PC on the stack for the final restore PC & CPSR instruction, no indexing */
  /* restore PC and load the SPSR into the CPSR */
  LDM     SP, {PC}^
.endm





.global svcHandler
svcHandler:
    /* We can NOT assume that the data abort is guest code */
    push   {LR}

    save_r0_to_r14 /* pops LR */
    save_pc
    save_cc_flags

    /* get SVC code into @parameter1 and call C function */
    LDR     R0, [LR, #-4]
    AND     R0, #0xFFFFFF
    BL      softwareInterrupt

    restore_r13_r14
    restore_r0_to_r12
    restore_cpsr_pc_usr_mode

.global dabtHandler
dabtHandler:
    /* We can NOT assume that the data abort is guest code */
    Push   {LR}
    /* Test SPSR -> are we from USR mode? */
    MRS    LR, SPSR
    ANDS   LR, LR, #0x0f
    BNE    dabtHandlerPriv

    /* We were in USR mode, we must have been running guest code */
    save_r0_to_r14
    /* Get the instr that aborted, after we fix up we probably want to re-try it */
    save_pc_abort
    save_cc_flags

    BL dataAbort

    /* We came from usr mode (emulation or not of guest state) lets restore it and try that faulting instr again*/
    restore_r13_r14
    restore_r0_to_r12
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
  ANDS   LR, LR, #0x0f
  BNE    undHandlerPriv /* Abort occured in Hypervisor (privileged) code */

  /* We were in USR mode, we must have been running guest code */
  save_r0_to_r14
  save_pc_abort
  save_cc_flags

  BL undefined

  /* We came from usr mode (emulation or not of guest state) lets restore it and resume */
  restore_r13_r14
  restore_r0_to_r12
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
  ANDS   LR, LR, #0x0f
  BNE    pabtHandlerPriv

  /* We were in USR mode, we must have been running guest code */
  save_r0_to_r14
  /* Get the instr that aborted, after we fix up we probably want to re-try it */
  save_pc_abort
  save_cc_flags

  BL prefetchAbort

  /* We came from usr mode (emulation or not of guest state) lets restore it and try that faulting instr again*/
  restore_r13_r14
  restore_r0_to_r12
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
  ANDS   LR, LR, #0x0f
  BNE    monHandlerPriv /* Call occured in Hypervisor (privileged) code */

  /* We were in USR mode, we must have been running guest code */
  save_r0_to_r14
  /* Get the instr that aborted, after we fix up we probably want to re-try it */
  save_pc_abort
  save_cc_flags

  BL monitorMode

  /* We came from usr mode (emulation or not of guest state) lets restore it and try that faulting instr again*/
  restore_r13_r14
  restore_r0_to_r12
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
  ANDS   LR, LR, #0x0f
  /* interrupt occured whilst running hypervisor */
  BNE    irqHandlerPriv

  /* We were in USR mode running guest code, need to save context */
  save_r0_to_r14
  /* save the PC of the guest, during which we got the interrupt */
  save_pc
  save_cc_flags
  BL irq
  restore_r13_r14
  restore_r0_to_r12
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

.ifndef CONFIG_SOC_TI_OMAP_35XX
exception_vector:
  .err @ Unknown target
.endif


.bss

/* pointer to current guest context structure lives here */
guestContextSpace:
  .space 4

.ifdef CONFIG_BLOCK_COPY
        
guestContextOther:
        .space 8

guestContextBlockCopyCache:
        .space 4
guestContextBlockCopyCacheLastUsedLine:
        .space 4
guestContextBlockCopyCacheEnd:
        .space 4
guestContextPCOfLastInstruction:
        .space 4
.endif

/* physical real mode stacks */
usrStack:
  .space 1024
svcStack:
  .space 1024
abtStack:
  .space 1024
undStack:
  .space 1024
irqStack:
  .space 1024
fiqStack:
  .space 1024

.section .rodata
