/* Standard definitions */

  .equ  A_BIT,          0x100     /* when set, async aborts are disabled */
  .equ  I_BIT,          0x80      /* when set, IRQ is disabled */
  .equ  F_BIT,          0x40      /* when set, FIQ is disabled */
  .equ  T_BIT,			0x20      /* when set, Thumb mode is enabled */

  .equ  USR_MODE,       0x10
  .equ  FIQ_MODE,       0x11
  .equ  IRQ_MODE,       0x12
  .equ  SVC_MODE,       0x13
  .equ  ABT_MODE,       0x17
  .equ  UND_MODE,       0x1B
  .equ  SYS_MODE,       0x1F

  .equ  GC_R0_OFFS,       0x0
  .equ  GC_R1_OFFS,       0x4
  .equ  GC_R2_OFFS,       0x8
  .equ  GC_R3_OFFS,       0xc
  .equ  GC_R4_OFFS,       0x10
  .equ  GC_R5_OFFS,       0x14
  .equ  GC_R6_OFFS,       0x18
  .equ  GC_R7_OFFS,       0x1c
  .equ  GC_R8_OFFS,       0x20
  .equ  GC_R9_OFFS,       0x24
  .equ  GC_R10_OFFS,      0x28
  .equ  GC_R11_OFFS,      0x2c
  .equ  GC_R12_OFFS,      0x30
  .equ  GC_R13_OFFS,      0x34
  .equ  GC_R14_OFFS,      0x38
  .equ  GC_R15_OFFS,      0x3c
  .equ  GC_CPSR_OFFS,     0x40
  .equ  GC_R8_FIQ_OFFS,   0x44
  .equ  GC_R9_FIQ_OFFS,   0x48
  .equ  GC_R10_FIQ_OFFS,  0x4c
  .equ  GC_R11_FIQ_OFFS,  0x50
  .equ  GC_R12_FIQ_OFFS,  0x54
  .equ  GC_R13_FIQ_OFFS,  0x58
  .equ  GC_R14_FIQ_OFFS,  0x5c
  .equ  GC_SPSR_FIQ_OFFS, 0x60
  .equ  GC_R13_SVC_OFFS,  0x64
  .equ  GC_R14_SVC_OFFS,  0x68
  .equ  GC_SPSR_SVC_OFFS, 0x6c
  .equ  GC_R13_ABT_OFFS,  0x70
  .equ  GC_R14_ABT_OFFS,  0x74
  .equ  GC_SPSR_ABT_OFFS, 0x78
  .equ  GC_R13_IRQ_OFFS,  0x7c
  .equ  GC_R14_IRQ_OFFS,  0x80
  .equ  GC_SPSR_IRQ_OFFS, 0x84
  .equ  GC_R13_UND_OFFS,  0x88
  .equ  GC_R14_UND_OFFS,  0x8c
  .equ  GC_SPSR_UND_OFFS, 0x90


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


.global startupHypervisor
.func   startupHypervisor
startupHypervisor:

