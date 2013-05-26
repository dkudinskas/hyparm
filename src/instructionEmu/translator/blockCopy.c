#include "common/stddef.h"
#include "common/debug.h"

#include "cpuArch/constants.h"

#include "common/linker.h"

#include "instructionEmu/translator/blockCopy.h"
#include "instructionEmu/decoder/arm/structs.h"


void armSpillRegister(TranslationStore* ts, BasicBlock* block, u32int conditionCode, u32int reg)
{
  ASSERT(conditionCode <= CC_AL, "invalid condition code");
  ASSERT(reg < GPR_PC, "invalid temporary register");

  ARMMcrInstruction mcr;

  mcr.value = MCR_BASE_VALUE;
  mcr.fields.CRm = 13;
  mcr.fields.opc2 = 2;
  mcr.fields.coproc = 15;
  mcr.fields.Rt = reg;
  mcr.fields.CRn = 9;
  mcr.fields.opc1 = 0;
  mcr.fields.cc = conditionCode;

  addInstructionToBlock(ts, block, mcr.value);
}


void armRestoreRegister(TranslationStore* ts, BasicBlock* block, u32int conditionCode, u32int reg)
{
  ASSERT(conditionCode <= CC_AL, "invalid condition code");
  ASSERT(reg < GPR_PC, "invalid temporary register");

  ARMMrcInstruction mrc;

  mrc.value = MRC_BASE_VALUE;
  mrc.fields.CRm = 13;
  mrc.fields.opc2 = 2;
  mrc.fields.coproc = 15;
  mrc.fields.Rt = reg;
  mrc.fields.CRn = 9;
  mrc.fields.opc1 = 0;
  mrc.fields.cc = conditionCode;

  addInstructionToBlock(ts, block, mrc.value);
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
