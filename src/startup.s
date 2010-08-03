/* Standard definitions */

	.equ	I_BIT,          0x80      /* when set, IRQ is disabled */
	.equ	F_BIT,          0x40      /* when set, FIQ is disabled */

	.equ	USR_MODE,       0x10
	.equ	FIQ_MODE,       0x11
	.equ	IRQ_MODE,       0x12
	.equ	SVC_MODE,       0x13
	.equ	ABT_MODE,       0x17
	.equ	UND_MODE,       0x1B
	.equ	SYS_MODE,       0x1F

	.align	5
.global startup_hypervisor
.func   startup_hypervisor
startup_hypervisor:

_start:

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
	LDR     sp,=guestStackSVC

  /* switch back to svc mode */
	MSR     CPSR_c,#(SVC_MODE | I_BIT | F_BIT)

  /* save to-be-dirty registers */
  PUSH    {R1, R2, R3, R4, lr}

  /* register allocated guest context */
  LDR     R0,=guestContextSpace
  BL      registerGuestContext

  /* Batch installation of interupt handlers */
  /* This section of code is order dependant */
  LDR     R4,=undefined_addr @First interupt handler address
  LDR     R4, [R4]
  LDR     R3,=undefined_handler
  STR     R3, [R4], #4       @auto increment to the next handler address
  LDR     R3,=swi_handler
  STR     R3, [R4], #4       @install swi handler
  LDR     R3,=prefetch_abort_handler
  STR     R3, [R4], #4
  LDR     R3,=data_abort_handler
  STR     R3, [R4], #4
  LDR     R3,=monitor_mode_handler
  STR     R3, [R4], #4
  LDR     R3,=irq_handler
  STR     R3, [R4], #4
  LDR     R3,=fiq_handler
  STR     R3, [R4], #4

  /* write guest stack pointers for now */
  LDR     R0, =guestContextR13_USR
  LDR     R1, =guestStackUser
  STR     R1, [R0]
  LDR     R0, =guestContextR13_FIQ
  LDR     R1, =guestStackFIQ
  STR     R1, [R0]
  LDR     R0, =guestContextR13_SVC
  LDR     R1, =guestStackSVC
  STR     R1, [R0]
  LDR     R0, =guestContextR13_ABT
  LDR     R1, =guestStackABT
  STR     R1, [R0]
  LDR     R0, =guestContextR13_IRQ
  LDR     R1, =guestStackIRQ
  STR     R1, [R0]
  LDR     R0, =guestContextR13_UND
  LDR     R1, =guestStackUND
  STR     R1, [R0]

  /* restore dirty registers */
  POP     {R1, R2, R3, R4, lr}
  MOV     PC, LR
