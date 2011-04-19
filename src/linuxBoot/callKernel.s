.global callKernel
.func   callKernel
callKernel:
  /* assuming stack has been setup!! */
  PUSH   {R0}
  /* Set USR mode in the SPSR */
  MRS    R0, SPSR
  BIC    R0, #0x1F
  ORR    R0, #0x10
  /* disable async and FIQ's */
  ORR    R0, #0x100
#ifdef CONFIG_BLOCK_COPY_NO_IRQ
  /*Make sure interupts are disabled*/
  ORR    R0, #0x80
#endif
  ORR    R0, #0x40
  MSR    SPSR, R0
  POP    {R0}

  /* Load the entry point onto the stack, then use the Load PC + copy SPSR to CPSR to jump into USR mode */
  STM SP, {r3}
  LDM SP, {PC}^
.endfunc
