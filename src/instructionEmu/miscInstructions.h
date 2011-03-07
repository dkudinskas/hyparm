#ifndef __MISC_INSTRUCTIONS_H__
#define __MISC_INSTRUCTIONS_H__
#include "types.h"
#include "serial.h"
#include "guestContext.h"
// uncomment me for ARM instruction trace: #define ARM_INSTR_TRACE
/* ARM instructions.  */

u32int nopPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int nopInstruction(GCONTXT * context);

u32int bxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int bxInstruction(GCONTXT * context);

u32int mulPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int mulInstruction(GCONTXT * context);

u32int mlaPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int mlaInstruction(GCONTXT * context);

u32int swpPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int swpInstruction(GCONTXT * context);

u32int sumullPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int sumullInstruction(GCONTXT * context);

u32int sumlalPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int sumlalInstruction(GCONTXT * context);
/* V7 instructions.  */

u32int pliPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int pliInstruction(GCONTXT * context);

u32int dbgPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int dbgInstruction(GCONTXT * context);

u32int dmbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int dmbInstruction(GCONTXT * context);

u32int dsbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int dsbInstruction(GCONTXT * context);

u32int isbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int isbInstruction(GCONTXT * context);
/* ARM V6T2 instructions.  */

u32int bfcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int bfcInstruction(GCONTXT * context);

u32int bfiPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int bfiInstruction(GCONTXT * context);

u32int mlsPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int mlsInstruction(GCONTXT * context);

u32int movwPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int movwInstruction(GCONTXT * context);

u32int movtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int movtInstruction(GCONTXT * context);

u32int rbitPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int rbitInstruction(GCONTXT * context);

u32int usbfxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int usbfxInstruction(GCONTXT * context);
/* ARM V6Z instructions.  */

u32int smcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int smcInstruction(GCONTXT * context);
/* ARM V6K instructions.  */

u32int clrexPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int clrexInstruction(GCONTXT * context);
/* ARM V6K NOP hints.  */

u32int yieldPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int yieldInstruction(GCONTXT * context);

u32int wfePCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int wfeInstruction(GCONTXT * context);

u32int wfiPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int wfiInstruction(GCONTXT * context);

u32int sevPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int sevInstruction(GCONTXT * context);
/* ARM V6 instructions.  */

u32int cpsiePCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int cpsieInstruction(GCONTXT * context);

u32int cpsidPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int cpsidInstruction(GCONTXT * context);

u32int cpsPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int cpsInstruction(GCONTXT * context);

u32int pkhbtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int pkhbtInstruction(GCONTXT * context);

u32int pkhtbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int pkhtbInstruction(GCONTXT * context);

u32int qadd16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int qadd16Instruction(GCONTXT * context);

u32int qadd8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int qadd8Instruction(GCONTXT * context);

u32int qaddsubxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int qaddsubxInstruction(GCONTXT * context);

u32int qsub16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int qsub16Instruction(GCONTXT * context);

u32int qsub8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int qsub8Instruction(GCONTXT * context);

u32int qsubaddxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int qsubaddxInstruction(GCONTXT * context);

u32int sadd16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int sadd16Instruction(GCONTXT * context);

u32int sadd8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int sadd8Instruction(GCONTXT * context);

u32int saddsubxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int saddsubxInstruction(GCONTXT * context);

u32int shadd16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int shadd16Instruction(GCONTXT * context);

u32int shadd8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int shadd8Instruction(GCONTXT * context);

u32int shaddsubxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int shaddsubxInstruction(GCONTXT * context);

u32int shsub16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int shsub16Instruction(GCONTXT * context);

u32int shsub8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int shsub8Instruction(GCONTXT * context);

u32int shsubaddxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int shsubaddxInstruction(GCONTXT * context);

u32int ssub16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int ssub16Instruction(GCONTXT * context);

u32int ssub8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int ssub8Instruction(GCONTXT * context);

u32int ssubaddxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int ssubaddxInstruction(GCONTXT * context);

u32int uadd16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int uadd16Instruction(GCONTXT * context);

u32int uadd8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int uadd8Instruction(GCONTXT * context);

u32int uaddsubxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int uaddsubxInstruction(GCONTXT * context);

u32int uhadd16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int uhadd16Instruction(GCONTXT * context);

u32int uhadd8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int uhadd8Instruction(GCONTXT * context);

u32int uhaddsubxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int uhaddsubxInstruction(GCONTXT * context);

u32int uhsub16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int uhsub16Instruction(GCONTXT * context);

u32int uhsub8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int uhsub8Instruction(GCONTXT * context);

u32int uhsubaddxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int uhsubaddxInstruction(GCONTXT * context);

u32int uqadd16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int uqadd16Instruction(GCONTXT * context);

u32int uqadd8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int uqadd8Instruction(GCONTXT * context);

u32int uqaddsubxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int uqaddsubxInstruction(GCONTXT * context);

u32int uqsub16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int uqsub16Instruction(GCONTXT * context);

