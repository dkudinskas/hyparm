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

  ARMLdrImmediateInstruction ldrPCInstruction;
  ARMStrImmediateInstruction strPCInstruction;

  // store a register into the next slot of user stack (do not adjust user SP)
  strPCInstruction.value = STR_IMMEDIATE_BASE_VALUE;
  strPCInstruction.fields.add = FALSE;
  strPCInstruction.fields.baseRegister = GPR_SP;
  strPCInstruction.fields.conditionCode = conditionCode;
  strPCInstruction.fields.immediate = 4;
  strPCInstruction.fields.index = TRUE;
  strPCInstruction.fields.sourceRegister = tempReg;
  strPCInstruction.fields.writeBackIfNotIndex = FALSE;
  addInstructionToBlock(ts, block, strPCInstruction.value);

  armWriteSpillLocToRegister(ts, block, conditionCode, tempReg);

  // spill register into spill address
  strPCInstruction.value = STR_IMMEDIATE_BASE_VALUE;
  strPCInstruction.fields.add = FALSE;
  strPCInstruction.fields.baseRegister = tempReg;
  strPCInstruction.fields.conditionCode = conditionCode;
  strPCInstruction.fields.immediate = 0;
  strPCInstruction.fields.index = TRUE;
  strPCInstruction.fields.sourceRegister = reg;
  strPCInstruction.fields.writeBackIfNotIndex = FALSE;
  addInstructionToBlock(ts, block, strPCInstruction.value);

  // restore stored temp register (do not adjust user SP)
  ldrPCInstruction.value = LDR_IMMEDIATE_BASE_VALUE;
  ldrPCInstruction.fields.add = FALSE;
  ldrPCInstruction.fields.baseRegister = GPR_SP;
  ldrPCInstruction.fields.conditionCode = conditionCode;
  ldrPCInstruction.fields.immediate = 4;
  ldrPCInstruction.fields.index = TRUE;
  ldrPCInstruction.fields.sourceRegister = tempReg;
  ldrPCInstruction.fields.writeBack = FALSE;
  addInstructionToBlock(ts, block, ldrPCInstruction.value);
}


void armRestoreRegister(TranslationStore* ts, BasicBlock* block, u32int conditionCode, u32int reg)
{
  ASSERT(conditionCode <= CC_AL, "invalid condition code");
  ASSERT(reg < GPR_PC, "invalid temporary register");

  ARMLdrImmediateInstruction ldrPCInstruction;

  armWriteSpillLocToRegister(ts, block, conditionCode, reg);

  // load spilled value back
  ldrPCInstruction.value = LDR_IMMEDIATE_BASE_VALUE;
  ldrPCInstruction.fields.add = TRUE;
  ldrPCInstruction.fields.baseRegister = reg;
  ldrPCInstruction.fields.conditionCode = conditionCode;
  ldrPCInstruction.fields.immediate = 0;
  ldrPCInstruction.fields.index = FALSE;
  ldrPCInstruction.fields.sourceRegister = reg;
  ldrPCInstruction.fields.writeBack = FALSE;
  addInstructionToBlock(ts, block, ldrPCInstruction.value);
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
