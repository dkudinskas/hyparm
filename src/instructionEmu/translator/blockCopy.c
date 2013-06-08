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

  ARM_ldr_str_imm loadStore;

  block->spills = TRUE;

  // store a register into the next slot of user stack (do not adjust user SP)
  loadStore.value = STR_IMMEDIATE_BASE_VALUE;
  loadStore.fields.imm12 = 4;
  loadStore.fields.Rt = tempReg;
  loadStore.fields.Rn = GPR_SP;
  loadStore.fields.L = 0;
  loadStore.fields.W = 0;
  loadStore.fields.B = 0;
  loadStore.fields.U = 0;
  loadStore.fields.P = 1;
  loadStore.fields.I = 0;
  loadStore.fields.cond = conditionCode;
  addInstructionToBlock(ts, block, loadStore.value);

  armWriteSpillLocToRegister(ts, block, conditionCode, tempReg);

  // spill register into spill address
  loadStore.value = STR_IMMEDIATE_BASE_VALUE;
  loadStore.fields.imm12 = 0;
  loadStore.fields.Rt = reg;
  loadStore.fields.Rn = tempReg;
  loadStore.fields.L = 0;
  loadStore.fields.W = 0;
  loadStore.fields.B = 0;
  loadStore.fields.U = 0;
  loadStore.fields.P = 1;
  loadStore.fields.I = 0;
  loadStore.fields.cond = conditionCode;
  addInstructionToBlock(ts, block, loadStore.value);

  // restore stored temp register (do not adjust user SP)
  loadStore.value = LDR_IMMEDIATE_BASE_VALUE;
  loadStore.fields.imm12 = 4;
  loadStore.fields.Rt = tempReg;
  loadStore.fields.Rn = GPR_SP;
  loadStore.fields.L = 1;
  loadStore.fields.W = 0;
  loadStore.fields.B = 0;
  loadStore.fields.U = 0;
  loadStore.fields.P = 1;
  loadStore.fields.I = 0;
  loadStore.fields.cond = conditionCode;
  addInstructionToBlock(ts, block, loadStore.value);
}


void armRestoreRegister(TranslationStore* ts, BasicBlock* block, u32int conditionCode, u32int reg)
{
  ASSERT(conditionCode <= CC_AL, "invalid condition code");
  ASSERT(reg < GPR_PC, "invalid temporary register");

  ARM_ldr_str_imm loadStore;

  armWriteSpillLocToRegister(ts, block, conditionCode, reg);

  // load spilled value back
  loadStore.value = LDR_IMMEDIATE_BASE_VALUE;
  loadStore.fields.imm12 = 0;
  loadStore.fields.Rt = reg;
  loadStore.fields.Rn = reg;
  loadStore.fields.L = 1;
  loadStore.fields.W = 0;
  loadStore.fields.B = 0;
  loadStore.fields.U = 1;
  loadStore.fields.P = 0;
  loadStore.fields.I = 0;
  loadStore.fields.cond = conditionCode;
  addInstructionToBlock(ts, block, loadStore.value);
}

void armWriteSpillLocToRegister(TranslationStore* ts, BasicBlock* block,
                                u32int conditionCode, u32int reg)
{
  u32int sploc = SPILL_PAGE_BEGIN;
  // assemble MOVW
  //MOVW -> ARM ARM A8.6.96 p506
  //|COND|0011|0000|imm4| Rd |    imm12   |
  addInstructionToBlock(ts, block, (conditionCode << 28) | (0b00110000 << 20) |
                      ((sploc & 0xF000) << 4) | (reg << 12) | (sploc & 0x0FFF));

  sploc >>= 16;
  // assemble MOVT
  //MOVT -> ARM ARM A8.6.99 p512
  //|COND|0011|0100|imm4| Rd |    imm12   |
  addInstructionToBlock(ts, block, (conditionCode << 28) | (0b00110100 << 20) |
                       ((sploc & 0xF000) << 4) | (reg << 12) | (sploc & 0x0FFF));
}



void armWritePCToRegister(TranslationStore* ts, BasicBlock* block,
                          u32int conditionCode, u32int reg, u32int pc)
{
  pc += 8;

  // assemble MOVW
  //MOVW -> ARM ARM A8.6.96 p506
  //|COND|0011|0000|imm4| Rd |    imm12   |
  addInstructionToBlock(ts, block, (conditionCode << 28) | (0b00110000 << 20) |
                      ((pc & 0xF000) << 4) | (reg << 12) | (pc & 0x0FFF));

  pc >>= 16;
  // assemble MOVT
  //MOVT -> ARM ARM A8.6.99 p512
  //|COND|0011|0100|imm4| Rd |    imm12   |
  addInstructionToBlock(ts, block, (conditionCode << 28) | (0b00110100 << 20) |
                      ((pc & 0xF000) << 4) | (reg << 12) | (pc & 0x0FFF));
}
