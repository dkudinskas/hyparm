/***************************************************************
 * guest in svc mode - exception vector
 ***************************************************************/
.text

.global svcReset
svcReset:
  B       svcReset

/***************************************************/
/* undefined error handler, guest was in svc mode  */
/***************************************************/
.global svcUndefined
svcUndefined:
  B   svcUndefined


/***************************************************/
/* serice call handler, guest was in svc mode      */
/***************************************************/
.global svcSvc
svcSvc:
  PUSH   {LR}

  /* Load guest context into LR and store R0--R12 into guest context. */
  LDR     LR, =guestContextSpace
  LDR     LR, [LR]
  STMIA   LR, {R0-R12}

  /* R0--R12 are now usable, move guest context in there. */
  MOV     R0, LR

  POP     {LR}

  /* load guest CPSR into R1 to save condition flags */
  LDR     R1, [R0, #GC_CPSR_OFFS]
  AND     R1, #0x0FFFFFFF
  MRS     R2, SPSR
  AND     R2, #0xF0000000
  ORR     R1, R1, R2
  STR     R1, [R0, #GC_CPSR_OFFS]

  /* guest context svc mode R13/R14 register pointer in R1, save state  */
  ADD     R1, R0, #GC_R13_SVC_OFFS
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
  BEQ     contSvcSvc
  CMP     R1, #PSR_ABT_MODE
  ADDEQ   R2, R0, #GC_R13_ABT_OFFS
  BEQ     contSvcSvc
  CMP     R1, #PSR_IRQ_MODE
  ADDEQ   R2, R0, #GC_R13_IRQ_OFFS
  BEQ     contSvcSvc
  CMP     R1, #PSR_FIQ_MODE
  ADDEQ   R2, R0, #GC_R13_FIQ_OFFS
  BEQ     contSvcSvc
  CMP     R1, #PSR_UND_MODE
  ADDEQ   R2, R0, #GC_R13_UND_OFFS
  BEQ     contSvcSvc
  ADD     R2, R0, #GC_R13_OFFS
contSvcSvc:
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
#ifdef CONFIG_THUMB2
  /* Construct SPSR in R0: restore condition flags from the CPSR in the guest context
   * set user mode, disable asynchronous aborts and FIQs
   * but enable IRQs, and restore the Thumb bit */
  TST     R0, #PSR_T_BIT
  AND     R0, R0, #PSR_CC_FLAGS_NZCV
  ORREQ   R0, R0, #(PSR_USR_MODE | PSR_A_BIT | PSR_F_BIT)
  ORRNE   R0, R0, #(PSR_USR_MODE | PSR_A_BIT | PSR_F_BIT | PSR_T_BIT)
  MSR     SPSR, R0
  /* Also update guest PC when guest is in Thumb mode, then save SPSR */
  LDR     LR, [LR, #GC_R15_OFFS]
  SUBNE   LR, LR, #2
  POP     {R0}
#else
  /* Construct SPSR in R0: restore condition flags from guest CPSR
   * set user mode, disable async aborts and FIQs, but enable IRQs, then save to SPSR */
  AND     R0, R0, #PSR_CC_FLAGS_NZCV
  ORR     R0, R0, #(PSR_USR_MODE | PSR_A_BIT | PSR_F_BIT)
  MSR     SPSR, R0
  POP     {R0}

  LDR     LR, [LR, #GC_R15_OFFS]
#endif
  MOVS      PC, LR


/***************************************************/
/* prefetch abort handler, guest was in svc mode   */
/***************************************************/
.global svcPrefetch
svcPrefetch:
  B   svcPrefetch


/***************************************************/
/* data abort handler, guest was in svc mode       */
/***************************************************/
.global svcDataAbort
svcDataAbort:
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

  /* load guest CPSR into R1 to save condition flags */
  LDR     R1, [R0, #GC_CPSR_OFFS]
  AND     R1, #0x0FFFFFFF
  MRS     R2, SPSR
  AND     R2, #0xF0000000
  ORR     R1, R1, R2
  STR     R1, [R0, #GC_CPSR_OFFS]
  
  /* lets save the host abort mode LR, as we are not coming back to ABT mode! */
  MOV     R2, LR

  /* guest context SVC mode R13/R14 register pointer in R1, save state  */
  ADD     R1, R0, #GC_R13_SVC_OFFS
  CPS     PSR_SYS_MODE
  STMIA   R1, {R13, R14}
  CPS     PSR_SVC_MODE

  /* save guest PC in guest context */
#ifdef CONFIG_THUMB2
  /* Load CPSR value from guest context */
  /* previously there was no separate implementation of this for THUMB. */
  /* is that correct? die, and checkit out if we hit it. */
  .global svcThumbModeAbort
svcThumbModeAbort:
  B       svcThumbModeAbort
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

  CMP     R1, #PSR_SVC_MODE
  ADDEQ   R2, R0, #GC_R13_SVC_OFFS
  BEQ     contSvcDabt
  CMP     R1, #PSR_ABT_MODE
  ADDEQ   R2, R0, #GC_R13_ABT_OFFS
  BEQ     contSvcDabt
  CMP     R1, #PSR_IRQ_MODE
  ADDEQ   R2, R0, #GC_R13_IRQ_OFFS
  BEQ     contSvcDabt
  CMP     R1, #PSR_FIQ_MODE
  ADDEQ   R2, R0, #GC_R13_FIQ_OFFS
  BEQ     contSvcDabt
  CMP     R1, #PSR_UND_MODE
  ADDEQ   R2, R0, #GC_R13_UND_OFFS
  BEQ     contSvcDabt
  ADD     R2, R0, #GC_R13_OFFS
contSvcDabt:
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

#ifdef CONFIG_THUMB2
  /* Construct SPSR in R0: restore condition flags from the CPSR in the guest context
   * set user mode, disable asynchronous aborts and FIQs
   * but enable IRQs, and restore the Thumb bit */
  TST     R0, #PSR_T_BIT
  AND     R0, R0, #PSR_CC_FLAGS_NZCV
  ORREQ   R0, R0, #(PSR_USR_MODE | PSR_A_BIT | PSR_F_BIT)
  ORRNE   R0, R0, #(PSR_USR_MODE | PSR_A_BIT | PSR_F_BIT | PSR_T_BIT)
  MSR     SPSR, R0
  /* Also update guest PC when guest is in Thumb mode */
  SUBNE   LR, LR, #2
#else
  /* Construct SPSR in R0: restore condition flags from guest CPSR
   * set user mode, disable async aborts and FIQs, but enable IRQs, then save to SPSR */
  AND     R0, R0, #PSR_CC_FLAGS_NZCV
  ORR     R0, R0, #(PSR_USR_MODE | PSR_A_BIT | PSR_F_BIT)
  MSR     SPSR, R0
#endif
  POP     {R0}

  MOVS      PC, LR
  

/***************************************************/
/* monitor call handler, guest was in svc mode     */
/***************************************************/
.global svcMonitor
svcMonitor:
  PUSH   {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12}
  /* PC of the aborting instruction in @param1 of C handler */
  SUB    R0, LR, #4
  BL monitorModePrivileged
  /* should never return from that handler, at least for now */
svcMonPrivLoop:
  B      svcMonPrivLoop



/***************************************************/
/* interrupt handler, guest was in svc mode        */
/***************************************************/
.global svcIrq
svcIrq:
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

  /* load guest CPSR into R1 to save condition flags */
  LDR     R1, [R0, #GC_CPSR_OFFS]
  AND     R1, #0x0FFFFFFF
  MRS     R2, SPSR
  AND     R2, #0xF0000000
  ORR     R1, R1, R2
  STR     R1, [R0, #GC_CPSR_OFFS]

  /* lets save the host IRQ mode LR, as we are not coming back to IRQ mode! */
  MOV     R2, LR

  /* guest context SVC mode R13/R14 register pointer in R1, save state  */
  ADD     R1, R0, #GC_R13_SVC_OFFS
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
   * could have changed modes in the irq handler. must check which to restore
   * guest came in IRQ mode. but it could no longer be in IRQ mode */
  LDR     R1, [R0, #GC_CPSR_OFFS]
  AND     R1, R1, #PSR_MODE

  CMP     R1, #PSR_ABT_MODE
  ADDEQ   R2, R0, #GC_R13_ABT_OFFS
  BEQ     contSvcIrq
  CMP     R1, #PSR_SVC_MODE
  ADDEQ   R2, R0, #GC_R13_SVC_OFFS
  BEQ     contSvcIrq
  CMP     R1, #PSR_IRQ_MODE
  ADDEQ   R2, R0, #GC_R13_IRQ_OFFS
  BEQ     contSvcIrq
  CMP     R1, #PSR_FIQ_MODE
  ADDEQ   R2, R0, #GC_R13_FIQ_OFFS
  BEQ     contSvcIrq
  CMP     R1, #PSR_UND_MODE
  ADDEQ   R2, R0, #GC_R13_UND_OFFS
  BEQ     contSvcIrq
  ADD     R2, R0, #GC_R13_OFFS
contSvcIrq:
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

#ifdef CONFIG_THUMB2
  /* Construct SPSR in R0: restore condition flags from the CPSR in the guest context
   * set user mode, disable asynchronous aborts and FIQs
   * but enable IRQs, and restore the Thumb bit */
  TST     R0, #PSR_T_BIT
  AND     R0, R0, #PSR_CC_FLAGS_NZCV
  ORREQ   R0, R0, #(PSR_USR_MODE | PSR_A_BIT | PSR_F_BIT)
  ORRNE   R0, R0, #(PSR_USR_MODE | PSR_A_BIT | PSR_F_BIT | PSR_T_BIT)
  MSR     SPSR, R0
  /* Also update guest PC when guest is in Thumb mode */
  SUBNE   LR, LR, #2
#else
  /* Construct SPSR in R0: restore condition flags from guest CPSR
   * set user mode, disable async aborts and FIQs, but enable IRQs, then save to SPSR */
  AND     R0, R0, #PSR_CC_FLAGS_NZCV
  ORR     R0, R0, #(PSR_USR_MODE | PSR_A_BIT | PSR_F_BIT)
  MSR     SPSR, R0
#endif
  POP     {R0}

  MOVS      PC, LR


/***************************************************/
/* fast interrupt handler, guest was in svc mode   */
/***************************************************/
.global svcFiq
svcFiq:
  /* this is not implemented really */
  STMFD SP!, {LR}
  STMFD SP!, {R0-R12}

  BL fiq

  LDMFD SP!, {R0-R12}
  LDMFD SP!, {PC}^
