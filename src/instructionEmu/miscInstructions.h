#ifndef __INSTRUCTION_EMU__MISC_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__MISC_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


// uncomment me for ARM instruction trace: #define ARM_INSTR_TRACE
/* ARM instructions.  */

#ifdef CONFIG_BLOCK_COPY
u32int* nopPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int nopInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* bxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int bxInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* mulPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int mulInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* mlaPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int mlaInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* swpPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int swpInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* sumullPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int sumullInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* sumlalPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int sumlalInstruction(GCONTXT * context);
/* V7 instructions.  */

#ifdef CONFIG_BLOCK_COPY
u32int* pliPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int pliInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* dbgPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int dbgInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* dmbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int dmbInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* dsbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int dsbInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* isbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int isbInstruction(GCONTXT * context);
/* ARM V6T2 instructions.  */

#ifdef CONFIG_BLOCK_COPY
u32int* bfcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int bfcInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* bfiPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int bfiInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* mlsPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int mlsInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* movwPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int movwInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* movtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int movtInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* rbitPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int rbitInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* usbfxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int usbfxInstruction(GCONTXT * context);
/* ARM V6Z instructions.  */

#ifdef CONFIG_BLOCK_COPY
u32int* smcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int smcInstruction(GCONTXT * context);
/* ARM V6K instructions.  */

#ifdef CONFIG_BLOCK_COPY
u32int* clrexPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int clrexInstruction(GCONTXT * context);
/* ARM V6K NOP hints.  */

#ifdef CONFIG_BLOCK_COPY
u32int* yieldPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int yieldInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* wfePCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int wfeInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* wfiPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int wfiInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* sevPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int sevInstruction(GCONTXT * context);
/* ARM V6 instructions.  */

#ifdef CONFIG_BLOCK_COPY
u32int* cpsiePCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int cpsieInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* cpsidPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int cpsidInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* cpsPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int cpsInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* pkhbtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int pkhbtInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* pkhtbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int pkhtbInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* qadd16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int qadd16Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* qadd8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int qadd8Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* qaddsubxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int qaddsubxInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* qsub16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int qsub16Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* qsub8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int qsub8Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* qsubaddxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int qsubaddxInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* sadd16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int sadd16Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* sadd8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int sadd8Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* saddsubxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int saddsubxInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* shadd16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int shadd16Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* shadd8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int shadd8Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* shaddsubxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int shaddsubxInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* shsub16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int shsub16Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* shsub8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int shsub8Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* shsubaddxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int shsubaddxInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* ssub16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int ssub16Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* ssub8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int ssub8Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* ssubaddxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int ssubaddxInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* uadd16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int uadd16Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* uadd8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int uadd8Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* uaddsubxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int uaddsubxInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* uhadd16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int uhadd16Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* uhadd8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int uhadd8Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* uhaddsubxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int uhaddsubxInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* uhsub16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int uhsub16Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* uhsub8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int uhsub8Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* uhsubaddxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int uhsubaddxInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* uqadd16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int uqadd16Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* uqadd8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int uqadd8Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* uqaddsubxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int uqaddsubxInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* uqsub16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int uqsub16Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* uqsub8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int uqsub8Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* uqsubaddxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int uqsubaddxInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* usub16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int usub16Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* usub8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int usub8Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* usubaddxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int usubaddxInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* revPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int revInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* rev16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int rev16Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* revshPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int revshInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* rfePCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int rfeInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* sxthPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int sxthInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* sxtb16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int sxtb16Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* sxtbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int sxtbInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* uxthPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int uxthInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* uxtb16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int uxtb16Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* uxtbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int uxtbInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* sxtahPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int sxtahInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* sxtab16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int sxtab16Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* sxtabPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int sxtabInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* uxtahPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int uxtahInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* uxtab16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int uxtab16Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* uxtabPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int uxtabInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* selPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int selInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* setendPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int setendInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* smuadPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int smuadInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* smusdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int smusdInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* smladPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int smladInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* smlaldPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int smlaldInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* smlsdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int smlsdInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* smlsldPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int smlsldInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* smmulPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int smmulInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* smmlaPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int smmlaInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* smmlsPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int smmlsInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* srsPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int srsInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* ssatPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int ssatInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* ssat16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int ssat16Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* umaalPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int umaalInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* usad8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int usad8Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* usada8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int usada8Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* usatPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int usatInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* usat16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int usat16Instruction(GCONTXT * context);
/* V5J instruction.  */

#ifdef CONFIG_BLOCK_COPY
u32int* bxjPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int bxjInstruction(GCONTXT * context);
/* V5 Instructions.  */

#ifdef CONFIG_BLOCK_COPY
u32int* bkptPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int bkptInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* blxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int blxInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* clzPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int clzInstruction(GCONTXT * context);
/* V5E "El Segundo" Instructions.  */

#ifdef CONFIG_BLOCK_COPY
u32int* pldPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int pldInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* smlabbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int smlabbInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* smlatbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int smlatbInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* smlabtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int smlabtInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* smlattPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int smlattInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* smlawbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int smlawbInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* smlawtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int smlawtInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* smlalbbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int smlalbbInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* smlaltbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int smlaltbInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* smlalbtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int smlalbtInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* smlalttPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int smlalttInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* smulbbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int smulbbInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* smultbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int smultbInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* smulbtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int smulbtInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* smulttPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int smulttInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* smulwbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int smulwbInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* smulwtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int smulwtInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* qaddPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int qaddInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* qdaddPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int qdaddInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* qsubPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int qsubInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* qdsubPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int qdsubInstruction(GCONTXT * context);
/* ARM Instructions.  */

#ifdef CONFIG_BLOCK_COPY
u32int* msrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int msrInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* mrsPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int mrsInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* bPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int bInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* svcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int svcInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* undefinedPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int undefinedInstruction(GCONTXT * context);
#endif