u32int uqsub8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int uqsub8Instruction(GCONTXT * context);

u32int uqsubaddxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int uqsubaddxInstruction(GCONTXT * context);

u32int usub16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int usub16Instruction(GCONTXT * context);

u32int usub8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int usub8Instruction(GCONTXT * context);

u32int usubaddxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int usubaddxInstruction(GCONTXT * context);

u32int revPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int revInstruction(GCONTXT * context);

u32int rev16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int rev16Instruction(GCONTXT * context);

u32int revshPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int revshInstruction(GCONTXT * context);

u32int rfePCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int rfeInstruction(GCONTXT * context);

u32int sxthPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int sxthInstruction(GCONTXT * context);

u32int sxtb16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int sxtb16Instruction(GCONTXT * context);

u32int sxtbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int sxtbInstruction(GCONTXT * context);

u32int uxthPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int uxthInstruction(GCONTXT * context);

u32int uxtb16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int uxtb16Instruction(GCONTXT * context);

u32int uxtbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int uxtbInstruction(GCONTXT * context);

u32int sxtahPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int sxtahInstruction(GCONTXT * context);

u32int sxtab16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int sxtab16Instruction(GCONTXT * context);

u32int sxtabPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int sxtabInstruction(GCONTXT * context);

u32int uxtahPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int uxtahInstruction(GCONTXT * context);

u32int uxtab16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int uxtab16Instruction(GCONTXT * context);

u32int uxtabPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int uxtabInstruction(GCONTXT * context);

u32int selPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int selInstruction(GCONTXT * context);

u32int setendPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int setendInstruction(GCONTXT * context);

u32int smuadPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int smuadInstruction(GCONTXT * context);

u32int smusdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int smusdInstruction(GCONTXT * context);

u32int smladPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int smladInstruction(GCONTXT * context);

u32int smlaldPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int smlaldInstruction(GCONTXT * context);

u32int smlsdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int smlsdInstruction(GCONTXT * context);

u32int smlsldPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int smlsldInstruction(GCONTXT * context);

u32int smmulPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int smmulInstruction(GCONTXT * context);

u32int smmlaPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int smmlaInstruction(GCONTXT * context);

u32int smmlsPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int smmlsInstruction(GCONTXT * context);

u32int srsPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int srsInstruction(GCONTXT * context);

u32int ssatPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int ssatInstruction(GCONTXT * context);

u32int ssat16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int ssat16Instruction(GCONTXT * context);

u32int umaalPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int umaalInstruction(GCONTXT * context);

u32int usad8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int usad8Instruction(GCONTXT * context);

u32int usada8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int usada8Instruction(GCONTXT * context);

u32int usatPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int usatInstruction(GCONTXT * context);

u32int usat16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int usat16Instruction(GCONTXT * context);
/* V5J instruction.  */

u32int bxjPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int bxjInstruction(GCONTXT * context);
/* V5 Instructions.  */

u32int bkptPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int bkptInstruction(GCONTXT * context);

u32int blxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int blxInstruction(GCONTXT * context);

u32int clzPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int clzInstruction(GCONTXT * context);
/* V5E "El Segundo" Instructions.  */

u32int pldPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int pldInstruction(GCONTXT * context);

u32int smlabbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int smlabbInstruction(GCONTXT * context);

u32int smlatbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int smlatbInstruction(GCONTXT * context);

u32int smlabtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int smlabtInstruction(GCONTXT * context);

u32int smlattPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int smlattInstruction(GCONTXT * context);

u32int smlawbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int smlawbInstruction(GCONTXT * context);

u32int smlawtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int smlawtInstruction(GCONTXT * context);

u32int smlalbbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int smlalbbInstruction(GCONTXT * context);

u32int smlaltbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int smlaltbInstruction(GCONTXT * context);

u32int smlalbtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int smlalbtInstruction(GCONTXT * context);

u32int smlalttPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int smlalttInstruction(GCONTXT * context);

u32int smulbbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int smulbbInstruction(GCONTXT * context);

u32int smultbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int smultbInstruction(GCONTXT * context);

u32int smulbtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int smulbtInstruction(GCONTXT * context);

u32int smulttPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int smulttInstruction(GCONTXT * context);

u32int smulwbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int smulwbInstruction(GCONTXT * context);

u32int smulwtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int smulwtInstruction(GCONTXT * context);

u32int qaddPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int qaddInstruction(GCONTXT * context);

u32int qdaddPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int qdaddInstruction(GCONTXT * context);

u32int qsubPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int qsubInstruction(GCONTXT * context);

u32int qdsubPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int qdsubInstruction(GCONTXT * context);
/* ARM Instructions.  */

u32int msrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int msrInstruction(GCONTXT * context);

u32int mrsPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int mrsInstruction(GCONTXT * context);

u32int bPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int bInstruction(GCONTXT * context);

u32int svcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int svcInstruction(GCONTXT * context);

u32int undefinedPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int undefinedInstruction(GCONTXT * context);
#endif
