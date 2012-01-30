#include "common/debug.h"

#include "instructionEmu/asm-dis.h"
#include "instructionEmu/coprocInstructions.h"
#include "instructionEmu/dataMoveInstr.h"
#include "instructionEmu/dataProcessInstr.h"
#include "instructionEmu/decoder.h"
#include "instructionEmu/miscInstructions.h"


instructionHandler decodeInstr(GCONTXT *context, u32int instr_word)
{
#include "autoDecoder.inc"
  DIE_NOW(context, "autoDecoder: control fell through - BUG !!!");
}
