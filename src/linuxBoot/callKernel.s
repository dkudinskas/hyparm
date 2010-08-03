.global callKernel
.func   callKernel
callKernel:
  /* Set USR mode in the SPSR */
  MSR SPSR, #0xD0

  /* Load the entry point onto the stack, then use the Load PC + copy SPSR to CPSR to jump into USR mode */
  STM SP, {r3}
  LDM SP, {PC}^
.endfunc
