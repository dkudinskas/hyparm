#ifndef __INSTRUCTION_EMU__MISC_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__MISC_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


// uncomment me for ARM instruction trace: #define ARM_INSTR_TRACE

#define SXTH_R0     0
#define SXTH_R8     8
#define SXTH_R16   16
#define SXTH_R24   24


/* ARM instructions.  */
u32int nopInstruction(GCONTXT *context, u32int instruction);
u32int mulInstruction(GCONTXT *context, u32int instruction);
u32int mlaInstruction(GCONTXT *context, u32int instruction);
u32int swpInstruction(GCONTXT *context, u32int instruction);
u32int sumullInstruction(GCONTXT *context, u32int instruction);
u32int sumlalInstruction(GCONTXT *context, u32int instruction);
/* V7 instructions.  */
u32int pliInstruction(GCONTXT *context, u32int instruction);
u32int dbgInstruction(GCONTXT *context, u32int instruction);
u32int dmbInstruction(GCONTXT *context, u32int instruction);
u32int dsbInstruction(GCONTXT *context, u32int instruction);
u32int isbInstruction(GCONTXT *context, u32int instruction);
/* ARM V6T2 instructions.  */
u32int bfcInstruction(GCONTXT *context, u32int instruction);
u32int bfiInstruction(GCONTXT *context, u32int instruction);
u32int mlsInstruction(GCONTXT *context, u32int instruction);
u32int movwInstruction(GCONTXT *context, u32int instruction);
u32int movtInstruction(GCONTXT *context, u32int instruction);
u32int rbitInstruction(GCONTXT *context, u32int instruction);
u32int usbfxInstruction(GCONTXT *context, u32int instruction);
/* ARM V6Z instructions.  */
u32int smcInstruction(GCONTXT *context, u32int instruction);
/* ARM V6K instructions.  */
u32int clrexInstruction(GCONTXT *context, u32int instruction);
/* ARM V6K NOP hints.  */
u32int yieldInstruction(GCONTXT *context, u32int instruction);
u32int wfeInstruction(GCONTXT *context, u32int instruction);
u32int wfiInstruction(GCONTXT *context, u32int instruction);
u32int sevInstruction(GCONTXT *context, u32int instruction);
/* ARM V6 instructions.  */
u32int cpsInstruction(GCONTXT *context, u32int instruction);
u32int pkhbtInstruction(GCONTXT *context, u32int instruction);
u32int pkhtbInstruction(GCONTXT *context, u32int instruction);
u32int qadd16Instruction(GCONTXT *context, u32int instruction);
u32int qadd8Instruction(GCONTXT *context, u32int instruction);
u32int qaddsubxInstruction(GCONTXT *context, u32int instruction);
u32int qsub16Instruction(GCONTXT *context, u32int instruction);
u32int qsub8Instruction(GCONTXT *context, u32int instruction);
u32int qsubaddxInstruction(GCONTXT *context, u32int instruction);
u32int sadd16Instruction(GCONTXT *context, u32int instruction);
u32int sadd8Instruction(GCONTXT *context, u32int instruction);
u32int saddsubxInstruction(GCONTXT *context, u32int instruction);
u32int shadd16Instruction(GCONTXT *context, u32int instruction);
u32int shadd8Instruction(GCONTXT *context, u32int instruction);
u32int shaddsubxInstruction(GCONTXT *context, u32int instruction);
u32int shsub16Instruction(GCONTXT *context, u32int instruction);
u32int shsub8Instruction(GCONTXT *context, u32int instruction);
u32int shsubaddxInstruction(GCONTXT *context, u32int instruction);
u32int ssub16Instruction(GCONTXT *context, u32int instruction);
u32int ssub8Instruction(GCONTXT *context, u32int instruction);
u32int ssubaddxInstruction(GCONTXT *context, u32int instruction);
u32int uadd16Instruction(GCONTXT *context, u32int instruction);
u32int uadd8Instruction(GCONTXT *context, u32int instruction);
u32int uaddsubxInstruction(GCONTXT *context, u32int instruction);
u32int uhadd16Instruction(GCONTXT *context, u32int instruction);
u32int uhadd8Instruction(GCONTXT *context, u32int instruction);
u32int uhaddsubxInstruction(GCONTXT *context, u32int instruction);
u32int uhsub16Instruction(GCONTXT *context, u32int instruction);
u32int uhsub8Instruction(GCONTXT *context, u32int instruction);
u32int uhsubaddxInstruction(GCONTXT *context, u32int instruction);
u32int uqadd16Instruction(GCONTXT *context, u32int instruction);
u32int uqadd8Instruction(GCONTXT *context, u32int instruction);
u32int uqaddsubxInstruction(GCONTXT *context, u32int instruction);
u32int uqsub16Instruction(GCONTXT *context, u32int instruction);
u32int uqsub8Instruction(GCONTXT *context, u32int instruction);
u32int uqsubaddxInstruction(GCONTXT *context, u32int instruction);
u32int usub16Instruction(GCONTXT *context, u32int instruction);
u32int usub8Instruction(GCONTXT *context, u32int instruction);
u32int usubaddxInstruction(GCONTXT *context, u32int instruction);
u32int revInstruction(GCONTXT *context, u32int instruction);
u32int rev16Instruction(GCONTXT *context, u32int instruction);
u32int revshInstruction(GCONTXT *context, u32int instruction);
u32int rfeInstruction(GCONTXT *context, u32int instruction);
u32int sxthInstruction(GCONTXT *context, u32int instruction);
u32int sxtb16Instruction(GCONTXT *context, u32int instruction);
u32int sxtbInstruction(GCONTXT *context, u32int instruction);
u32int uxthInstruction(GCONTXT *context, u32int instruction);
u32int uxtb16Instruction(GCONTXT *context, u32int instruction);
u32int uxtbInstruction(GCONTXT *context, u32int instruction);
u32int sxtahInstruction(GCONTXT *context, u32int instruction);
u32int sxtab16Instruction(GCONTXT *context, u32int instruction);
u32int sxtabInstruction(GCONTXT *context, u32int instruction);
u32int uxtahInstruction(GCONTXT *context, u32int instruction);
u32int uxtab16Instruction(GCONTXT *context, u32int instruction);
u32int uxtabInstruction(GCONTXT *context, u32int instruction);
u32int selInstruction(GCONTXT *context, u32int instruction);
u32int setendInstruction(GCONTXT *context, u32int instruction);
u32int smuadInstruction(GCONTXT *context, u32int instruction);
u32int smusdInstruction(GCONTXT *context, u32int instruction);
u32int smladInstruction(GCONTXT *context, u32int instruction);
u32int smlaldInstruction(GCONTXT *context, u32int instruction);
u32int smlsdInstruction(GCONTXT *context, u32int instruction);
u32int smlsldInstruction(GCONTXT *context, u32int instruction);
u32int smmulInstruction(GCONTXT *context, u32int instruction);
u32int smmlaInstruction(GCONTXT *context, u32int instruction);
u32int smmlsInstruction(GCONTXT *context, u32int instruction);
u32int srsInstruction(GCONTXT *context, u32int instruction);
u32int ssatInstruction(GCONTXT *context, u32int instruction);
u32int ssat16Instruction(GCONTXT *context, u32int instruction);
u32int umaalInstruction(GCONTXT *context, u32int instruction);
u32int usad8Instruction(GCONTXT *context, u32int instruction);
u32int usada8Instruction(GCONTXT *context, u32int instruction);
u32int usatInstruction(GCONTXT *context, u32int instruction);
u32int usat16Instruction(GCONTXT *context, u32int instruction);

