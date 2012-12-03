#include "common/stddef.h"
#include "common/debug.h"


#include "cpuArch/constants.h"

#include "instructionEmu/translator/blockCopy.h"
#include "instructionEmu/decoder/arm/structs.h"


void armSpillRegister(TranslationStore* ts, BasicBlock* block, u32int conditionCode, u32int reg, u32int tempReg)
{
  ASSERT(conditionCode <= CC_AL, "invalid condition code");
  ASSERT(reg < GPR_PC, "invalid temporary register");

  ARMLdrImmediateInstruction ldrPCInstruction;
  ARMStrImmediateInstruction strPCInstruction;
  ARMAddImmediateInstruction addInstruction;

  addInstructionToBlock(ts, block, UNDEFINED_CALL | reg );
  addInstructionToBlock(ts, block, (u32int)&ts->spillLocation);

  // These following instructions are going to be executed in priv mode!

  // push a register onto undefind stack
  strPCInstruction.value = STR_IMMEDIATE_BASE_VALUE;
  strPCInstruction.fields.add = FALSE;
  strPCInstruction.fields.baseRegister = GPR_SP;
  strPCInstruction.fields.conditionCode = conditionCode;
  strPCInstruction.fields.immediate = 4;
  strPCInstruction.fields.index = TRUE;
  strPCInstruction.fields.sourceRegister = tempReg;
  strPCInstruction.fields.writeBackIfNotIndex = TRUE;
  addInstructionToBlock(ts, block, strPCInstruction.value);

  // get spill address
  ldrPCInstruction.value = LDR_IMMEDIATE_BASE_VALUE;
  ldrPCInstruction.fields.add = FALSE;
  ldrPCInstruction.fields.baseRegister = GPR_PC;
  ldrPCInstruction.fields.conditionCode = conditionCode;
  ldrPCInstruction.fields.immediate = 0x10;
  ldrPCInstruction.fields.index = TRUE;
  ldrPCInstruction.fields.sourceRegister = tempReg;
  ldrPCInstruction.fields.writeBack = FALSE;
  addInstructionToBlock(ts, block, ldrPCInstruction.value);

  // spill register into spill address
  strPCInstruction.value = STR_IMMEDIATE_BASE_VALUE;
  strPCInstruction.fields.add = FALSE;
  strPCInstruction.fields.baseRegister = tempReg;
  strPCInstruction.fields.conditionCode = conditionCode;
  strPCInstruction.fields.immediate = 0;
  strPCInstruction.fields.index = TRUE;
  strPCInstruction.fields.sourceRegister = reg;
  strPCInstruction.fields.writeBackIfNotIndex = TRUE;
  addInstructionToBlock(ts, block, strPCInstruction.value);
  
  // restore pushed temp register
  ldrPCInstruction.value = LDR_IMMEDIATE_BASE_VALUE;
  ldrPCInstruction.fields.add = TRUE;
  ldrPCInstruction.fields.baseRegister = GPR_SP;
  ldrPCInstruction.fields.conditionCode = conditionCode;
  ldrPCInstruction.fields.immediate = 4;
  ldrPCInstruction.fields.index = FALSE;
  ldrPCInstruction.fields.sourceRegister = tempReg;
  ldrPCInstruction.fields.writeBack = FALSE;
  addInstructionToBlock(ts, block, ldrPCInstruction.value);

  // NOW return to user mode
  addInstruction.value = ADD_IMMEDIATE_BASE_VALUE;
  addInstruction.fields.conditionCode = conditionCode;
  addInstruction.fields.destinationRegister = GPR_PC;
  addInstruction.fields.immediate = 0x18;
  addInstruction.fields.operandRegister = GPR_LR;
  addInstruction.fields.setFlags = TRUE;
  addInstructionToBlock(ts, block, addInstruction.value);
}


void armRestoreRegister(TranslationStore* ts, BasicBlock* block, u32int conditionCode, u32int reg)
{
  ASSERT(conditionCode <= CC_AL, "invalid condition code");
  ASSERT(reg < GPR_PC, "invalid temporary register");

  ARMLdrImmediateInstruction ldrPCInstruction;
  ARMAddImmediateInstruction addInstruction;

  addInstructionToBlock(ts, block, UNDEFINED_CALL | reg );
  addInstructionToBlock(ts, block, (u32int)&ts->spillLocation);

  // get spill address
  ldrPCInstruction.value = LDR_IMMEDIATE_BASE_VALUE;
  ldrPCInstruction.fields.add = FALSE;
  ldrPCInstruction.fields.baseRegister = GPR_PC;
  ldrPCInstruction.fields.conditionCode = conditionCode;
  ldrPCInstruction.fields.immediate = 0xC;
  ldrPCInstruction.fields.index = TRUE;
  ldrPCInstruction.fields.sourceRegister = reg;
  ldrPCInstruction.fields.writeBack = FALSE;
  addInstructionToBlock(ts, block, ldrPCInstruction.value);

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

  // return to user mode
  addInstruction.value = ADD_IMMEDIATE_BASE_VALUE;
  addInstruction.fields.conditionCode = conditionCode;
  addInstruction.fields.destinationRegister = GPR_PC;
  addInstruction.fields.immediate = 0x10;
  addInstruction.fields.operandRegister = GPR_LR;
  addInstruction.fields.setFlags = TRUE;
  addInstructionToBlock(ts, block, addInstruction.value);
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
