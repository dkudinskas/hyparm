#include "common/stddef.h"

#include "cpuArch/constants.h"

#include "instructionEmu/interpreter/blockCopy.h"
#include "instructionEmu/interpreter/internals.h"


void armBackupRegisterToSpill(TranslationCache *tc, ARMTranslationInfo *block, u32int conditionCode, u32int reg)
{
  ASSERT(conditionCode <= CC_AL, "invalid condition code");
  ASSERT(reg < GPR_SP, "invalid temporary register");

  block->code = updateCodeCachePointer(tc, block->code);
  u32int *pc = block->code + 2;
  bool add = tc->spillPage >= pc;
  u32int offset = (add ? (tc->spillPage - pc) : (pc - tc->spillPage)) * sizeof(u32int);

  ASSERT(offset <= 4095, "spill location unreachable from this location in C$");

  // assemble STR(offset, index=1,wback=0) to available spill location. currently always in spill page
  //|COND|010P|U0W0| Rn | Rt |    imm12   |
  *block->code = (conditionCode << 28) | (0b0101 << 24) | ((u32int)add << 23) | (GPR_PC << 16) | (reg << 12) | offset;
  block->code++;
  block->pcRemapBitmapShift += PC_REMAP_BIT_COUNT;
}

void armRestoreRegisterFromSpill(TranslationCache *tc, ARMTranslationInfo *block, u32int conditionCode, u32int reg)
{
  ASSERT(conditionCode <= CC_AL, "invalid condition code");
  ASSERT(reg < GPR_SP, "invalid temporary register");

  block->code = updateCodeCachePointer(tc, block->code);
  u32int *pc = block->code + 2;
  bool add = tc->spillPage >= pc;
  u32int offset = (add ? (tc->spillPage - pc) : (pc - tc->spillPage)) * sizeof(u32int);

  ASSERT(offset <= 4095, "spill location unreachable from this location in C$");

  // assemble LDR(offset, index=1,wback=0) to available spill location. currently always in spill page
  //|COND|010P|U0W1| Rn | Rt |    imm12   |
  *block->code = (conditionCode << 28) | (0b0101 << 24) | ((u32int)add << 23) | (1 << 20) | (GPR_PC << 16) | (reg << 12) | offset;
  block->code++;
  block->pcRemapBitmapShift += PC_REMAP_BIT_COUNT;
}

void armWritePCToRegister(TranslationCache *tc, ARMTranslationInfo *block, u32int conditionCode, u32int reg, u32int pc)
{
  ASSERT(conditionCode <= CC_AL, "invalid condition code");
  ASSERT(reg < 13, "invalid temporary register");

  pc += 8;

  // assemble MOVW
  //MOVW -> ARM ARM A8.6.96 p506
  //|COND|0011|0000|imm4| Rd |    imm12   |
  block->code = updateCodeCachePointer(tc, block->code);
  *block->code = (conditionCode << 28) | (0b00110000 << 20) | ((pc & 0xF000) << 4) | (reg << 12) | (pc & 0x0FFF);
  block->code++;
  block->pcRemapBitmapShift += PC_REMAP_BIT_COUNT;

  pc >>= 16;

  // assemble MOVT
  //MOVT -> ARM ARM A8.6.99 p512
  //|COND|0011|0100|imm4| Rd |    imm12   |
  block->code = updateCodeCachePointer(tc, block->code);
  *block->code = (conditionCode << 28) | (0b00110100 << 20) | ((pc & 0xF000) << 4) | (reg << 12) | (pc & 0x0FFF);
  block->code++;
  block->pcRemapBitmapShift += PC_REMAP_BIT_COUNT;
}
