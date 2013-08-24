/***************************************************************
 * guest in abort mode - exception vector
 ***************************************************************/
.text

.global abortReset
abortReset:
  B       abortReset


/***************************************************/
/* undefined errorhandler, guest was in abort mode */
/***************************************************/
.global abortUndefined
abortUndefined:
  B      abortUndefined


/***************************************************/
/* serice call handler, guest was in abort mode    */
/***************************************************/
.global abortSvc
abortSvc:
#ifdef CONFIG_STATS
  recordStartSvc
#endif
  PUSH   {LR}

  /* Load guest context into LR and store R0--R12 into guest context. */
  LDR     LR, =guestContextSpace
  LDR     LR, [LR]
  STMIA   LR, {R0-R12}

  /* R0--R12 are now usable, move guest context in there. */
  MOV     R0, LR

  POP     {LR}

  /* load guest CPSR into R1 to save everything but AIF flags */
  LDR     R1, [R0, #GC_CPSR_OFFS]
  MOVW    R3, #0x1DF
  AND     R1, R3
  MRS     R2, SPSR
  BIC     R2, R3
  ORR     R1, R1, R2
  STR     R1, [R0, #GC_CPSR_OFFS]

  /* guest context abort mode R13/R14 register pointer in R1, save state  */
  ADD     R1, R0, #GC_R13_ABT_OFFS
  CPS     PSR_SYS_MODE
  STMIA   R1, {R13, R14}
  CPS     PSR_SVC_MODE

  /* save guest PC in guest context */
  MOV     R2, LR
#ifdef CONFIG_THUMB2
  /* Load CPSR value from guest context */
  LDR     R3, [R0, #GC_CPSR_OFFS]
  TST     R3, #PSR_T_BIT
  SUBEQ   R2, R2, #4 @ARM
  SUBNE   R2, R2, #2 @Thumb
#else
  SUB     R2, R2, #4
#endif
  STR     R2, [R0, #GC_R15_OFFS]

  /* GCONTXT *softwareInterrupt(GCONTXT *gContext, u32int code);
   * Guest context pointer is already in R0! Store SVC code in R1.*/
#ifdef CONFIG_THUMB2
  LDR     R1, [R0, #GC_CPSR_OFFS]
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

  /* switch to system mode to restore R13 R14 reg: problem is, the guest
   * could have changed modes in the SVC handler. must check which to restore
   * guest came in SVC mode. but after handling svc it could no longer be in SVC */
  LDR     R1, [R0, #GC_CPSR_OFFS]
  AND     R1, R1, #PSR_MODE
  CMP     R1, #PSR_SVC_MODE
  ADDEQ   R2, R0, #GC_R13_SVC_OFFS
  BEQ     contAbortSvc
  CMP     R1, #PSR_ABT_MODE
  ADDEQ   R2, R0, #GC_R13_ABT_OFFS
  BEQ     contAbortSvc
  CMP     R1, #PSR_IRQ_MODE
  ADDEQ   R2, R0, #GC_R13_IRQ_OFFS
  BEQ     contAbortSvc
  CMP     R1, #PSR_FIQ_MODE
  ADDEQ   R2, R0, #GC_R13_FIQ_OFFS
  BEQ     contAbortSvc
  CMP     R1, #PSR_UND_MODE
  ADDEQ   R2, R0, #GC_R13_UND_OFFS
  BEQ     contAbortSvc
  ADD     R2, R0, #GC_R13_OFFS
contAbortSvc:
  CPS     PSR_SYS_MODE
  LDR     SP, [R2]
  LDR     LR, [R2, #4]
  CPS     PSR_SVC_MODE

  /* Lets not loose the guest context pointer.. SP/LR are already free to use */
  MOV     LR, R0
  /* Restore general purpose registers R0--R12 */
  LDMIA   LR, {R0-R12}

  PUSH    {R0}
  /* Get guest CPSR into R0 */
  LDR     R0, [LR, #GC_CPSR_OFFS]
  
  /* get next guest pc in lr */
  LDR     LR, [LR, #GC_R15_OFFS]
  
  /* Construct SPSR in R0: set user mode, disable asynchronous aborts and FIQs, but enable IRQs, and
    restore all other fields from the CPSR*/
#ifdef CONFIG_PROFILER
  BIC     R0, R0, #(PSR_I_BIT | PSR_MODE | PSR_F_BIT)
  ORR     R0, R0, #(PSR_A_BIT | PSR_USR_MODE)
#else
#ifdef CONFIG_HW_PASSTHROUGH
  /* exception bits set from guset context in passthrough */
  BIC     R0, R0, #(PSR_MODE)
  ORR     R0, R0, #(PSR_USR_MODE)
#else
  BIC     R0, R0, #(PSR_I_BIT | PSR_MODE)
  ORR     R0, R0, #(PSR_A_BIT | PSR_F_BIT | PSR_USR_MODE)
#endif
#endif
  MSR     SPSR, R0
  POP     {R0}
#ifdef CONFIG_THUMB2
  STMDB   SP, {LR}
  LDMDB   SP, {PC}^
#else
#ifdef CONFIG_STATS
  recordEndSvc
#endif
  MOVS    PC, LR
#endif


/***************************************************/
/* prefetch abort handler, guest was in abort mode */
/***************************************************/
.global abortPrefetch
abortPrefetch:
  B   abortPrefetch


/***************************************************/
/* data abort handler, guest was in abort mode     */
/***************************************************/
.global abortDataAbort
abortDataAbort:
  /* The state of monitors is architecturally undefined after a Data Abort exception (ARM DHT 0008A). */
  CLREX
#ifdef CONFIG_STATS
  recordStartDabt
#endif
  PUSH   {LR}
  /* Test SPSR -> are we from USR mode? */
  MRS    LR, SPSR
  AND    LR, LR, #PSR_MODE
  CMP    LR, #PSR_USR_MODE

  BNE    dabtHandlerPriv

  /* Load guest context into LR and store R0--R12 into guest context. */
  LDR     LR, =guestContextSpace
  LDR     LR, [LR]
  STMIA   LR, {R0-R12}

  /* R0--R12 are now usable, move guest context in there. */
  MOV     R0, LR

  POP     {LR}

/* load guest CPSR into R1 to save everything but AIF flags */
  LDR     R1, [R0, #GC_CPSR_OFFS]
  MOVW    R3, #0x1DF
  AND     R1, R3
  MRS     R2, SPSR
  BIC     R2, R3
  ORR     R1, R1, R2
  STR     R1, [R0, #GC_CPSR_OFFS]
  
  /* lets save the host abort mode LR, as we are not coming back to ABT mode! */
  MOV     R2, LR

  /* guest context abort mode R13/R14 register pointer in R1, save state  */
  ADD     R1, R0, #GC_R13_ABT_OFFS
  CPS     PSR_SYS_MODE
  STMIA   R1, {R13, R14}
  CPS     PSR_SVC_MODE

  /* save guest PC in guest context */
#ifdef CONFIG_THUMB2
  /* Load CPSR value from guest context */
  /* previously there was no separate implementation of this for THUMB. */
  /* is that correct? die, and checkit out if we hit it. */
  .global abtThumbModeAbort
abtThumbModeAbort:
  B       abtThumbModeAbort
#else
  SUB     R2, R2, #8
#endif
  STR     R2, [R0, #GC_R15_OFFS]

  /* Pointer to guest context is already in R0! */
  BL     dataAbort
  
  /* switch to system mode to restore R13 R14 reg: problem is, the guest
   * could have changed modes in the abort handler. must check which to restore
   * guest came in SVC mode. but after handling abort it could change to ABT mode */
  LDR     R1, [R0, #GC_CPSR_OFFS]
  AND     R1, R1, #PSR_MODE

  CMP     R1, #PSR_ABT_MODE
  ADDEQ   R2, R0, #GC_R13_ABT_OFFS
  BEQ     contAbortAbort
  CMP     R1, #PSR_SVC_MODE
  ADDEQ   R2, R0, #GC_R13_SVC_OFFS
  BEQ     contAbortAbort
  CMP     R1, #PSR_IRQ_MODE
  ADDEQ   R2, R0, #GC_R13_IRQ_OFFS
  BEQ     contAbortAbort
  CMP     R1, #PSR_FIQ_MODE
  ADDEQ   R2, R0, #GC_R13_FIQ_OFFS
  BEQ     contAbortAbort
  CMP     R1, #PSR_UND_MODE
  ADDEQ   R2, R0, #GC_R13_UND_OFFS
  BEQ     contAbortAbort
  ADD     R2, R0, #GC_R13_OFFS
contAbortAbort:
  CPS     PSR_SYS_MODE
  LDR     SP, [R2]
  LDR     LR, [R2, #4]
  CPS     PSR_SVC_MODE

  /* Lets not lose the guest context pointer.. */
  MOV     LR, R0
  /* Restore general purpose registers R0--R12 */
  LDMIA   LR, {R0-R12}

  PUSH    {R0}
  /* Get guest CPSR into R0 */
  LDR     R0, [LR, #GC_CPSR_OFFS]

  /* Get value of PC saved in guest context into LR */
  LDR     LR, [LR, #GC_R15_OFFS]
  
  /* Construct SPSR in R0: set user mode, disable asynchronous aborts and FIQs, but enable IRQs, and
    restore all other fields from the CPSR*/
#ifdef CONFIG_PROFILER
  BIC     R0, R0, #(PSR_I_BIT | PSR_MODE | PSR_F_BIT)
  ORR     R0, R0, #(PSR_A_BIT | PSR_USR_MODE)
#else
#ifdef CONFIG_HW_PASSTHROUGH
  /* exception bits set from guset context in passthrough */
  BIC     R0, R0, #(PSR_MODE)
  ORR     R0, R0, #(PSR_USR_MODE)
#else
  BIC     R0, R0, #(PSR_I_BIT | PSR_MODE)
  ORR     R0, R0, #(PSR_A_BIT | PSR_F_BIT | PSR_USR_MODE)
#endif
#endif

  MSR     SPSR, R0
  POP     {R0}
#ifdef CONFIG_THUMB2
  STMDB   SP, {LR}
  LDMDB   SP, {PC}^
#else
#ifdef CONFIG_STATS
  recordEndDabt
#endif
  MOVS    PC, LR
#endif


/***************************************************/
/* monitor call handler, guest was in abort mode   */
/***************************************************/
.global abortMonitor
abortMonitor:
  PUSH   {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12}
  /* PC of the aborting instruction in @param1 of C handler */
  SUB    R0, LR, #4
  BL monitorModePrivileged
  /* should never return from that handler, at least for now */
abtMonPrivLoop:
  B      abtMonPrivLoop


/***************************************************/
/* interrupt handler, guest was in abort mode      */
/***************************************************/
.global abortIrq
abortIrq:
#ifdef CONFIG_STATS
  recordStartIrq
#endif
  PUSH   {LR}
  /* Test SPSR -> are we from USR mode? */
  MRS    LR, SPSR
  AND    LR, LR, #PSR_MODE
  CMP    LR, #PSR_USR_MODE

  BNE    irqHandlerPriv

  /* Load guest context into LR and store R0--R12 into guest context. */
  LDR     LR, =guestContextSpace
  LDR     LR, [LR]
  STMIA   LR, {R0-R12}

  /* R0--R12 are now usable, move guest context in there. */
  MOV     R0, LR

  POP     {LR}

/* load guest CPSR into R1 to save everything but AIF flags */
  LDR     R1, [R0, #GC_CPSR_OFFS]
  MOVW    R3, #0x1DF
  AND     R1, R3
  MRS     R2, SPSR
  BIC     R2, R3
  ORR     R1, R1, R2
  STR     R1, [R0, #GC_CPSR_OFFS]

  /* lets save the host IRQ mode LR, as we are not coming back to IRQ mode! */
  MOV     R2, LR

  /* guest context ABT mode R13/R14 register pointer in R1, save state  */
  ADD     R1, R0, #GC_R13_ABT_OFFS
  CPS     PSR_SYS_MODE
  STMIA   R1, {R13, R14}
  CPS     PSR_SVC_MODE

  /* save guest PC in guest context */
#ifdef CONFIG_THUMB2
  /* Load CPSR value from guest context */
  LDR     R3, [R0, #GC_CPSR_OFFS]
  TST     R3, #PSR_T_BIT
  SUBEQ   R2, R2, #4 @ARM
  SUBNE   R2, R2, #2 @Thumb
#else
  SUB     R2, R2, #4
#endif
  STR     R2, [R0, #GC_R15_OFFS]

  /* Pointer to guest context is already in R0! */
  BL      irq

  /* switch to system mode to restore R13 R14 reg: problem is, the guest
   * could have changed modes in the IRQ handler. must check which to restore
   * guest came in IRQ mode. but it could no longer be in IRQ mode */
  LDR     R1, [R0, #GC_CPSR_OFFS]
  AND     R1, R1, #PSR_MODE

  CMP     R1, #PSR_ABT_MODE
  ADDEQ   R2, R0, #GC_R13_ABT_OFFS
  BEQ     contAbtIrq
  CMP     R1, #PSR_SVC_MODE
  ADDEQ   R2, R0, #GC_R13_SVC_OFFS
  BEQ     contAbtIrq
  CMP     R1, #PSR_IRQ_MODE
  ADDEQ   R2, R0, #GC_R13_IRQ_OFFS
  BEQ     contAbtIrq
  CMP     R1, #PSR_FIQ_MODE
  ADDEQ   R2, R0, #GC_R13_FIQ_OFFS
  BEQ     contAbtIrq
  CMP     R1, #PSR_UND_MODE
  ADDEQ   R2, R0, #GC_R13_UND_OFFS
  BEQ     contAbtIrq
  ADD     R2, R0, #GC_R13_OFFS
contAbtIrq:
  CPS     PSR_SYS_MODE
  LDR     SP, [R2]
  LDR     LR, [R2, #4]
  CPS     PSR_SVC_MODE

  /* Lets not loose the guest context pointer.. SP/LR are already free to use */
  MOV     LR, R0
  /* Restore general purpose registers R0--R12 */
  LDMIA   LR, {R0-R12}

  PUSH    {R0}
  /* Get guest CPSR into R0 */
  LDR     R0, [LR, #GC_CPSR_OFFS]

  /* Get value of PC saved in guest context into LR */
  LDR     LR, [LR, #GC_R15_OFFS]
  
  /* Construct SPSR in R0: set user mode, disable asynchronous aborts and FIQs, but enable IRQs, and
    restore all other fields from the CPSR*/
#ifdef CONFIG_PROFILER
  BIC     R0, R0, #(PSR_I_BIT | PSR_MODE | PSR_F_BIT)
  ORR     R0, R0, #(PSR_A_BIT | PSR_USR_MODE)
#else
#ifdef CONFIG_HW_PASSTHROUGH
  /* exception bits set from guset context in passthrough */
  BIC     R0, R0, #(PSR_MODE)
  ORR     R0, R0, #(PSR_USR_MODE)
#else
  BIC     R0, R0, #(PSR_I_BIT | PSR_MODE)
  ORR     R0, R0, #(PSR_A_BIT | PSR_F_BIT | PSR_USR_MODE)
#endif
#endif

#ifdef CONFIG_THUMB2
  TST     R0, #PSR_T_BIT
#endif
  MSR     SPSR, R0
  POP     {R0}
#ifdef CONFIG_THUMB2
  SUBNE   LR, LR, #2
  STMDB   SP, {LR}
  LDMDB   SP, {PC}^
#else
#ifdef CONFIG_STATS
  recordEndIrq
#endif
  MOVS    PC, LR
#endif


/***************************************************/
/* fast interrupt handler, guest was in abort mode */
/***************************************************/
.global abortFiq
abortFiq:
  push    {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
  SUB    r0, lr, #4
  BL fiq
  pop     {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
  SUBS PC, LR, #4
