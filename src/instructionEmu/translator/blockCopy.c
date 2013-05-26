#include "common/stddef.h"
#include "common/debug.h"

#include "cpuArch/constants.h"

#include "common/linker.h"

#include "instructionEmu/translator/blockCopy.h"
#include "instructionEmu/decoder/arm/structs.h"


void armSpillRegister(TranslationStore* ts, BasicBlock* block, u32int conditionCode, u32int reg, u32int tempReg)
{
  ASSERT(conditionCode <= CC_AL, "invalid condition code");
  ASSERT(reg < GPR_PC, "invalid temporary register");

  ARM_ldr_str_imm ldr;
  ARM_ldr_str_imm str;

  block->spills = TRUE;

  // store a register into the next slot of user stack (do not adjust user SP)
  str.value = STR_IMMEDIATE_BASE_VALUE;
  str.fields.imm12 = 4;
  str.fields.Rt = GPR_SP;
  str.fields.Rn = tempReg;
  str.fields.L  = 0;
  str.fields.W  = 0;
  str.fields.B  = 0;
  str.fields.U  = 0;
  str.fields.P  = 1;
  str.fields.I  = 0;
  str.fields.cond = conditionCode;
  addInstructionToBlock(ts, block, str.value);

  armWriteSpillLocToRegister(ts, block, conditionCode, tempReg);

  // spill register into spill address
  str.value = STR_IMMEDIATE_BASE_VALUE;
  str.fields.imm12 = 0;
  str.fields.Rt = tempReg;
  str.fields.Rn = reg;
  str.fields.L  = 0;
  str.fields.W  = 0;
  str.fields.B  = 0;
  str.fields.U  = 0;
  str.fields.P  = 1;
  str.fields.I  = 0;
  str.fields.cond = conditionCode;
  addInstructionToBlock(ts, block, str.value);

  // restore stored temp register (do not adjust user SP)
  ldr.value = LDR_IMMEDIATE_BASE_VALUE;
  ldr.fields.imm12 = 4;
  ldr.fields.Rt = tempReg;
  ldr.fields.Rn = GPR_SP;
  ldr.fields.L  = 1;
  ldr.fields.W  = 0;
  ldr.fields.B  = 0;
  ldr.fields.U  = 0;
  ldr.fields.P  = 1;
  ldr.fields.I  = 0;
  ldr.fields.cond = conditionCode;
  addInstructionToBlock(ts, block, ldr.value);
}


void armRestoreRegister(TranslationStore* ts, BasicBlock* block, u32int conditionCode, u32int reg)
{
  ASSERT(conditionCode <= CC_AL, "invalid condition code");
  ASSERT(reg < GPR_PC, "invalid temporary register");

  ARM_ldr_str_imm ldr;

  armWriteSpillLocToRegister(ts, block, conditionCode, reg);

  // load spilled value back
  ldr.value = LDR_IMMEDIATE_BASE_VALUE;
  ldr.fields.imm12 = 0;
  ldr.fields.Rt = reg;
  ldr.fields.Rn = reg;
  ldr.fields.L  = 1;
  ldr.fields.W  = 0;
  ldr.fields.B  = 0;
  ldr.fields.U  = 1;
  ldr.fields.P  = 0;
  ldr.fields.I  = 0;
  ldr.fields.cond = conditionCode;
  addInstructionToBlock(ts, block, ldr.value);
}


void armWriteSpillLocToRegister(TranslationStore* ts, BasicBlock* block, u32int conditionCode, u32int reg)
{
  u32int sploc = SPILL_PAGE_BEGIN;
  // assemble MOVW
  //MOVW -> ARM ARM A8.6.96 p506
  //|COND|0011|0000|imm4| Rd |    imm12   |
  addInstructionToBlock(ts, block, (conditionCode << 28) | (0b00110000 << 20) | ((sploc & 0xF000) << 4) | (reg << 12) | (sploc & 0x0FFF));

  sploc >>= 16;
  // assemble MOVT
  //MOVT -> ARM ARM A8.6.99 p512
  //|COND|0011|0100|imm4| Rd |    imm12   |
  addInstructionToBlock(ts, block, (conditionCode << 28) | (0b00110100 << 20) | ((sploc & 0xF000) << 4) | (reg << 12) | (sploc & 0x0FFF));
}



void armWritePCToRegister(TranslationStore* ts, BasicBlock* block, u32int conditionCode, u32int reg, u32int pc)
{
  pc += 8;

  // assemble MOVW
  //MOVW -> ARM ARM A8.6.96 p506
  //|COND|0011|0000|imm4| Rd |    imm12   |
  addInstructionToBlock(ts, block, (conditionCode << 28) | (0b00110000 << 20) | ((pc & 0xF000) << 4) | (reg << 12) | (pc & 0x0FFF));

  pc >>= 16;
  // assemble MOVT
  //MOVT -> ARM ARM A8.6.99 p512
  //|COND|0011|0100|imm4| Rd |    imm12   |
  addInstructionToBlock(ts, block, (conditionCode << 28) | (0b00110100 << 20) | ((pc & 0xF000) << 4) | (reg << 12) | (pc & 0x0FFF));
}
