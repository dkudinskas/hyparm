#include "common/debug.h"

#include "instructionEmu/asm-dis.h"
#include "instructionEmu/coprocInstructions.h"
#include "instructionEmu/dataMoveInstr.h"
#include "instructionEmu/dataProcessInstr.h"
#include "instructionEmu/miscInstructions.h"


extern GCONTXT * getGuestContext(void);

struct opcode32 arm_coproc_opcodes[] = {
/* Generic coprocessor instructions.  */
{3,  &mcrrInstruction,  0x0c400000, 0x0ff00000, "mcrr%c\t%8-11d, %4-7d, %12-15r, %16-19r, cr%0-3d"},
{3,  &mrrcInstruction,  0x0c500000, 0x0ff00000, "mrrc%c\t%8-11d, %4-7d, %12-15r, %16-19r, cr%0-3d"},
{3,  &cdpInstruction,   0x0e000000, 0x0f000010, "cdp%c\t%8-11d, %20-23d, cr%12-15d, cr%16-19d, cr%0-3d, {%5-7d}"},
{3,  &mrcInstruction,   0x0e100010, 0x0f100010, "MRC"},
{3,  &mcrInstruction,   0x0e000010, 0x0f100010, "MCR"},
{3,  &stcInstruction,   0x0c000000, 0x0e100000, "stc%22'l%c\t%8-11d, cr%12-15d, %A"},
{3,  &ldcInstruction,   0x0c100000, 0x0e100000, "ldc%22'l%c\t%8-11d, cr%12-15d, %A"},
/* V6 coprocessor instructions.  */
{3,  &mrrc2Instruction, 0xfc500000, 0xfff00000, "mrrc2%c\t%8-11d, %4-7d, %12-15r, %16-19r, cr%0-3d"},
{3,  &mcrr2Instruction, 0xfc400000, 0xfff00000, "mcrr2%c\t%8-11d, %4-7d, %12-15r, %16-19r, cr%0-3d"},
/* V5 coprocessor instructions.  */
{3,  &ldc2Instruction,  0xfc100000, 0xfe100000, "ldc2%22'l%c\t%8-11d, cr%12-15d, %A"},
{3,  &stc2Instruction,  0xfc000000, 0xfe100000, "stc2%22'l%c\t%8-11d, cr%12-15d, %A"},
{3,  &cdp2Instruction,  0xfe000000, 0xff000010, "cdp2%c\t%8-11d, %20-23d, cr%12-15d, cr%16-19d, cr%0-3d, {%5-7d}"},
{3,  &mcr2Instruction,  0xfe000010, 0xff100010, "mcr2%c\t%8-11d, %21-23d, %12-15r, cr%16-19d, cr%0-3d, {%5-7d}"},
{3,  &mrc2Instruction,  0xfe100010, 0xff100010, "mrc2%c\t%8-11d, %21-23d, %12-15r, cr%16-19d, cr%0-3d, {%5-7d}"},

{-1, &undefinedInstruction, 0x00000000, 0x00000000, "UNDEFINED_INSTRUCTION"}
};

