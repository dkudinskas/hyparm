#ifndef __MISC_INSTRUCTIONS_H__
#define __MISC_INSTRUCTIONS_H__

#include "types.h"
#include "serial.h"
#include "guestContext.h"

// uncomment me for ARM instruction trace: #define ARM_INSTR_TRACE

/* ARM instructions.  */
u32int nopInstruction(GCONTXT * context);
u32int bxInstruction(GCONTXT * context);
u32int mulInstruction(GCONTXT * context);
u32int mlaInstruction(GCONTXT * context);
u32int swpInstruction(GCONTXT * context);
u32int sumullInstruction(GCONTXT * context);
u32int sumlalInstruction(GCONTXT * context);
/* V7 instructions.  */
u32int pliInstruction(GCONTXT * context);
u32int dbgInstruction(GCONTXT * context);
u32int dmbInstruction(GCONTXT * context);
u32int dsbInstruction(GCONTXT * context);
u32int isbInstruction(GCONTXT * context);
/* ARM V6T2 instructions.  */
u32int bfcInstruction(GCONTXT * context);
u32int bfiInstruction(GCONTXT * context);
u32int mlsInstruction(GCONTXT * context);
u32int strhtInstruction(GCONTXT * context);
u32int ldrhtInstruction(GCONTXT * context);
u32int movwInstruction(GCONTXT * context);
u32int movtInstruction(GCONTXT * context);
u32int rbitInstruction(GCONTXT * context);
u32int usbfxInstruction(GCONTXT * context);
/* ARM V6Z instructions.  */
u32int smcInstruction(GCONTXT * context);
/* ARM V6K instructions.  */
u32int clrexInstruction(GCONTXT * context);
/* ARM V6K NOP hints.  */
u32int yieldInstruction(GCONTXT * context);
u32int wfeInstruction(GCONTXT * context);
u32int wfiInstruction(GCONTXT * context);
u32int sevInstruction(GCONTXT * context);
/* ARM V6 instructions.  */
u32int cpsieInstruction(GCONTXT * context);
u32int cpsidInstruction(GCONTXT * context);
u32int cpsInstruction(GCONTXT * context);
u32int pkhbtInstruction(GCONTXT * context);
u32int pkhtbInstruction(GCONTXT * context);
u32int qadd16Instruction(GCONTXT * context);
u32int qadd8Instruction(GCONTXT * context);
u32int qaddsubxInstruction(GCONTXT * context);
u32int qsub16Instruction(GCONTXT * context);
u32int qsub8Instruction(GCONTXT * context);
u32int qsubaddxInstruction(GCONTXT * context);
u32int sadd16Instruction(GCONTXT * context);
u32int sadd8Instruction(GCONTXT * context);
u32int saddsubxInstruction(GCONTXT * context);
u32int shadd16Instruction(GCONTXT * context);
u32int shadd8Instruction(GCONTXT * context);
u32int shaddsubxInstruction(GCONTXT * context);
u32int shsub16Instruction(GCONTXT * context);
u32int shsub8Instruction(GCONTXT * context);
u32int shsubaddxInstruction(GCONTXT * context);
u32int ssub16Instruction(GCONTXT * context);
u32int ssub8Instruction(GCONTXT * context);
u32int ssubaddxInstruction(GCONTXT * context);
u32int uadd16Instruction(GCONTXT * context);
u32int uadd8Instruction(GCONTXT * context);
u32int uaddsubxInstruction(GCONTXT * context);
u32int uhadd16Instruction(GCONTXT * context);
u32int uhadd8Instruction(GCONTXT * context);
u32int uhaddsubxInstruction(GCONTXT * context);
u32int uhsub16Instruction(GCONTXT * context);
u32int uhsub8Instruction(GCONTXT * context);
u32int uhsubaddxInstruction(GCONTXT * context);
u32int uqadd16Instruction(GCONTXT * context);
u32int uqadd8Instruction(GCONTXT * context);
u32int uqaddsubxInstruction(GCONTXT * context);
u32int uqsub16Instruction(GCONTXT * context);
u32int uqsub8Instruction(GCONTXT * context);
u32int uqsubaddxInstruction(GCONTXT * context);
u32int usub16Instruction(GCONTXT * context);
u32int usub8Instruction(GCONTXT * context);
u32int usubaddxInstruction(GCONTXT * context);
u32int revInstruction(GCONTXT * context);
u32int rev16Instruction(GCONTXT * context);
u32int revshInstruction(GCONTXT * context);
u32int rfeInstruction(GCONTXT * context);
u32int sxthInstruction(GCONTXT * context);
u32int sxtb16Instruction(GCONTXT * context);
u32int sxtbInstruction(GCONTXT * context);
u32int uxthInstruction(GCONTXT * context);
u32int uxtb16Instruction(GCONTXT * context);
u32int uxtbInstruction(GCONTXT * context);
u32int sxtahInstruction(GCONTXT * context);
u32int sxtab16Instruction(GCONTXT * context);
u32int sxtabInstruction(GCONTXT * context);
u32int uxtahInstruction(GCONTXT * context);
u32int uxtab16Instruction(GCONTXT * context);
u32int uxtabInstruction(GCONTXT * context);
u32int selInstruction(GCONTXT * context);
u32int setendInstruction(GCONTXT * context);
u32int smuadInstruction(GCONTXT * context);
u32int smusdInstruction(GCONTXT * context);
u32int smladInstruction(GCONTXT * context);
u32int smlaldInstruction(GCONTXT * context);
u32int smlsdInstruction(GCONTXT * context);
u32int smlsldInstruction(GCONTXT * context);
u32int smmulInstruction(GCONTXT * context);
u32int smmlaInstruction(GCONTXT * context);
u32int smmlsInstruction(GCONTXT * context);
u32int srsInstruction(GCONTXT * context);
u32int ssatInstruction(GCONTXT * context);
u32int ssat16Instruction(GCONTXT * context);
u32int umaalInstruction(GCONTXT * context);
u32int usad8Instruction(GCONTXT * context);
u32int usada8Instruction(GCONTXT * context);
u32int usatInstruction(GCONTXT * context);
u32int usat16Instruction(GCONTXT * context);
/* V5J instruction.  */
u32int bxjInstruction(GCONTXT * context);
/* V5 Instructions.  */
u32int bkptInstruction(GCONTXT * context);
u32int blxInstruction(GCONTXT * context);
u32int clzInstruction(GCONTXT * context);
/* V5E "El Segundo" Instructions.  */
u32int pldInstruction(GCONTXT * context);
u32int smlabbInstruction(GCONTXT * context);
u32int smlatbInstruction(GCONTXT * context);
u32int smlabtInstruction(GCONTXT * context);
u32int smlattInstruction(GCONTXT * context);
u32int smlawbInstruction(GCONTXT * context);
u32int smlawtInstruction(GCONTXT * context);
u32int smlalbbInstruction(GCONTXT * context);
u32int smlaltbInstruction(GCONTXT * context);
u32int smlalbtInstruction(GCONTXT * context);
u32int smlalttInstruction(GCONTXT * context);
u32int smulbbInstruction(GCONTXT * context);
u32int smultbInstruction(GCONTXT * context);
u32int smulbtInstruction(GCONTXT * context);
u32int smulttInstruction(GCONTXT * context);
u32int smulwbInstruction(GCONTXT * context);
u32int smulwtInstruction(GCONTXT * context);
u32int qaddInstruction(GCONTXT * context);
u32int qdaddInstruction(GCONTXT * context);
u32int qsubInstruction(GCONTXT * context);
u32int qdsubInstruction(GCONTXT * context);
/* ARM Instructions.  */
u32int msrInstruction(GCONTXT * context);
u32int mrsInstruction(GCONTXT * context);
u32int bInstruction  (GCONTXT * context);
u32int svcInstruction(GCONTXT * context);
u32int undefinedInstruction(GCONTXT * context);

#endif