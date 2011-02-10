#include "decoder.h"
#include "miscInstructions.h"
#include "coprocInstructions.h"
#include "dataProcessInstr.h"
#include "dataMoveInstr.h"
#include "asm-dis.h"
#include "debug.h"

instructionHandler decodeInstr(u32int instr_word)
{
#include "autoDecoder.inc"
	DIE_NOW(0, "autoDecoder: control fell through - BUG !!!");
}