/* Initialize stacks for all modes */
  /* set IRQ stack */
  MSR     CPSR_c,#(IRQ_MODE | I_BIT | F_BIT)
  LDR     sp,=irqStack

  /* set FIQ stack */
  MSR     CPSR_c,#(FIQ_MODE | I_BIT | F_BIT)
  LDR     sp,=fiqStack

  /* set SVC stack */
  MSR     CPSR_c,#(SVC_MODE | I_BIT | F_BIT)
  LDR     sp,=serviceStack

  /* set ABT stack */
  MSR     CPSR_c,#(ABT_MODE | I_BIT | F_BIT)
  LDR     sp,=abortStack

  /* set UND stack */
  MSR     CPSR_c,#(UND_MODE | I_BIT | F_BIT)
  LDR     sp,=undefinedStack

  /* set user/system stack */
  MSR     CPSR_c,#(SYS_MODE | I_BIT | F_BIT)
  /* since we will start the guest thinking its in SVC mode, choose this stack*/
  LDR     sp,=userStack

  /* switch back to svc mode */
  MSR     CPSR_c,#(SVC_MODE | I_BIT | F_BIT)

  /* save to-be-dirty registers */
  PUSH    {R1, R2, R3, R4, lr}

  /* register allocated guest context */
  .ifdef CONFIG_CPU_HAS_ARM_SEC_EXT
    /* Set VBAR to exceptionVectorBase. On the OMAP35xx, this eliminates a few branches through the NAND and the SRAM */
    LDR     r4, =exceptionVectorBase
    MCR     p15, 0, r4, c12, c0, 0
  .else
    /* Fix RAM exception vectors on TI OMAP 35xx */
    .ifdef CONFIG_CPU_TI_OMAP_35XX
      LDR     R4, [pc, #0x20]
      LDR     R3, [pc, #0x20]
      STR     R3, [R4], #4
      STR     R3, [R4], #4
      STR     R3, [R4], #4
      STR     R3, [R4], #4
      STR     R3, [R4], #4
      STR     R3, [R4], #4
      STR     R3, [R4], #4
      ADD     pc, pc, #0x4
      /* next 2 lines are data */
      .word 0x4020FFC8
      LDR     pc, [pc, #0x14]
    .endif

    /* Batch installation of interupt handlers */
    /* This section of code is order dependant */
    .ifndef CONFIG_CPU_TI_OMAP_35XX
      /* On the TI OMAP 35xx R4 is already correct */
      LDR     R4,=exception_vector @First interupt handler address
      LDR     R4, [R4]
    .endif
    LDR     R3,=undHandler
    STR     R3, [R4], #4       @auto increment to the next handler address
    LDR     R3,=swiHandler
    STR     R3, [R4], #4       @install swi handler
    LDR     R3,=pabthandler
    STR     R3, [R4], #4
    LDR     R3,=dabtHandler
    STR     R3, [R4], #4
    LDR     R3,=monHandler
    STR     R3, [R4], #4
    LDR     R3,=irqHandler
    STR     R3, [R4], #4
    LDR     R3,=fiqHandler
    STR     R3, [R4], #4
  .endif

  /* restore dirty registers */
  POP     {R1, R2, R3, R4, lr}
  MOV     PC, LR
.endfunc

/* Loads guest mode into R0, loads addr into R1 */
.macro get_emulated_mode
  LDR     R0, =guestContextSpace
  LDR     R0, [R0]
  MOV     R1, R0
  ADD     R0, R0, #GC_CPSR_OFFS
  LDR     R0, [R0]
  AND     R0, R0, #0x1F
  CMP	  R0, #0x1F
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
  PUSH	  {R3}
  LDR	  R3, =guestContextSpace
  LDR	  R3, [R3]
  ADD	  R3, R3, #GC_CPSR_OFFS
  LDR     R3, [R3]
  AND     R3, R3, #0x20
  CMP	  R3, #0x20	
  SUBNE   R0, R0, #4 @ARM
  SUBEQ	  R0, R0, #2 @Thumb
  POP	  {R3}
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
  /* set user mode, disable async abts and fiqs, but enable irqs and check for Thumb Bit */
  PUSH    {R3}
  LDR	  R3, =guestContextSpace
  LDR     R3, [R3]
  ADD     R3, R3, #GC_CPSR_OFFS
  LDR     R3, [R3]
  AND     R3, R3, #0x20
  CMP	  R3, #0x20
  ORREQ   LR, LR, #(USR_MODE | A_BIT | F_BIT | T_BIT)
  ORRNE   LR, LR, #(USR_MODE | A_BIT | F_BIT)
  MSR     SPSR, LR
  POP     {R3}
  /* get PC and save on stack */
  LDR     LR, =guestContextSpace
  LDR     LR, [LR]
  ADD     LR, LR, #GC_R15_OFFS
  LDR     LR, [LR]
  PUSH	  {R3}
  LDR	  R3, =guestContextSpace
  LDR	  R3, [R3]
  ADD	  R3, R3, #GC_CPSR_OFFS
  LDR	  R3, [R3]
  AND	  R3, R3, #0x20
  CMP	  R3, #0x20
  SUBEQ   LR, LR, #2
  POP	  {R3}
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
  /* Preserve exception flags & Thumb state*/
  AND     LR, LR, #0x1E0
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

.ifdef CONFIG_CPU_HAS_ARM_SEC_EXT
  .align 5
exceptionVectorBase:
  MOV     pc, #0x00014000
  B       undHandler
  B       swiHandler
  B       pabthandler
  B       dabtHandler
  B       monHandler
  B       irqHandler
  B       fiqHandler
.endif

.global swiHandler
swiHandler:
    push   {LR}
 
 	save_r0_to_r14 /* pops LR */
	save_pc
	save_cc_flags

    /* get SVC code into @parameter1 and call C function */
    LDR		R0, =guestContextSpace
	LDR		R0, [R0]
	ADD		R0, R0, #GC_CPSR_OFFS
	LDR		R0, [R0]
	AND		R1, R0, #0x20 @Check thumb bit
	CMP		R1, #0x20
	LDRNE   R0, [LR, #-4] @Thumb bit = 0
    ANDNE   R0, #0xFFFFFF
	LDREQB	R0, [LR, #-2]
	ANDEQ	R0, #0x00FF
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
    AND    LR, LR, #0x0f
	CMP	   LR, #0x0f
    BEQ    dabtHandlerPriv

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
exception_vector:
        .ifdef TARGET_BEAGLE
          .word 0x4020FFE4
        .else
          .ifdef TARGET_TEGRA250
            .word 0x6000F200
          .else
            .err @Unknown target
          .endif
        .endif

.bss
/* pointer to current guest context structure lives here */
guestContextSpace:
        .space 4

/* physical real mode stacks */
userStack:
        .space 1024
serviceStack:
        .space 1024
abortStack:
        .space 1024
undefinedStack:
        .space 1024
irqStack:
        .space 1024
fiqStack:
        .space 1024

.section .rodata
