/*
 * Routines for use from within the JTAG debugger. They are linked just after startup.S, so that
 * when the interrupt handlers are available, these routines are available as well. In case the
 * interrupt handlers don't work, there's not much left we can do anyhow.
 *
 * Upon a crash, you can step into one of the routines below with OpenOCD.
 * First, look up the address in hypervisor.map, then do:
 *
 *   step 0x8f......
 *
 * Do not change PSRs, SCTLR, etc. using the debugger because this often leads to unpredictable
 * behavior. Instead, write a new routine below that makes the desired changes. General-purpose
 * registers can be used as parameters, as they are safe to change with OpenOCD (using 'reg').
 *
 * NOTE: this file *MUST* be included in the build because the linker script relies on it. To
 *       exclude it, the linker script itself must be preprocessed (dirty!).
 */

#ifdef CONFIG_OCD_ROUTINES
#include "cpuArch/constants.h"

.syntax unified
.arm
.text

/*
 * disableMMU (not exported)
 *
 * Disables the MMU and performs the necessary barriers.
 */
disableMMU:
.func disableMMU
  MRC     p15, 0, r0, c1, c0, 0
  BIC     r0, r0, #1
  MCR     p15, 0, r0, c1, c0, 0
  ISB     SY
  DSB     SY
  BX      LR
.endfunc


/*
 * ocdDisableMMU
 *
 * Routine to disable the MMU, usable from within the debugger.
 */
.global ocdDisableMMU
.func ocdDisableMMU
ocdDisableMMU:
  NOP
  BL      disableMMU
  BKPT    #0
.endfunc


/*
 * ocdDumpLastTranslationTable
 *
 * This routine switches to SVC mode, turns off the MMU and dumps the translation table referred to
 * by TTBR0. This routine relies on a 1:1 mapping of the hypervisor in physical memory, and can be
 * used to dump 'broken'/'corrupt' translation tables. When the dump completes succesfully, the
 * ends in a software breakpoint.
 */
.global ocdDumpLastTranslationTable
.func ocdDumpLastTranslationTable
ocdDumpLastTranslationTable:
  NOP
  CPSID   aif, #PSR_SVC_MODE
  BL      disableMMU
  MRC     p15, 0, r0, c2, c0, 0
  BFC     r0, #0, #14
  BL      dumpTranslationTable
  BKPT    #0
.endfunc

#endif /* CONFIG_OCD_ROUTINES */