/* V5 Instructions.  */
u32int bkptInstruction(GCONTXT *context, u32int instruction);
u32int clzInstruction(GCONTXT *context, u32int instruction);
/* V5E "El Segundo" Instructions.  */
u32int pldInstruction(GCONTXT *context, u32int instruction);
u32int smlabbInstruction(GCONTXT *context, u32int instruction);
u32int smlatbInstruction(GCONTXT *context, u32int instruction);
u32int smlabtInstruction(GCONTXT *context, u32int instruction);
u32int smlattInstruction(GCONTXT *context, u32int instruction);
u32int smlawbInstruction(GCONTXT *context, u32int instruction);
u32int smlawtInstruction(GCONTXT *context, u32int instruction);
u32int smlalbbInstruction(GCONTXT *context, u32int instruction);
u32int smlaltbInstruction(GCONTXT *context, u32int instruction);
u32int smlalbtInstruction(GCONTXT *context, u32int instruction);
u32int smlalttInstruction(GCONTXT *context, u32int instruction);
u32int smulbbInstruction(GCONTXT *context, u32int instruction);
u32int smultbInstruction(GCONTXT *context, u32int instruction);
u32int smulbtInstruction(GCONTXT *context, u32int instruction);
u32int smulttInstruction(GCONTXT *context, u32int instruction);
u32int smulwbInstruction(GCONTXT *context, u32int instruction);
u32int smulwtInstruction(GCONTXT *context, u32int instruction);
u32int smullInstruction(GCONTXT *context, u32int instruction);
u32int qaddInstruction(GCONTXT *context, u32int instruction);
u32int qdaddInstruction(GCONTXT *context, u32int instruction);
u32int qsubInstruction(GCONTXT *context, u32int instruction);
u32int qdsubInstruction(GCONTXT *context, u32int instruction);
/* ARM Instructions.  */
u32int msrInstruction(GCONTXT *context, u32int instruction);
u32int mrsInstruction(GCONTXT *context, u32int instruction);
u32int svcInstruction(GCONTXT *context, u32int instruction);
u32int undefinedInstruction(GCONTXT *context, u32int instruction);

#ifdef CONFIG_THUMB2
/* Exclusive Thumb Instructions. */
u32int t16ItInstruction(GCONTXT *context, u32int instruction);
#endif

#endif
