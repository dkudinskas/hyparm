#ifdef CONFIG_STATS
.macro recordStartSvc
  PUSH    {R0, R1}
  MRC     p15, 0, R1, c9, c13, 0
  LDR     R0, =timerStartSvc
  STR     R1, [R0]
  POP     {R0, R1}
.endm

.macro recordEndSvc
  PUSH    {R0, R1, r2}
  MRC     p15, 0, R1, c9, c13, 0    /* read second cycle counter value */
  LDR     R0, =timerStartSvc        /* get starting value address */
  LDR     R0, [R0]                  /* load starting value */
  SUB     R1, R1, R0                /* calculate delta */
  /* now we have the cycle count in R1. lets add to total and inc count */
  LDR     R0, =timerTotalSvc;
  LDR     R2, [R0]                  /* total now in r2 */
  ADD     R2, R1, R2                /* updated total in r2 */
  STR     R2, [R0]                  /* total updated */
  LDR     R0, =timerNumberSvc;
  LDR     R2, [R0]                  /* count now in r2 */
  ADD     R2, R2, #1                /* incremented count */
  STR     R2, [R0]                  /* count updated in guest context */

  POP     {R0, R1, R2}
.endm

.macro recordStartDabt
  PUSH    {R0, R1}
  MRC     p15, 0, R1, c9, c13, 0
  LDR     R0, =timerStartDabt
  STR     R1, [R0]
  POP     {R0, R1}
.endm

.macro recordEndDabt
  PUSH    {R0, R1, r2}
  MRC     p15, 0, R1, c9, c13, 0    /* read second cycle counter value */
  LDR     R0, =timerStartDabt       /* get starting value address */
  LDR     R0, [R0]                  /* load starting value */
  SUB     R1, R1, R0                /* calculate delta */
  /* now we have the cycle count in R1. lets add to total and inc count */
  LDR     R0, =timerTotalDabt;
  LDR     R2, [R0]                  /* total now in r2 */
  ADD     R2, R1, R2                /* updated total in r2 */
  STR     R2, [R0]                  /* total updated */
  LDR     R0, =timerNumberDabt;
  LDR     R2, [R0]                  /* count now in r2 */
  ADD     R2, R2, #1                /* incremented count */
  STR     R2, [R0]                  /* count updated in guest context */
   
  POP     {R0, R1, R2}
.endm


.macro recordStartIrq
  PUSH    {R0, R1}
  MRC     p15, 0, R1, c9, c13, 0
  LDR     R0, =timerStartIrq
  STR     R1, [R0]
  POP     {R0, R1}
.endm

.macro recordEndIrq
  PUSH    {R0, R1, r2}
  MRC     p15, 0, R1, c9, c13, 0    /* read second cycle counter value */
  LDR     R0, =timerStartIrq        /* get starting value address */
  LDR     R0, [R0]                  /* load starting value */
  SUB     R1, R1, R0                /* calculate delta */
  /* now we have the cycle count in R1. lets add to total and inc count */
  LDR     R0, =timerTotalIrq;
  LDR     R2, [R0]                  /* total now in r2 */
  ADD     R2, R1, R2                /* updated total in r2 */
  STR     R2, [R0]                  /* total updated */
  LDR     R0, =timerNumberIrq;
  LDR     R2, [R0]                  /* count now in r2 */
  ADD     R2, R2, #1                /* incremented count */
  STR     R2, [R0]                  /* count updated in guest context */
   
  POP     {R0, R1, R2}
.endm


.macro recordStartPabt
  PUSH    {R0, R1}
  MRC     p15, 0, R1, c9, c13, 0
  LDR     R0, =timerStartPabt
  STR     R1, [R0]
  POP     {R0, R1}
.endm

.macro recordEndPabt
  PUSH    {R0, R1, r2}
  MRC     p15, 0, R1, c9, c13, 0    /* read second cycle counter value */
  LDR     R0, =timerStartPabt       /* get starting value address */
  LDR     R0, [R0]                  /* load starting value */
  SUB     R1, R1, R0                /* calculate delta */
  /* now we have the cycle count in R1. lets add to total and inc count */
  LDR     R0, =timerTotalPabt;
  LDR     R2, [R0]                  /* total now in r2 */
  ADD     R2, R1, R2                /* updated total in r2 */
  STR     R2, [R0]                  /* total updated */
  LDR     R0, =timerNumberPabt;
  LDR     R2, [R0]                  /* count now in r2 */
  ADD     R2, R2, #1                /* incremented count */
  STR     R2, [R0]                  /* count updated in guest context */
   
  POP     {R0, R1, R2}
.endm

#endif