struct opcode32 arm_opcodes[] = {
/* ARM instructions.  */
// NOP is just a nop...
{0,  &nopInstruction,       0xe1a00000, 0xffffffff, "nop\t\t\t; (mov r0, r0)"},
// branch and exchange - this changes execution mode!!! panic there.
{1,  &bxInstruction,        0x012FFF10, 0x0ffffff0, "BRANCH & EXCHANGE"},
// MUL: Rd = Rm * Rn; Rd != PC. pass through
{0,  &mulInstruction,       0x00000090, 0x0fe000f0, "MUL Rd, Rm, Rn"},
// MLA: Rd = Rm * Rn + Ra; Rd != PC. pass through
{0,  &mlaInstruction,       0x00200090, 0x0fe000f0, "MLA Rd, Rm, Rn, Ra"},
// UNIMPLEMENTED
{1,  &swpInstruction,       0x01000090, 0x0fb00ff0, "swp%22'b%c\t%12-15r, %0-3r, [%16-19r]"},
{0,  &sumullInstruction,    0x00800090, 0x0fa000f0, "SMULL"},
{0,  &sumlalInstruction,    0x00a00090, 0x0fa000f0, "%22?sumlal%20's%c\t%12-15r, %16-19r, %0-3r, %8-11r"},
/* V7 instructions.  */
{1,  &pliInstruction,       0xf450f000, 0xfd70f000, "pli\t%P"},
{1,  &dbgInstruction,       0x0320f0f0, 0x0ffffff0, "dbg%c\t#%0-3d"},
{1,  &dmbInstruction,       0xf57ff050, 0xfffffff0, "dmb\t%U"},
{0,  &dsbInstruction,       0xf57ff040, 0xfffffff0, "dsb\t%U"},
{0,  &isbInstruction,       0xf57ff060, 0xfffffff0, "isb\t%U"},
/* ARM V6T2 instructions.  */
{1,  &bfcInstruction,       0x07c0001f, 0x0fe0007f, "bfc%c\t%12-15r, %E"},
{1,  &bfiInstruction,       0x07c00010, 0x0fe00070, "bfi%c\t%12-15r, %0-3r, %E"},
{1,  &mlsInstruction,       0x00600090, 0x0ff000f0, "mls%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
{1,  &strhtInstruction,     0x006000b0, 0x0f7000f0, "strht%c\t%12-15r, %s"},
{1,  &ldrhtInstruction,     0x00300090, 0x0f300090, "ldr%6's%5?hbt%c\t%12-15r, %s"},
// MOVW - indication of 'wide' - to select ARM encoding. Rd cant be PC, pass through.
{0,  &movwInstruction,      0x03000000, 0x0ff00000, "MOVW Rd, Rn"},
{0,  &movtInstruction,      0x03400000, 0x0ff00000, "MOVT Rd, Rn"},
{1,  &rbitInstruction,      0x06ff0f30, 0x0fff0ff0, "rbit%c\t%12-15r, %0-3r"},
{1,  &usbfxInstruction,     0x07a00050, 0x0fa00070, "%22?usbfx%c\t%12-15r, %0-3r, #%7-11d, #%16-20W"},

/* ARM V6Z instructions.  */
{1,  &smcInstruction,       0x01600070, 0x0ff000f0, "smc%c\t%e"},

/* ARM V6K instructions.  */
{1,  &clrexInstruction,     0xf57ff01f, 0xffffffff, "clrex"},
{1,  &ldrexbInstruction,    0x01d00f9f, 0x0ff00fff, "ldrexb%c\t%12-15r, [%16-19r]"},
{1,  &ldrexdInstruction,    0x01b00f9f, 0x0ff00fff, "ldrexd%c\t%12-15r, [%16-19r]"},
{1,  &ldrexhInstruction,    0x01f00f9f, 0x0ff00fff, "ldrexh%c\t%12-15r, [%16-19r]"},
{1,  &strexbInstruction,    0x01c00f90, 0x0ff00ff0, "strexb%c\t%12-15r, %0-3r, [%16-19r]"},
{1,  &strexdInstruction,    0x01a00f90, 0x0ff00ff0, "strexd%c\t%12-15r, %0-3r, [%16-19r]"},
{1,  &strexhInstruction,    0x01e00f90, 0x0ff00ff0, "strexh%c\t%12-15r, %0-3r, [%16-19r]"},

/* ARM V6K NOP hints.  */
{1,  &yieldInstruction,     0x0320f001, 0x0fffffff, "yield%c"},
{1,  &wfeInstruction,       0x0320f002, 0x0fffffff, "wfe%c"},
{1,  &wfiInstruction,       0x0320f003, 0x0fffffff, "wfi%c"},
{1,  &sevInstruction,       0x0320f004, 0x0fffffff, "sev%c"},
{1,  &nopInstruction,       0x0320f000, 0x0fffff00, "nop"},

/* ARM V6 instructions.  */
{1,  &cpsieInstruction,     0xf1080000, 0xfffffe3f, "cpsie\t%8'a%7'i%6'f"},
{1,  &cpsieInstruction,     0xf10a0000, 0xfffffe20, "cpsie\t%8'a%7'i%6'f,#%0-4d"},
{1,  &cpsidInstruction,     0xf10C0000, 0xfffffe3f, "cpsid\t%8'a%7'i%6'f"},
{1,  &cpsidInstruction,     0xf10e0000, 0xfffffe20, "cpsid\t%8'a%7'i%6'f,#%0-4d"},
{1,  &cpsInstruction,       0xf1000000, 0xfff1fe20, "cps\t#%0-4d"},
{1,  &pkhbtInstruction,     0x06800010, 0x0ff00ff0, "pkhbt%c\t%12-15r, %16-19r, %0-3r"},
{1,  &pkhbtInstruction,     0x06800010, 0x0ff00070, "pkhbt%c\t%12-15r, %16-19r, %0-3r, lsl #%7-11d"},
{1,  &pkhtbInstruction,     0x06800050, 0x0ff00ff0, "pkhtb%c\t%12-15r, %16-19r, %0-3r, asr #32"},
{1,  &pkhtbInstruction,     0x06800050, 0x0ff00070, "pkhtb%c\t%12-15r, %16-19r, %0-3r, asr #%7-11d"},
{1,  &ldrexInstruction,     0x01900f9f, 0x0ff00fff, "ldrex%c\tr%12-15d, [%16-19r]"},
{1,  &qadd16Instruction,    0x06200f10, 0x0ff00ff0, "qadd16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &qadd8Instruction,     0x06200f90, 0x0ff00ff0, "qadd8%c\t%12-15r, %16-19r, %0-3r"},
{1,  &qaddsubxInstruction,  0x06200f30, 0x0ff00ff0, "qaddsubx%c\t%12-15r, %16-19r, %0-3r"},
{1,  &qsub16Instruction,    0x06200f70, 0x0ff00ff0, "qsub16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &qsub8Instruction,     0x06200ff0, 0x0ff00ff0, "qsub8%c\t%12-15r, %16-19r, %0-3r"},
{1,  &qsubaddxInstruction,  0x06200f50, 0x0ff00ff0, "qsubaddx%c\t%12-15r, %16-19r, %0-3r"},
{1,  &sadd16Instruction,    0x06100f10, 0x0ff00ff0, "sadd16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &sadd8Instruction,     0x06100f90, 0x0ff00ff0, "sadd8%c\t%12-15r, %16-19r, %0-3r"},
{1,  &saddsubxInstruction,  0x06100f30, 0x0ff00ff0, "saddsubx%c\t%12-15r, %16-19r, %0-3r"},
{1,  &shadd16Instruction,   0x06300f10, 0x0ff00ff0, "shadd16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &shadd8Instruction,    0x06300f90, 0x0ff00ff0, "shadd8%c\t%12-15r, %16-19r, %0-3r"},
{1,  &shaddsubxInstruction, 0x06300f30, 0x0ff00ff0, "shaddsubx%c\t%12-15r, %16-19r, %0-3r"},
{1,  &shsub16Instruction,   0x06300f70, 0x0ff00ff0, "shsub16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &shsub8Instruction,    0x06300ff0, 0x0ff00ff0, "shsub8%c\t%12-15r, %16-19r, %0-3r"},
{1,  &shsubaddxInstruction, 0x06300f50, 0x0ff00ff0, "shsubaddx%c\t%12-15r, %16-19r, %0-3r"},
{1,  &ssub16Instruction,    0x06100f70, 0x0ff00ff0, "ssub16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &ssub8Instruction,     0x06100ff0, 0x0ff00ff0, "ssub8%c\t%12-15r, %16-19r, %0-3r"},
{1,  &ssubaddxInstruction,  0x06100f50, 0x0ff00ff0, "ssubaddx%c\t%12-15r, %16-19r, %0-3r"},
{1,  &uadd16Instruction,    0x06500f10, 0x0ff00ff0, "uadd16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &uadd8Instruction,     0x06500f90, 0x0ff00ff0, "uadd8%c\t%12-15r, %16-19r, %0-3r"},
{1,  &uaddsubxInstruction,  0x06500f30, 0x0ff00ff0, "uaddsubx%c\t%12-15r, %16-19r, %0-3r"},
{1,  &uhadd16Instruction,   0x06700f10, 0x0ff00ff0, "uhadd16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &uhadd8Instruction,    0x06700f90, 0x0ff00ff0, "uhadd8%c\t%12-15r, %16-19r, %0-3r"},
{1,  &uhaddsubxInstruction, 0x06700f30, 0x0ff00ff0, "uhaddsubx%c\t%12-15r, %16-19r, %0-3r"},
{1,  &uhsub16Instruction,   0x06700f70, 0x0ff00ff0, "uhsub16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &uhsub8Instruction,    0x06700ff0, 0x0ff00ff0, "uhsub8%c\t%12-15r, %16-19r, %0-3r"},
{1,  &uhsubaddxInstruction, 0x06700f50, 0x0ff00ff0, "uhsubaddx%c\t%12-15r, %16-19r, %0-3r"},
{1,  &uqadd16Instruction,   0x06600f10, 0x0ff00ff0, "uqadd16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &uqadd8Instruction,    0x06600f90, 0x0ff00ff0, "uqadd8%c\t%12-15r, %16-19r, %0-3r"},
{1,  &uqaddsubxInstruction, 0x06600f30, 0x0ff00ff0, "uqaddsubx%c\t%12-15r, %16-19r, %0-3r"},
{1,  &uqsub16Instruction,   0x06600f70, 0x0ff00ff0, "uqsub16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &uqsub8Instruction,    0x06600ff0, 0x0ff00ff0, "uqsub8%c\t%12-15r, %16-19r, %0-3r"},
{1,  &uqsubaddxInstruction, 0x06600f50, 0x0ff00ff0, "uqsubaddx%c\t%12-15r, %16-19r, %0-3r"},
{1,  &usub16Instruction,    0x06500f70, 0x0ff00ff0, "usub16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &usub8Instruction,     0x06500ff0, 0x0ff00ff0, "usub8%c\t%12-15r, %16-19r, %0-3r"},
{1,  &usubaddxInstruction,  0x06500f50, 0x0ff00ff0, "usubaddx%c\t%12-15r, %16-19r, %0-3r"},
{1,  &revInstruction,       0x06bf0f30, 0x0fff0ff0, "rev%c\t%12-15r, %0-3r"},
{1,  &rev16Instruction,     0x06bf0fb0, 0x0fff0ff0, "rev16%c\t%12-15r, %0-3r"},
{1,  &revshInstruction,     0x06ff0fb0, 0x0fff0ff0, "revsh%c\t%12-15r, %0-3r"},
{1,  &rfeInstruction,       0xf8100a00, 0xfe50ffff, "rfe%23?id%24?ba\t%16-19r%21'!"},
{1,  &sxthInstruction,      0x06bf0070, 0x0fff0ff0, "sxth%c\t%12-15r, %0-3r"},
{1,  &sxthInstruction,      0x06bf0470, 0x0fff0ff0, "sxth%c\t%12-15r, %0-3r, ror #8"},
{1,  &sxthInstruction,      0x06bf0870, 0x0fff0ff0, "sxth%c\t%12-15r, %0-3r, ror #16"},
{1,  &sxthInstruction,      0x06bf0c70, 0x0fff0ff0, "sxth%c\t%12-15r, %0-3r, ror #24"},
{1,  &sxtb16Instruction,    0x068f0070, 0x0fff0ff0, "sxtb16%c\t%12-15r, %0-3r"},
{1,  &sxtb16Instruction,    0x068f0470, 0x0fff0ff0, "sxtb16%c\t%12-15r, %0-3r, ror #8"},
{1,  &sxtb16Instruction,    0x068f0870, 0x0fff0ff0, "sxtb16%c\t%12-15r, %0-3r, ror #16"},
{1,  &sxtb16Instruction,    0x068f0c70, 0x0fff0ff0, "sxtb16%c\t%12-15r, %0-3r, ror #24"},
{1,  &sxtbInstruction,      0x06af0070, 0x0fff0ff0, "sxtb%c\t%12-15r, %0-3r"},
{1,  &sxtbInstruction,      0x06af0470, 0x0fff0ff0, "sxtb%c\t%12-15r, %0-3r, ror #8"},
{1,  &sxtbInstruction,      0x06af0870, 0x0fff0ff0, "sxtb%c\t%12-15r, %0-3r, ror #16"},
{1,  &sxtbInstruction,      0x06af0c70, 0x0fff0ff0, "sxtb%c\t%12-15r, %0-3r, ror #24"},
{1,  &uxthInstruction,      0x06ff0070, 0x0fff0ff0, "UXTH"},
{1,  &uxthInstruction,      0x06ff0470, 0x0fff0ff0, "UXTH"},
{1,  &uxthInstruction,      0x06ff0870, 0x0fff0ff0, "UXTH"},
{1,  &uxthInstruction,      0x06ff0c70, 0x0fff0ff0, "UXTH"},
{1,  &uxtb16Instruction,    0x06cf0070, 0x0fff0ff0, "uxtb16%c\t%12-15r, %0-3r"},
{1,  &uxtb16Instruction,    0x06cf0470, 0x0fff0ff0, "uxtb16%c\t%12-15r, %0-3r, ror #8"},
{1,  &uxtb16Instruction,    0x06cf0870, 0x0fff0ff0, "uxtb16%c\t%12-15r, %0-3r, ror #16"},
{1,  &uxtb16Instruction,    0x06cf0c70, 0x0fff0ff0, "uxtb16%c\t%12-15r, %0-3r, ror #24"},
// UXTB permitted, if Rd or Rn = 15 then unpredictable.
{0,  &uxtbInstruction,      0x06ef0070, 0x0fff0ff0, "UXTB Rd, Rn"},
{0,  &uxtbInstruction,      0x06ef0470, 0x0fff0ff0, "UXTB Rd, Rn, ROR #8"},
{0,  &uxtbInstruction,      0x06ef0870, 0x0fff0ff0, "UXTB Rd, Rn, ROR #16"},
{0,  &uxtbInstruction,      0x06ef0c70, 0x0fff0ff0, "UXTB Rd, Rn, ROR #24"},
// UNIMPLEMENTED
{1,  &sxtahInstruction,     0x06b00070, 0x0ff00ff0, "sxtah%c\t%12-15r, %16-19r, %0-3r"},
{1,  &sxtahInstruction,     0x06b00470, 0x0ff00ff0, "sxtah%c\t%12-15r, %16-19r, %0-3r, ror #8"},
{1,  &sxtahInstruction,     0x06b00870, 0x0ff00ff0, "sxtah%c\t%12-15r, %16-19r, %0-3r, ror #16"},
{1,  &sxtahInstruction,     0x06b00c70, 0x0ff00ff0, "sxtah%c\t%12-15r, %16-19r, %0-3r, ror #24"},
{1,  &sxtab16Instruction,   0x06800070, 0x0ff00ff0, "sxtab16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &sxtab16Instruction,   0x06800470, 0x0ff00ff0, "sxtab16%c\t%12-15r, %16-19r, %0-3r, ror #8"},
{1,  &sxtab16Instruction,   0x06800870, 0x0ff00ff0, "sxtab16%c\t%12-15r, %16-19r, %0-3r, ror #16"},
{1,  &sxtab16Instruction,   0x06800c70, 0x0ff00ff0, "sxtab16%c\t%12-15r, %16-19r, %0-3r, ror #24"},
{1,  &sxtabInstruction,     0x06a00070, 0x0ff00ff0, "sxtab%c\t%12-15r, %16-19r, %0-3r"},
{1,  &sxtabInstruction,     0x06a00470, 0x0ff00ff0, "sxtab%c\t%12-15r, %16-19r, %0-3r, ror #8"},
{1,  &sxtabInstruction,     0x06a00870, 0x0ff00ff0, "sxtab%c\t%12-15r, %16-19r, %0-3r, ror #16"},
{1,  &sxtabInstruction,     0x06a00c70, 0x0ff00ff0, "sxtab%c\t%12-15r, %16-19r, %0-3r, ror #24"},
{1,  &uxtahInstruction,     0x06f00070, 0x0ff00ff0, "uxtah%c\t%12-15r, %16-19r, %0-3r"},
{1,  &uxtahInstruction,     0x06f00470, 0x0ff00ff0, "uxtah%c\t%12-15r, %16-19r, %0-3r, ror #8"},
{1,  &uxtahInstruction,     0x06f00870, 0x0ff00ff0, "uxtah%c\t%12-15r, %16-19r, %0-3r, ror #16"},
{1,  &uxtahInstruction,     0x06f00c70, 0x0ff00ff0, "uxtah%c\t%12-15r, %16-19r, %0-3r, ror #24"},
{1,  &uxtab16Instruction,   0x06c00070, 0x0ff00ff0, "uxtab16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &uxtab16Instruction,   0x06c00470, 0x0ff00ff0, "uxtab16%c\t%12-15r, %16-19r, %0-3r, ror #8"},
{1,  &uxtab16Instruction,   0x06c00870, 0x0ff00ff0, "uxtab16%c\t%12-15r, %16-19r, %0-3r, ror #16"},
{1,  &uxtab16Instruction,   0x06c00c70, 0x0ff00ff0, "uxtab16%c\t%12-15r, %16-19r, %0-3r, ROR #24"},
{1,  &uxtabInstruction,     0x06e00070, 0x0ff00ff0, "uxtab%c\t%12-15r, %16-19r, %0-3r"},
{1,  &uxtabInstruction,     0x06e00470, 0x0ff00ff0, "uxtab%c\t%12-15r, %16-19r, %0-3r, ror #8"},
{1,  &uxtabInstruction,     0x06e00870, 0x0ff00ff0, "uxtab%c\t%12-15r, %16-19r, %0-3r, ror #16"},
{1,  &uxtabInstruction,     0x06e00c70, 0x0ff00ff0, "uxtab%c\t%12-15r, %16-19r, %0-3r, ror #24"},
{1,  &selInstruction,       0x06800fb0, 0x0ff00ff0, "sel%c\t%12-15r, %16-19r, %0-3r"},
{1,  &setendInstruction,    0xf1010000, 0xfffffc00, "setend\t%9?ble"},
{1,  &smuadInstruction,     0x0700f010, 0x0ff0f0d0, "smuad%5'x%c\t%16-19r, %0-3r, %8-11r"},
{1,  &smusdInstruction,     0x0700f050, 0x0ff0f0d0, "smusd%5'x%c\t%16-19r, %0-3r, %8-11r"},
{1,  &smladInstruction,     0x07000010, 0x0ff000d0, "smlad%5'x%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
{1,  &smlaldInstruction,    0x07400010, 0x0ff000d0, "smlald%5'x%c\t%12-15r, %16-19r, %0-3r, %8-11r"},
{1,  &smlsdInstruction,     0x07000050, 0x0ff000d0, "smlsd%5'x%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
{1,  &smlsldInstruction,    0x07400050, 0x0ff000d0, "smlsld%5'x%c\t%12-15r, %16-19r, %0-3r, %8-11r"},
{1,  &smmulInstruction,     0x0750f010, 0x0ff0f0d0, "smmul%5'r%c\t%16-19r, %0-3r, %8-11r"},
{1,  &smmlaInstruction,     0x07500010, 0x0ff000d0, "smmla%5'r%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
{1,  &smmlsInstruction,     0x075000d0, 0x0ff000d0, "smmls%5'r%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
{1,  &srsInstruction,       0xf84d0500, 0xfe5fffe0, "srs%23?id%24?ba\t%16-19r%21'!, #%0-4d"},
{1,  &ssatInstruction,      0x06a00010, 0x0fe00ff0, "ssat%c\t%12-15r, #%16-20W, %0-3r"},
{1,  &ssatInstruction,      0x06a00010, 0x0fe00070, "ssat%c\t%12-15r, #%16-20W, %0-3r, lsl #%7-11d"},
{1,  &ssatInstruction,      0x06a00050, 0x0fe00070, "ssat%c\t%12-15r, #%16-20W, %0-3r, asr #%7-11d"},
{1,  &ssat16Instruction,    0x06a00f30, 0x0ff00ff0, "ssat16%c\t%12-15r, #%16-19W, %0-3r"},
{1,  &strexInstruction,     0x01800f90, 0x0ff00ff0, "strex%c\t%12-15r, %0-3r, [%16-19r]"},
{1,  &umaalInstruction,     0x00400090, 0x0ff000f0, "umaal%c\t%12-15r, %16-19r, %0-3r, %8-11r"},
{1,  &usad8Instruction,     0x0780f010, 0x0ff0f0f0, "usad8%c\t%16-19r, %0-3r, %8-11r"},
{1,  &usada8Instruction,    0x07800010, 0x0ff000f0, "usada8%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
{1,  &usatInstruction,      0x06e00010, 0x0fe00ff0, "usat%c\t%12-15r, #%16-20d, %0-3r"},
{1,  &usatInstruction,      0x06e00010, 0x0fe00070, "usat%c\t%12-15r, #%16-20d, %0-3r, lsl #%7-11d"},
{1,  &usatInstruction,      0x06e00050, 0x0fe00070, "usat%c\t%12-15r, #%16-20d, %0-3r, asr #%7-11d"},
{1,  &usat16Instruction,    0x06e00f30, 0x0ff00ff0, "usat16%c\t%12-15r, #%16-19d, %0-3r"},

/* V5J instruction.  */
{1,  &bxjInstruction,       0x012fff20, 0x0ffffff0, "bxj%c\t%0-3r"},

/* V5 Instructions.  */
{1,  &bkptInstruction,      0xe1200070, 0xfff000f0, "bkpt\t0x%16-19X%12-15X%8-11X%0-3X"},
{1,  &blxInstruction,       0xfa000000, 0xfe000000, "BLX <label>"},
{1,  &blxInstruction,       0x012fff30, 0x0ffffff0, "blx%c\t%0-3r"},
// CLZ: Count leading zeroes - Rd, Rm != PC, pass through
{0,  &clzInstruction,       0x016f0f10, 0x0fff0ff0, "CLZ Rd, Rm"},

/* V5E "El Segundo" Instructions.  */
{1,  &ldrdInstruction,      0x000000d0, 0x0e1000f0, "ldrd%c\t%12-15r, %s"},
{1,  &strdInstruction,      0x000000f0, 0x0e1000f0, "strd%c\t%12-15r, %s"},
{1,  &pldInstruction,       0xf450f000, 0xfc70f000, "pld\t%a"},
{1,  &smlabbInstruction,    0x01000080, 0x0ff000f0, "smlabb%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
{1,  &smlatbInstruction,    0x010000a0, 0x0ff000f0, "smlatb%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
{1,  &smlabtInstruction,    0x010000c0, 0x0ff000f0, "smlabt%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
{1,  &smlattInstruction,    0x010000e0, 0x0ff000f0, "smlatt%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
{1,  &smlawbInstruction,    0x01200080, 0x0ff000f0, "smlawb%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
{1,  &smlawtInstruction,    0x012000c0, 0x0ff000f0, "smlawt%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
{1,  &smlalbbInstruction,   0x01400080, 0x0ff000f0, "smlalbb%c\t%12-15r, %16-19r, %0-3r, %8-11r"},
{1,  &smlaltbInstruction,   0x014000a0, 0x0ff000f0, "smlaltb%c\t%12-15r, %16-19r, %0-3r, %8-11r"},
{1,  &smlalbtInstruction,   0x014000c0, 0x0ff000f0, "smlalbt%c\t%12-15r, %16-19r, %0-3r, %8-11r"},
{1,  &smlalttInstruction,   0x014000e0, 0x0ff000f0, "smlaltt%c\t%12-15r, %16-19r, %0-3r, %8-11r"},
{1,  &smulbbInstruction,    0x01600080, 0x0ff0f0f0, "smulbb%c\t%16-19r, %0-3r, %8-11r"},
{1,  &smultbInstruction,    0x016000a0, 0x0ff0f0f0, "smultb%c\t%16-19r, %0-3r, %8-11r"},
{1,  &smulbtInstruction,    0x016000c0, 0x0ff0f0f0, "smulbt%c\t%16-19r, %0-3r, %8-11r"},
{1,  &smulttInstruction,    0x016000e0, 0x0ff0f0f0, "smultt%c\t%16-19r, %0-3r, %8-11r"},
{1,  &smulwbInstruction,    0x012000a0, 0x0ff0f0f0, "smulwb%c\t%16-19r, %0-3r, %8-11r"},
{1,  &smulwtInstruction,    0x012000e0, 0x0ff0f0f0, "smulwt%c\t%16-19r, %0-3r, %8-11r"},
{1,  &qaddInstruction,      0x01000050, 0x0ff00ff0, "qadd%c\t%12-15r, %0-3r, %16-19r"},
{1,  &qdaddInstruction,     0x01400050, 0x0ff00ff0, "qdadd%c\t%12-15r, %0-3r, %16-19r"},
{1,  &qsubInstruction,      0x01200050, 0x0ff00ff0, "qsub%c\t%12-15r, %0-3r, %16-19r"},
{1,  &qdsubInstruction,     0x01600050, 0x0ff00ff0, "qdsub%c\t%12-15r, %0-3r, %16-19r"},
/* ARM Instructions.  */
// STR imm12 and reg are pass-through
{0,  &strInstruction,       0x04000000, 0x0e100000, "STR Rt, [Rn, +-imm12]"},
{0,  &strInstruction,       0x06000000, 0x0e100ff0, "STR Rt, [Rn], +-Rm"},
{0,  &strInstruction,       0x04000000, 0x0c100010, "STR any? dont get this."},
{0,  &strbInstruction,      0x04400000, 0x0e500000, "STRB Rt, [Rn, +-imm12]"},
{0,  &strbInstruction,      0x06400000, 0x0e500010, "STRB Rt, [Rn], +-Rm"},
{0,  &strhInstruction,      0x004000b0, 0x0e5000f0, "STRH Rt, [Rn, +-imm8]"},
{0,  &strhInstruction,      0x000000b0, 0x0e500ff0, "STRH Rt, [Rn], +-Rm"},
// LDRH cant load halfword to PC, passthrough
{0,  &ldrhInstruction,      0x00500090, 0x0e500090, "LDRH Rt, [Rn, +-imm8]"},
{0,  &ldrhInstruction,      0x00100090, 0x0e500f90, "LDRH Rt, [Rn], +-Rm"},
// AND: Rd = PC end block, others are fine
{1,  &andInstruction,       0x0000f000, 0x0de0f000, "AND PC, Rn/#imm"},
{0,  &andInstruction,       0x02000000, 0x0fe00000, "AND Rd, Rn, #imm"},
{0,  &andInstruction,       0x00000000, 0x0fe00010, "AND Rd, Rn, Rm, #shamt"},
{0,  &andInstruction,       0x00000010, 0x0fe00090, "AND Rd, Rn, Rm, Rshamt"},
// EOR: Rd = PC end block, others are fine
{1,  &eorInstruction,       0x0020f000, 0x0de0f000, "EOR PC, Rn, Rm/#imm"},
{0,  &eorInstruction,       0x02200000, 0x0fe00000, "EOR Rd, Rn, #imm"},
{0,  &eorInstruction,       0x00200000, 0x0fe00010, "EOR Rd, Rn, Rm, #shamt"},
{0,  &eorInstruction,       0x00200010, 0x0fe00090, "EOR Rd, Rn, Rm, Rshamt"},
// SUB: Rd = PC end block, others are fine
{1,  &subInstruction,       0x0040f000, 0x0de0f000, "SUB PC, Rn, Rm/imm"},
{0,  &subInstruction,       0x02400000, 0x0fe00000, "SUB Rd, Rn, #imm"},
{0,  &subInstruction,       0x00400000, 0x0fe00010, "SUB Rd, Rn, Rm, #shamt"},
{0,  &subInstruction,       0x00400010, 0x0fe00090, "SUB Rd, Rn, Rm, Rshamt"},
// RSB: Rd = PC end block, others are fine
{1,  &rsbInstruction,       0x0060f000, 0x0de0f000, "RSB PC, Rn, Rm/imm"},
{0,  &rsbInstruction,       0x02600000, 0x0fe00000, "RSB Rd, Rn, #imm"},
{0,  &rsbInstruction,       0x00600000, 0x0fe00010, "RSB Rd, Rn, Rm, #shamt"},
{0,  &rsbInstruction,       0x00600010, 0x0fe00090, "RSB Rd, Rn, Rm, Rshamt"},
// ADD: Rd = PC end block, others are fine
{1,  &addInstruction,       0x0080f000, 0x0de0f000, "ADD PC, Rn/#imm"},
{0,  &addInstruction,       0x02800000, 0x0fe00000, "ADD Rd, Rn, #imm"},
{0,  &addInstruction,       0x00800000, 0x0fe00010, "ADD Rd, Rn, Rm, #shamt"},
{0,  &addInstruction,       0x00800010, 0x0fe00090, "ADD Rd, Rn, Rm, Rshamt"},
// ADC: Rd = PC end block, others are fine
{1,  &adcInstruction,       0x00a0f000, 0x0de0f000, "ADC PC, Rn/#imm"},
{0,  &adcInstruction,       0x02a00000, 0x0fe00000, "ADC Rd, Rn, #imm"},
{0,  &adcInstruction,       0x00a00000, 0x0fe00010, "ADC Rd, Rn, Rm, #shamt"},
{0,  &adcInstruction,       0x00a00010, 0x0fe00090, "ADC Rd, Rn, Rm, Rshamt"},
// SBC: Rd = PC end block, others are fine
{1,  &sbcInstruction,       0x00c0f000, 0x0de0f000, "SBC PC, Rn/#imm"},
{0,  &sbcInstruction,       0x02c00000, 0x0fe00000, "SBC Rd, Rn, #imm"},
{0,  &sbcInstruction,       0x00c00000, 0x0fe00010, "SBC Rd, Rn, Rm, #shamt"},
{0,  &sbcInstruction,       0x00c00010, 0x0fe00090, "SBC Rd, Rn, Rm, Rshamt"},
// RSC: Rd = PC end block, others are fine
{1,  &rscInstruction,       0x02e00000, 0x0fe00000, "RSC PC, Rn/#imm"},
{0,  &rscInstruction,       0x02e00000, 0x0fe00000, "RSC Rd, Rn, #imm"},
{0,  &rscInstruction,       0x00e00000, 0x0fe00010, "RSC Rd, Rn, Rm, #shamt"},
{0,  &rscInstruction,       0x00e00010, 0x0fe00090, "RSC Rd, Rn, Rm, Rshamt"},
// MSR/MRS: always hypercall! we must hide the real state from guest.
{2,  &msrInstruction,       0x0120f000, 0x0db0f000, "MSR, s/cpsr, Rn"},
{2,  &mrsInstruction,       0x010f0000, 0x0fbf0fff, "MRS, Rn, s/cpsr"},
// TST instructions are all fine
{0,  &tstInstruction,       0x03000000, 0x0fe00000, "TST Rn, #imm"},
{0,  &tstInstruction,       0x01000000, 0x0fe00010, "TST Rn, Rm, #shamt"},
{0,  &tstInstruction,       0x01000010, 0x0fe00090, "TST Rn, Rm, Rshift"},
// TEQ instructions are all fine
{0,  &teqInstruction,       0x03200000, 0x0fe00000, "TEQ Rn, #imm"},
{0,  &teqInstruction,       0x01200000, 0x0fe00010, "TEQ Rn, Rm, #shamt"},
{0,  &teqInstruction,       0x01200010, 0x0fe00090, "TEQ Rn, Rm, Rshift"},
// CMP instructions are all fine
{0,  &cmpInstruction,       0x03400000, 0x0fe00000, "CMP Rn, #imm"},
{0,  &cmpInstruction,       0x01400000, 0x0fe00010, "CMP Rn, Rm, #shamt"},
{0,  &cmpInstruction,       0x01400010, 0x0fe00090, "CMP Rn, Rm, Rshamt"},
// CMN instructions are all fine
{0,  &cmnInstruction,       0x03600000, 0x0fe00000, "CMN Rn, #imm"},
{0,  &cmnInstruction,       0x01600000, 0x0fe00010, "CMN Rn, Rm, #shamto"},
{0,  &cmnInstruction,       0x01600010, 0x0fe00090, "CMN Rn, #immo"},
// ORR: Rd = PC end block, other are fine
{1,  &orrInstruction,       0x0180f000, 0x0de0f000, "ORR PC, Rn/#imm"},
{0,  &orrInstruction,       0x03800000, 0x0fe00000, "ORR Rd, Rn, #imm"},
{0,  &orrInstruction,       0x01800000, 0x0fe00010, "ORR Rd, Rn, Rm, #shamt"},
{0,  &orrInstruction,       0x01800010, 0x0fe00090, "ORR Rd, Rn, Rm, Rshamt"},
// MOV with Rd = PC end block. MOV <shifted reg> is a pseudo instr..
{1,  &movInstruction,       0x03a0f000, 0x0feff000, "MOV PC, #imm"},
{1,  &movInstruction,       0x01a0f000, 0x0deffff0, "MOV PC, Rm"},
{0,  &movInstruction,       0x03a00000, 0x0fef0000, "MOV Rn, #imm"},
{0,  &movInstruction,       0x01a00000, 0x0def0ff0, "MOV Rn, Rm"},
// LSL: Rd = PC and shamt = #imm, then end block. Otherwise fine
{1,  &lslInstruction,       0x01a0f000, 0x0deff060, "LSL PC, Rm, #imm"},
{0,  &lslInstruction,       0x01a00000, 0x0def0060, "LSL Rd, Rm, Rshamt/#imm"},
// LSR: Rd = PC and shamt = #imm, then end block. Otherwise fine
{1,  &lsrInstruction,       0x01a0f020, 0x0deff060, "LSR PC, Rm, #imm"},
{0,  &lsrInstruction,       0x01a00020, 0x0def0060, "LSR Rd, Rm, Rshamt/#imm"},
// ASR: Rd = PC and shamt = #imm, then end block. Otherwise fine
{1,  &asrInstruction,       0x01a0f040, 0x0deff060, "ASR PC, Rm, #imm"},
{0,  &asrInstruction,       0x01a00040, 0x0def0060, "ASR Rd, Rm, Rshamt/#imm"},
// UNIMPLEMENTED
{1,  &rrxInstruction,       0x01a00060, 0x0def0ff0, "RRX"},
{1,  &rorInstruction,       0x01a00060, 0x0def0060, "ROR"},
// BIC with Rd = PC end block, other are fine.
{1,  &bicInstruction,       0x01c0f000, 0x0de0f000, "BIC PC, Rn/#imm"},
{0,  &bicInstruction,       0x03c00000, 0x0fe00000, "BIC Rd, Rn, #imm"},
{0,  &bicInstruction,       0x01c00000, 0x0fe00010, "BIC Rd, Rn, Rm, #shamt"},
{0,  &bicInstruction,       0x01c00010, 0x0fe00090, "BIC Rd, Rn, Rm, Rshamt"},
// MVN with Rd = PC end block, other are fine.
{1,  &mvnInstruction,       0x01e0f000, 0x0de0f000, "MVN PC, Rm/#imm"},
{0,  &mvnInstruction,       0x03e00000, 0x0fe00000, "MVN Rd, #imm"},
{0,  &mvnInstruction,       0x01e00000, 0x0fe00010, "MVN Rd, Rm, #shamt"},
{0,  &mvnInstruction,       0x01e00010, 0x0fe00090, "MVN Rd, Rm, Rshamt"},
// UNDEFINED INSTRUCTION SPACE
{-1,  &undefinedInstruction, 0x06000010, 0x0e000010, UNDEFINED_INSTRUCTION},
// UNIMPLEMENTED
{1,  &popLdrInstruction,       0x049d0004, 0x0fff0fff, "POP"},
// LDR traps if dest = PC, otherwise pass through
{1,  &ldrInstruction,       0x0410f000, 0x0c10f000, "LDR PC, Rn/#imm12"},
{0,  &ldrInstruction,       0x04100000, 0x0c100000, "LDR Rd, Rn/#imm12"},
// STM traps if ^ postfix, otherwise pass through
{1,  &stmInstruction,       0x08400000, 0x0e500000, "STM {regList}^"},
{0,  &stmInstruction,       0x08800000, 0x0ff00000, "STMIA {regList}"},
{0,  &stmInstruction,       0x08000000, 0x0e100000, "STM {regList}"},
// POP LDM syntax, only care if PC in reglist
{1,  &popLdmInstruction,       0x08bd8000, 0x0fff8000, "LDM SP, {...r15}"},
{0,  &popLdmInstruction,       0x08bd0000, 0x0fff0000, "LDM SP, {reglist}"},
// LDM traps if ^ postfix and/or PC is in register list, otherwise pass through
{1,  &ldmInstruction,       0x08500000, 0x0e500000, "LDM Rn, {regList}^"},
{1,  &ldmInstruction,       0x08108000, 0x0e108000, "LDM Rn, {..r15}"},
{0,  &ldmInstruction,       0x08900000, 0x0f900000, "LDMIA Rn, {regList}"},
{0,  &ldmInstruction,       0x08100000, 0x0e100000, "LDM Rn, {regList}"},
// B/BL: always hypercall! obviously.
{1,  &bInstruction,         0x0a000000, 0x0e000000, "BRANCH"},
// damn. if we hit this then i have to do some serious thinking :)
{1,  &svcInstruction,       0x0f000000, 0x0f000000, "SWI"},

{-1, &undefinedInstruction, 0x00000000, 0x00000000, "UNDEFINED_INSTRUCTION"}
};


#ifdef CONFIG_THUMB2

struct opcode32 thumb32_opcodes[] = {

{0, &movInstruction, 0xF04F0000, 0xFBEF1000, "MOVW<c> <Rd>, #<imm12>"},
// this is for T3 encoding
{0, &movInstruction, 0xF2400000, 0xFBF00000, "MOVW<c> <Rd>, #<imm16>"},
{0, &movInstruction, 0xF04F0000, 0xFBEF8000, "MOV{S}/W <Rd>, #<immt12>"},
{0, &movInstruction, 0xF2400000, 0xFBF08000, "MOVW <Rd>, #<imm16>"},
{0, &movtInstruction, 0xF2C00000, 0xFBF08000, "MOVT<c> <Rd>, #<imm16>"},
{0, &orrInstruction, 0xEA400000, 0xFFE08000, "ORR{S} <Rd>, <Rn>{,<shift>}"},
{0, &orrInstruction, 0xF0400000, 0xFBE08000, "ORR{S} <Rd>, <Rn>,#<imm12>"},
{0, &andInstruction, 0xEA000000, 0xFFF00000, "AND{S}.W <Rd>. <Rn>. <Rm>"},
//trap for RD=15
{1, &andInstruction, 0xF0000F00, 0xFBE08F00, "AND{S}<c> PC, <Rm>, #<imm12>"},
{0, &andInstruction, 0xF0000000, 0xFBE08000, "AND{S}<c> <Rd>, <Rm>, #<imm12>"},
{0, &addInstruction, 0xF1000000, 0xFBE08000, "ADD{S}.W <Rd>, <Rn>, #<imm8>"},
{0, &addInstruction, 0xEB000000, 0xFFE08000, "ADD{S}.W <Rd>, <Rn>, <Rm>{, <shift>}"},
//ADD -> RD=PC -> CMN page 306
{1, addInstruction, 0xF1000F00, 0xFBE08F00, "ADD{S}.W PC, <Rn>, #<imm8>"},
// RN=SP -> unimplemented. Should be OK
{0, &addInstruction, 0xF10D8000, 0xFBEF8000, "ADD{S}.W <Rd>, SP, #<imm8>"},
// Encoding T4
{0, &addInstruction, 0xF2000000, 0xF2008000, "ADDW <Rd>, <Rn>, #<imm12>"},
// RC=PC
{1, &addInstruction, 0xF20F0000, 0xF20F8000, "ADDW PC, <Rn>, #<imm12>"},
// RN=SP -> should be ok
{0, &addInstruction, 0xF20D0000, 0xF20F8000, "ADDW <Rd>, SP, #<imm8>"},
{0, &bicInstruction, 0xF0200000, 0xFBE08000, "BIC{S} <Rd>, <Rn>, #<imm12>"},
{0, &rsbInstruction, 0xF1C00000, 0xFBE08000, "RSB <Rd>, <Rn>, #<imm12>"},
{0, &rsbInstruction, 0xEBC00000, 0xFFE08000, "RSB <Rd>, <Rn>, <Rm>{,<shift>}"},
{0, &subInstruction, 0xF1A00000, 0xFBE08000, "SUB{S}.W <Rd>, <Rn>, #<imm12>"},
{0, &subInstruction, 0xF2A00000, 0xFBE08000, "SUB{S}W <Rd>, <Rn>, #<imm12>"},
{0, &subInstruction, 0xEBA00000, 0xFFE08000, "SUB{S}.W <Rd>, <Rn>, <Rm>{,<shitft>}"},
// RN = SP -> should be ok
{0, &subInstruction, 0xF2AB0000, 0xFBEF8000, "SUBW <Rd>, SP, #<imm12>"},
{0, &subInstruction, 0xEBAB0000, 0xFFEF8000, "SUB{S} <Rd>, SP, <Rm>{,<shift>}"},
{0, &mvnInstruction, 0xEA6F0000, 0xFFEF8000, "MVN<c> <Rd>, <Rm>{,<shift>}"},
{0, &mvnInstruction, 0xE0AF0000, 0xFBEF8000, "MVN<c> <Rd>, #<imm12>"},

{0, &strbInstruction, 0xF8800000, 0xFFF00000, "STRB Rt, [Rn, #<imm12>]"},
{0, &strbInstruction, 0xF8000800, 0xFFF00800, "STRB Rt, [Rn, +-#<imm8>]"},

{0, &strhInstruction, 0xF8A00000, 0xFFF00000, "STRH.W <Rt> [<Rn>, #<imm12>}]"},
{0, &strhInstruction, 0xF8200000, 0xFFF00000, "STRH.W <Rt> [<Rn>, #<imm8>}]!"},

{1, &ldrbInstruction, 0xF81FF000, 0xFEFFFFC0, "LDRB<c> PC ,#<label>"},
{0, &ldrbInstruction, 0xF8900000, 0xFFF00000, "LDRB<c> Rt, [Rn{,#<imm12>}]"},
{0, &ldrbInstruction, 0xF8100900, 0xFFF00900, "LDRB<c> Rt, [Rn,{#<imm12>}]"},
{0, &ldrbInstruction, 0xF8100C00, 0xFFF00E00, "LDRB<c> Rt, [Rn,{#<imm12>}]"},

{0, &ldrhInstruction, 0xF8B00000, 0xFFF00000, "LDRH.W <Rt>, [<Rn>{. #<imm32>}]"},
{0, &ldrhInstruction, 0xF83F0000, 0xFEFF0000, "LDRH <Rt>, <label>"},
{0, &ldrhInstruction, 0xF8300000, 0xFFF00FE0, "LDRH, <Rt>, [<Rn>{,LSL #<imm2>}]"},
{0, &ldrhInstruction, 0xF9B00000, 0xFFF00000, "LDRSH<c> <Rt>, [<Rn>, #<imm12>]"},
{0, &ldrhInstruction, 0xF9300800, 0xFFFF0800, "LDRSH<c> <Rt>, [<Rn>, #<Rn>, +/-#imm8]"}, // Page 454, A8-168
{0, &ldrhInstruction, 0xF9300FC0, 0xFFF00FC0, "LDRSH<c> <Rt>, [<Rn>, <Rm>]"},

{0, &mulInstruction, 0xFB00F000, 0xFFF0F0F0, "MULW <Rd>, <Rn>, <Rm>"},
{0, &smullInstruction, 0xFB800000, 0xFFE000F0, "SMULL <RdLo>, <RdHi>, <Rn>, <Rm>"},

{1, &bInstruction, 0xF0008000, 0xF800D000, "B <imm17>"},
{1, &bInstruction, 0xF0009000, 0xF800D000, "B <imm21>"},
{1, &bInstruction, 0xF000D000, 0xF800D000, "BL, #<imm21>"},
{1, &blxInstruction, 0xF000C000, 0xF800D000, "BLX, #<imm21>"},

{0, &ldrdInstruction, 0xE8500000, 0xFE500000, "LDRD <Rt>, <Rt2>, [<Rn>,{,#+/-<imm>}]"},
{0, &ldrdInstruction, 0xE85F0000, 0xFE7F0000, "LDRD <Rt>, <Rt2>, <label>"},
{0, &strdInstruction, 0xE8400000, 0xFE500000, "STRD <Rt>, <Rt2>, [<Rn>,{,#+/-<imm>}]"}
};

struct opcode32 thumb16_opcodes[] = {
{0, &addInstruction, 0xA800, 0xF800, "ADD <Rd>, SP, #<imm>"},
{0, &addInstruction, 0xB000, 0xFF80, "ADD SP, SP, #<imm>"},

// ADR instruction is add or sub but in any case it does not trap
{0, &addInstruction, 0xA000,0xF800, "ADR <Rd>, <label>"},

{1, &svcInstruction, 0xDF00, 0xFF00, "SVC call"},
{1, &bInstruction, 0xD000, 0xF000, "B<c> <label>"},

{0, &ldrInstruction, 0x6800, 0xF800, "LDR<c> <Rt>, [<Rn>{,#<imm5>}]"},
{0, &ldrInstruction, 0x9800, 0xF800, "LDR<c> <Rt>, [SP{,#<imm8>}]"},
{0, &ldrInstruction, 0x4800, 0xF800, "LDR<c> <Rt>, <label>"},
{0, &ldrInstruction, 0x5800, 0xFE00, "LDR<c> <Rt>, [<Rn>,<Rm>]"},

{0, &mvnInstruction, 0x43C0, 0xFFC0, "MVN <Rd>, <Rm>"},
{0, &cmpInstruction, 0x4280, 0xFFC0, "CMP <Rn>, <Rm>"},
{0, &cmpInstruction, 0x4500, 0xFF00, "CMP <Rn>, <Rm>"},

{1, &bxInstruction, 0x4700, 0xFF80, "BX<c> <Rm>"},
{1, &blxInstruction, 0x4780, 0xFF80, "BLX<c> <Rm>"},
{1, &movInstruction, 0x4607, 0xFF07, "MOV PC, Rm"},
{0, &movInstruction, 0x4600, 0xFF00,  "MOV Rd, Rm"},
{0, &addInstruction, 0x1800, 0xFE00, "ADD <Rd>, <Rn>, <Rm>"},
{0, &addInstruction, 0x4400, 0xFF00, "ADD <Rdn>, <Rm>"},

{0, &stmInstruction, 0xB400, 0xFE00, "PUSH {reglist}"},
{0, &subInstruction, 0xB080, 0xFF80, "SUB SP,SP,<imm7"},
{0, &uxtbInstruction, 0xB2C0, 0xFFC0, "UXTB<c> <Rd>, <Rm>"},
{0, &uxthInstruction, 0xB280, 0xFFC0, "UXTH<C> <Rd>, <Rm>"},
{0, &addInstruction, 0xA800, 0xF800, "ADD <Rd>, SP, #<imm8>"},
{0, &addInstruction, 0xB000, 0xFF80, "ADD SP, SP, #<imm>"},
// trap if PC is on POP reglist
{1, &popLdmInstruction, 0xBD00, 0xFF00, "POP <reglist+PC>"},
{0, &popLdmInstruction, 0xBC00, 0xFE00, "POP <reglist>"},
{0, &nopInstruction, 0xBF00,0xFFFF, "NOP"},
// I think that the If-Then-Else Instruction does not need to trap -.-
{0, &itInstruction, 0xBF00,0xFF00, "IT"},

{0, &strInstruction, 0x6000, 0xF800, "STR Rd, [<Rn>, {#<imm5>}]"},
{0, &strInstruction, 0x9000, 0xF800, "STR Rd, [SP,#<imm8]"},
{0, &ldrInstruction, 0x6800, 0xF800, "LDR Rd, [<Rn> {,#<imm5>}]"},
{0, &ldrInstruction, 0x9800, 0xF800, "LDR Rd, [SP {,#<imm8>}]"},
{0, &ldrInstruction, 0x7800, 0x7800, "LDRB Rd, #<imm5>"},
{0, &ldrhInstruction, 0x8800, 0xF800, "LDRH <Rt>, [<Rn>{, #<imm5>}]"},
{0, &strbInstruction, 0x7000, 0xF800, "STRB <Rt>, [<Rn>, #<imm5>]"},
{0, &strbInstruction, 0xA400, 0xFE00, "STRB <Rt>, [<Rn>, <Rm>]"},
{0, &strhInstruction, 0x8000, 0xF800, "STRH <Rt>,[<Rn>,{,#<imm>}]"},
{0, &strhInstruction, 0x5200, 0xFE00, "STRH <Rt>, [<Rn>,{,<Rm>}"},

{1, &bInstruction, 0xE000, 0xE000, "B<c> #<imm8>"},
// According to Thumb-2 spec, R15 cannot be used in arithmetic
// instructions. So dont trap
{0, &addInstruction, 0x1800, 0xFF00, "ADD Rd, Rn, Rm"},
// page 310. Rd can be R15
{1, &addInstruction, 0x4487, 0x4487, "ADD<c> R15, Rm"},
// Otherwise don't trap
{0, &addInstruction, 0x4400, 0x4400, "ADD<c> Rd, Rm"},
// CMP is fine
{0, &cmpInstruction, 0x2800, 0xF800, "CMP<c> Rn, #<imm8>"},
{0, &subInstruction, 0x1E00, 0xFE00, "SUB{S} <Rd>, <Rn>,#<imm3>"},
{0, &subInstruction, 0x3800, 0xF800, "SUB{S} <Rdn>, #<imm8>"},
{0, &subInstruction, 0x1A00, 0xFE00, "SUB{S} <Rd>, <Rn> <Rm>"},
{0, &movInstruction, 0x2000, 0xF800, "MOV<c> <Rd>, #<imm8>"}

};

#endif


struct opcode32 * decodeInstruction(u32int instr)
{
  int index = 0;
  struct opcode32 * retInstr = 0;

  /* LOOP through all ARM instructions looking for match */
  /*
   * FIXME: check what assembly code is generated for both cases below
   */
#ifdef CONFIG_THUMB2
  while (index < INDEX_OF(arm_opcodes))
#else
  while (TRUE)
#endif
  {
    if ( (instr & arm_opcodes[index].mask) == arm_opcodes[index].value)
    {
      retInstr = &arm_opcodes[index];
      break;
    }
    else
    {
      index = index + 1;
    }
  }

  if (retInstr->replaceCode != -1)
  {
    // got a match in ARM instructions!
    return retInstr;
  }
  /* else - no match in ARM instructions, check coproc instructions. */
  index = 0;
  /*
   * FIXME: check what assembly code is generated for both cases below
   */
#ifdef CONFIG_THUMB2
  while (index < INDEX_OF(arm_coproc_opcodes))
#else
  while (TRUE)
#endif
  {
    if ( (instr & arm_coproc_opcodes[index].mask) == arm_coproc_opcodes[index].value)
    {
      retInstr = &arm_coproc_opcodes[index];
      break;
    }
    else
    {
      index = index + 1;
    }
  }

  if (retInstr->replaceCode == -1)
  {
    // could not match the instruction in coproc as well! lunch time...
    printf("decodeInstruction: undefined instruction found: %08x\n", (u32int)instr);
    DIE_NOW(0, "DIE");
  }

  // FIXME: if the compiler still complains there must be a path that returns to the caller
  //to make compiler happy
  return retInstr;
}


#ifdef CONFIG_THUMB2

void dumpInstrString(GCONTXT *gc, u32int instr)
{
  u32int index = 0;
  /* LOOP through all ARM instructions looking for match */
  if(gc->CPSR & T_BIT)
  {
    switch (instr & THUMB32 << 16)
    {
      // instr is 32-bit so THUMB32 defs need to be extended to 32-bit
      case THUMB32_1 << 16:
      case THUMB32_2 << 16:
      case THUMB32_3 << 16:
      {
        while(index < INDEX_OF(thumb32_opcodes))
        {
          if ((instr & thumb32_opcodes[index].mask) == thumb32_opcodes[index].value)
          {
            printf("%s\n", thumb32_opcodes[index].instructionString);
            return;
          }
          ++index;
        }
        printf("decoder: dumpInstr(Thumb32): undefined instruction found: %08x\n", (u32int)instr);
        DIE_NOW(0, "DIE");
      }
      default:
      {
        while(index < INDEX_OF(thumb16_opcodes))
        {
          if ((instr & thumb16_opcodes[index].mask) == thumb16_opcodes[index].value)
          {
            printf("%s\n", thumb16_opcodes[index].instructionString);
            return;
          }
          ++index;
        }
        printf("decoder: dumpInstr(Thumb16): undefined instruction found: %08x\n", (u32int)instr);
        DIE_NOW(0, "DIE");
      }
    }
  }
  // ARM
  else
  {
    while (index < INDEX_OF(arm_opcodes))
    {
      if ( (instr & arm_opcodes[index].mask) == arm_opcodes[index].value)
      {
        printf("%s\n", arm_opcodes[index].instructionString);
        return;
      }
      ++index;
    }
    /* no match in ARM instructions, check coproc instructions. */
    index = 0;
    while (index<INDEX_OF(arm_coproc_opcodes))
    {
      if ( (instr & arm_coproc_opcodes[index].mask) == arm_coproc_opcodes[index].value)
      {
        printf("%s\n", arm_coproc_opcodes[index].instructionString);
        return;
      }
      ++index;
    }
    // could not match the instruction in coproc as well! lunch time...
    printf("decoder: dumpInstr: undefined instruction found: %08x\n", (u32int)instr);
    DIE_NOW(0, "DIE");
  }
}

#else

void dumpInstrString(u32int instr)
{
  u32int index = 0;
  /* LOOP through all ARM instructions looking for match */
  while (TRUE)
  {
    if ( (instr & arm_opcodes[index].mask) == arm_opcodes[index].value)
    {
      if (arm_opcodes[index].mask == 0x00000000)
      {
        break;
      }
      printf("%s\n", arm_opcodes[index].instructionString);
      return;
    }
    ++index;
  }

  /* no match in ARM instructions, check coproc instructions. */
  index = 0;
  while (TRUE)
  {
    if ( (instr & arm_coproc_opcodes[index].mask) == arm_coproc_opcodes[index].value)
    {
      printf("%s\n", arm_coproc_opcodes[index].instructionString);
      return;
    }
    ++index;
  }

  // could not match the instruction in coproc as well! lunch time...
  printf("decoder: dumpInstr: undefined instruction found: %08x\n", (u32int)instr);
  DIE_NOW(0, "DIE");
}

#endif /* CONFIG_THUMB2 */