.endfunc

      /* Alex - Macroed Interupt Service Handler code for reuse between handlers*/

        /* Loads guest mode into R0, loads addr into R1 */
        .macro get_emulated_mode
        /* WARNING: changing registers will break dependant code */
        LDR     R0, =guestContextCPSR
        LDR     R0, [R0]
        ANDS    R0, R0, #0x1F
        LDREQ   R1, =guestContextR13_USR /* system mode - same register set as usr */
        CMP     R0, #0x10
        LDREQ   R1, =guestContextR13_USR
        CMP     R0, #0x11
        LDREQ   R1, =guestContextR13_FIQ
        CMP     R0, #0x12
        LDREQ   R1, =guestContextR13_IRQ
        CMP     R0, #0x13
        LDREQ   R1, =guestContextR13_SVC
        CMP     R0, #0x17
        LDREQ   R1, =guestContextR13_ABT
        CMP     R0, #0x1B
        LDREQ   R1, =guestContextR13_UND
        .endm

        .macro save_r0_to_r14
        LDR     lr,=guestContextSpace
        STMIA   lr, {R0-R7}
        POP     {lr}

        /* store SP & LR to the correct places.*/
        get_emulated_mode @ uses R0 as scratch, puts guestContext addr into R1
        /* use system mode to extract guest stack pointer and link register */
        MRS     R2, CPSR /* Store current mode so we can switch back to it */
        MSR     CPSR_c,#(SYS_MODE | I_BIT | F_BIT)
        STMIA   R1, {R13, R14}
        /* switch back to previous mode */
        MSR     CPSR_c, R2

        /* store r8-r12, guest mode still in R0 */
        /* evaluate guest mode - was guest in FIQ? */
        CMP     R0, #0x11 /* 10001b - FIQ mode bits */
        LDRNE   R1, =guestContextR8
        LDREQ   R1, =guestContextR8_FIQ /* FIQ has its own R8-R12 registers */
        STMIA   R1, {R8-R12}
        .endm

        .macro save_pc
        /* store guest PC */
        MOV     R0, LR
        SUB     R0, R0, #4
        LDR     R1, =guestContextR15
        STR     R0, [R1]
        .endm

        .macro save_pc_abort
        /* store guest PC */
        MOV     R0, LR
        SUB     R0, R0, #8 /* Get offending instruction */
        LDR     R1, =guestContextR15
        STR     R0, [R1]
        .endm

        .macro save_cc_flags
        LDR     R0, =guestContextCPSR
        LDR     R1, [R0]
        AND     R1, #0x0FFFFFFF
        MRS     R2, SPSR
        AND     R2, #0xF0000000
        ORR     R1, R1, R2
        STR     R1, [R0]
        .endm

        .macro restore_r0_to_r12
        /* general purpose registers common to all modes, using LR as scratch */
        LDR     LR,=guestContextR0
        LDMIA   LR, {R0-R7}
        /* now either R8-12_FIQ or R8_12_common */
        LDR     LR, =guestContextCPSR
        LDR     LR, [LR]
        AND     LR, LR, #0x1F
        CMP     LR, #0x11 /* 10001b - FIQ mode bits */
        LDRNE   LR, =guestContextR8 /* Not FIQ, restore normal R8-R12 */
        LDREQ   LR, =guestContextR8_FIQ /* FIQ has its own R8-R12 registers, restore from them instead */
        LDMIA   LR, {R8-R12}
        .endm

        /* Uses R0,R1,R2 as scratch registers */
        .macro restore_r13_r14
        /* Use guest CPSR to work out which mode we are meant to be emulating */
        get_emulated_mode
        /* switch to system mode to restore SP & LR */
        MRS     R2, CPSR /* backup up current mode */
        MSR     CPSR_c,#(SYS_MODE | I_BIT | F_BIT)
        /* restore SP & LR from guest Context */
        LDR     SP, [R1]
        LDR     LR, [R1, #4]
        /* switch back to previous mode */
        MSR     CPSR_c, R2
        .endm

        /* WARNING: only restores a subset of things in the CPSR, will need expanding in future to support THUMB, SIMD etc */
        /* Restores the cpsr to USR mode (& cc flags) then restore pc */
        .macro restore_cpsr_pc_usr_mode
        /* get PC and save on stack */
        LDR     LR, =guestContextR15
        LDR     LR, [LR]
        STM     SP, {LR} /* Store the PC on the stack for the final restore PC & CPSR instruction, no indexing */
        /* Get the CPSR cc flags from guestContext, and put them in the SPSR ready for the restore */
        LDR     LR,=guestContextCPSR
        LDR     LR, [LR]
        AND     LR, #0xF0000000
        ORR     LR, LR, #(USR_MODE | I_BIT | F_BIT)
        MSR     SPSR, LR
        LDM     SP, {PC}^ /* restore PC and load the SPSR into the CPSR */
        .endm
        

.global swi_handler
swi_handler:
  /* Not in macro -> other ISRs do other checks before saving r0-r14 */
  PUSH    {LR}

  /* pops LR */
  save_r0_to_r14 
  save_pc
  /* we need the condition flags that were in the guest saved in ARB! */
  save_cc_flags

  /* get SVC code into @parameter1 and call C function */
  LDR     R0, [LR, #-4]
  AND     R0, #0xFFFFFF
  BL      do_software_interrupt

  /* handled this SWI. lets restore user state! */
  
  /* uses R0,R1,R2 as scratch keep above "restore_r0_to_r12" */
  restore_r13_r14 
  restore_r0_to_r12
  restore_cpsr_pc_usr_mode

.global data_abort_handler
data_abort_handler:
  /* We can NOT assume that the data abort is guest code */
  /* Assuming Data Abort SP is valid at our own peril!*/

  push   {LR}
  /* If we aborted in FIQ then we can switch mode to get r8-12 later */

  @Test SPSR -> are we from USR mode?
  MRS    LR, SPSR
  ANDS   LR, LR, #0x0f
  BNE    data_abort_handler_privileged_mode /* Abort occured in Hypervisor (privileged) code */

  /* We were in USR mode, we must have been running guest code */
  save_r0_to_r14 @pops LR
  /* Get the instr that aborted, after we fix up we probably want to re-try it */
  save_pc_abort
  save_cc_flags
  
  BL do_data_abort

  /* We came from usr mode (emulation or not of guest state) lets restore it and try that faulting instr again*/
  restore_r13_r14
  restore_r0_to_r12
  restore_cpsr_pc_usr_mode
  /* End restore code */

.global data_abort_handler_privileged_mode
data_abort_handler_privileged_mode:
  /* Not from USR mode -> Our Hypervisor has caused this as we were running in a priviledged mode */
  /* -> grab state, and abort nicely */
  /* Save r0-r7 & r8-r12 */
  pop     {LR} /* we backed up the LR to hold the SPSR */
  push    {r0-r7}
  CMP     LR, #0x11
  STMNEIA SP!, {r8-r12} /* Not FIQ save r8-12 */
  /* Switch to FIQ_MODE if eq */
  MOVEQ   R0, SP
  MSREQ   cpsr_c,#(FIQ_MODE | I_BIT | F_BIT)
  STMEQIA R0!, {r8-r12}
  /* and switch back to abort mode */
  MSREQ   cpsr_c,#(ABT_MODE | I_BIT | F_BIT)
  MOVEQ   SP, R0

  /* Save SP & LR */
  CMP     LR, #0x1 @FIQ
  MSREQ   cpsr_c, #(FIQ_MODE | I_BIT | F_BIT)
  CMP     LR, #0x2 @IRQ
  MSREQ   cpsr_c, #(IRQ_MODE | I_BIT | F_BIT)
  CMP     LR, #0x3 @SVC
  MSREQ   cpsr_c, #(SVC_MODE | I_BIT | F_BIT)
  CMP     LR, #0xB @UND
  MSREQ   cpsr_c, #(UND_MODE | I_BIT | F_BIT)

  MOV     R1, SP
  MOV     R2, LR
  MSR     cpsr_c, #(ABT_MODE | I_BIT | F_BIT)
  STMIA   SP!, {R1,R2} /* Store SP & LR to data abort stack */

  /* Save PC  of instr that aborted*/
  SUB     R0, LR, #8
  STMIA   SP!, {r0}
  MRS     R0, SPSR
  PUSH    {r0}

  BL do_data_abort_hypervisor

  /* Hypervisor should have fixed things up and be ready to continue */
  /* Unwind the abort*/

  /* Pop */
  POP     {R0} @ pop the SPSR
  POP     {LR} /* Pop aborted PC onto LR */
  POP     {R1,R2} /* Pop SP & LR of previous mode */

  CMP     R0, #0x1 @FIQ
  MSREQ   cpsr_c, #(FIQ_MODE | I_BIT | F_BIT)
  CMP     R0, #0x2 @IRQ
  MSREQ   cpsr_c, #(IRQ_MODE | I_BIT | F_BIT)
  CMP     R0, #0x3 @SVC
  MSREQ   cpsr_c, #(SVC_MODE | I_BIT | F_BIT)
  CMP     R0, #0xB @UND
  MSREQ   cpsr_c, #(UND_MODE | I_BIT | F_BIT)

  MOV     SP, R1
  MOV     LR, R2
  MSR     cpsr_c, #(ABT_MODE | I_BIT | F_BIT)
  /* Restored SP & LR of previous mode*/

  /* Restore r8-r12 to correct mode */
  CMP     R0, #0x11
  LDMNEIA SP!, {r8-r12} /* Not FIQ store r8-12 */
  /* Switch to FIQ_MODE if eq */
  MOVEQ   R1, SP
  MSREQ   cpsr_c,#(FIQ_MODE | I_BIT | F_BIT)
  LDMEQIA R1!, {r8-r12}
  /* and switch back to abort mode */
  MSREQ   cpsr_c,#(ABT_MODE | I_BIT | F_BIT)
  MOVEQ   SP, R1

  /* SPSR is held in R0, save to the current SPSR */
  MSR     SPSR, R0

  POP    {r0-r7} /* Restore R0-R7 */

  PUSH    {LR} /* Holding the aborted PC in the LR */
  LDM     SP, {PC}^ /* restore PC and load the SPSR into the CPSR */

.global undefined_handler
undefined_handler:
  STMFD  SP!, {LR}
  /* If we aborted in FIQ then we can switch mode to get r8-12 later */

  @Test SPSR -> are we from USR mode?
  MRS    LR, SPSR
  ANDS   LR, LR, #0x0f
  BNE    undefined_handler_privileged_mode /* Abort occured in Hypervisor (privileged) code */

  /* We were in USR mode, we must have been running guest code */
  save_r0_to_r14 @pops LR
  save_pc_abort

  BL do_undefined

  /* We came from usr mode (emulation or not of guest state) lets restore it and resume */
  restore_r13_r14
  restore_r0_to_r12
  restore_cpsr_pc_usr_mode
  /* End restore code */

.global undefined_handler_privileged_mode
undefined_handler_privileged_mode:

  @LR already saved
  STMFD SP!, {R0-R12}

  BL do_undefined_hypervisor

  LDMFD SP!, {R0-R12}
  LDMFD SP!, {PC}^

.global prefetch_abort_handler
prefetch_abort_handler:

  STMFD  SP!, {LR}
  /* If we aborted in FIQ then we can switch mode to get r8-12 later */

  @Test SPSR -> are we from USR mode?
  MRS    LR, SPSR
  ANDS   LR, LR, #0x0f
  BNE    prefetch_abort_handler_privileged_mode /* Abort occured in Hypervisor (privileged) code */

  /* We were in USR mode, we must have been running guest code */
  save_r0_to_r14 @pops LR
  /* Get the instr that aborted, after we fix up we probably want to re-try it */
  save_pc_abort

  BL do_prefetch_abort

  /* We came from usr mode (emulation or not of guest state) lets restore it and try that faulting instr again*/
  restore_r13_r14
  restore_r0_to_r12
  restore_cpsr_pc_usr_mode
  /* End restore code */

.global prefetch_abort_handler_privileged_mode
prefetch_abort_handler_privileged_mode:

  @LR already saved
  STMFD SP!, {R0-R12}

  BL do_prefetch_abort_hypervisor

  LDMFD SP!, {R0-R12}
  LDMFD SP!, {PC}^

.global monitor_mode_handler
monitor_mode_handler:
    /* This may never be used, but if it occurs we want the same level of information as the other handlers*/
  STMFD  SP!, {LR}
  /* If we aborted in FIQ then we can switch mode to get r8-12 later */

  @Test SPSR -> are we from USR mode?
  MRS    LR, SPSR
  ANDS   LR, LR, #0x0f
  BNE    monitor_mode_handler_privileged_mode /* Call occured in Hypervisor (privileged) code */

  /* We were in USR mode, we must have been running guest code */
  save_r0_to_r14 @pops LR
  /* Get the instr that aborted, after we fix up we probably want to re-try it */
  save_pc_abort

  BL do_monitor_mode

  /* We came from usr mode (emulation or not of guest state) lets restore it and try that faulting instr again*/
  restore_r13_r14
  restore_r0_to_r12
  restore_cpsr_pc_usr_mode
  /* End restore code */

.global monitor_mode_handler_privileged_mode
monitor_mode_handler_privileged_mode:

  @LR already saved
  STMFD SP!, {R0-R12}

  BL do_monitor_mode_hypervisor

  LDMFD SP!, {R0-R12}
  LDMFD SP!, {PC}^

.global irq_handler
irq_handler:
  STMFD SP!, {LR}
  STMFD SP!, {R0-R12}

  BL do_irq

  LDMFD SP!, {R0-R12}
  LDMFD SP!, {PC}^

.global fiq_handler
fiq_handler:
  STMFD SP!, {LR}
  STMFD SP!, {R0-R12}

  BL do_fiq

  LDMFD SP!, {R0-R12}
  LDMFD SP!, {PC}^

        .data
var_data:
        .word   0x12345678
/* WARNING: Changes to any of these interupt handler addresses may require the installation code to be modified */
undefined_addr: @only this addr is used in code, others are here for reference
        .word   0x4020FFE4
@ swi_addr:
@         .word   0x4020FFE8
@ prefetch_abort_addr:
@         .word   0x4020FFEC
@ data_abort_addr:
@         .word   0x4020FFF0
@ monitor_mode_addr:
@         .word   0x4020FFF4
@ irq_addr:
@         .word   0x4020FFF8
@ fiq_addr:
@         .word   0x4020FFFC
/* END WARNING */

        .bss

/* ARB STRUCT */
.global guestContextSpace
guestContextSpace:
guestContextR0:
        .space 4
guestContextR1:
        .space 4
guestContextR2:
        .space 4
guestContextR3:
        .space 4
guestContextR4:
        .space 4
guestContextR5:
        .space 4
guestContextR6:
        .space 4
guestContextR7:
        .space 4
guestContextR8:
        .space 4
guestContextR9:
        .space 4
guestContextR10:
        .space 4
guestContextR11:
        .space 4
guestContextR12:
        .space 4
guestContextR13_USR:
        .space 4
guestContextR14_USR:
        .space 4
guestContextR15:
        .space 4
guestContextCPSR:
        .space 4
guestContextR8_FIQ:
        .space 4
guestContextR9_FIQ:
        .space 4
guestContextR10_FIQ:
        .space 4
guestContextR11_FIQ:
        .space 4
guestContextR12_FIQ:
        .space 4
guestContextR13_FIQ:
        .space 4
guestContextR14_FIQ:
        .space 4
guestContextSPSR_FIQ:
        .space 4
guestContextR13_SVC:
        .space 4
guestContextR14_SVC:
        .space 4
guestContextSPSR_SVC:
        .space 4
guestContextR13_ABT:
        .space 4
guestContextR14_ABT:
        .space 4
guestContextSPSR_ABT:
        .space 4
guestContextR13_IRQ:
        .space 4
guestContextR14_IRQ:
        .space 4
guestContextSPSR_IRQ:
        .space 4
guestContextR13_UND:
        .space 4
guestContextR14_UND:
        .space 4
guestContextSPSR_UND:
        .space 4
guestContextEOB_INSTR:
        .space 4
guestContextHDL_FUNCT:
        .space 4
guestContextCRB_ADDRESS:
        .space 4
guestContextBLOCK_CACHE_ADDRESS:
        .space 4
PT_physical:
        .space 4
PT_os:
        .space 4
PT_os_real:
        .space 4
PT_shadow:
        .space 4
virtAddrEnabled:
        .space 4
memProt:
        .space 4
guestUndefinedHandler:
        .space 4
guestSwiHandler:
        .space 4
guestPrefAbortHandler:
        .space 4
guestDataAbortHandler:
        .space 4
guestUnusedHandler:
        .space 4
guestIrqHandler:
        .space 4
guestFiqHandler:
        .space 4
hardwareLibrary:
        .space 4
        
guestContextOther:
        .space 8

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
systemStack:
        .space 1024

/* guest OS stacks */
guestStackUser:
        .space 1024
guestStackFIQ:
        .space 1024
guestStackSVC:
        .space 1024
guestStackABT:
        .space 1024
guestStackIRQ:
        .space 1024
guestStackUND:
        .space 1024
var_zero:
        .space  4
        .section .rodata
