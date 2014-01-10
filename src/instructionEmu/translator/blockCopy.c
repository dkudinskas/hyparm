#include "common/stddef.h"
#include "common/debug.h"

#include "cpuArch/constants.h"

#include "common/linker.h"

#include "instructionEmu/translator/blockCopy.h"
#include "instructionEmu/decoder/arm/structs.h"


void armSpillRegister(TranslationStore* ts, BasicBlock* block, u32int cond, u32int reg, u32int tempReg)
{
  ASSERT(cond <= AL, "invalid condition code");
  ASSERT(reg < GPR_PC, "invalid temporary register");

  if (getActiveGuestContext()->virtAddrEnabled)
  {
    // push scratch register to user stack
    Instruction push = {.raw = LDM_STM_BASE_VALUE};
    push.ldStMulti.regList = 1 << reg;
    push.ldStMulti.Rn = 13;
    push.ldStMulti.load = 0;
    push.ldStMulti.W = 1;
    push.ldStMulti.user = 0;
    push.ldStMulti.U = 0;
    push.ldStMulti.P = 1;
    push.ldStMulti.cc = cond;
    addInstructionToBlock(ts, block, push.raw);
  }
  else
  {
    // spill value to a spare reg in CP15
    Instruction instr = {.raw = MCR_BASE_VALUE};
    instr.mcr.CRm = 13;
    instr.mcr.opc2 = 2;
    instr.mcr.coproc = 15;
    instr.mcr.Rt = reg;
    instr.mcr.CRn = 9;
    instr.mcr.opc1 = 0;
    instr.mcr.CRn = 13;
    instr.mcr.CRm = 0;
    instr.mcr.opc2 = 2;
    instr.mcr.Rt = reg;
    instr.mcr.cc = cond;
    addInstructionToBlock(ts, block, instr.raw);
  }
}


void armRestoreRegister(TranslationStore* ts, BasicBlock* block, u32int cond, u32int reg)
{
  ASSERT(cond <= AL, "invalid condition code");
  ASSERT(reg < GPR_PC, "invalid temporary register");

  if (getActiveGuestContext()->virtAddrEnabled)
  {
    // pop scratch register.
    Instruction pop = {.raw = LDM_STM_BASE_VALUE};
    pop.ldStMulti.regList = 1 << reg;
    pop.ldStMulti.Rn = 13;
    pop.ldStMulti.load = 1;
    pop.ldStMulti.W = 1;
    pop.ldStMulti.user = 0;
    pop.ldStMulti.U = 1;
    pop.ldStMulti.P = 0;
    pop.ldStMulti.cc = cond;
    addInstructionToBlock(ts, block, pop.raw);
  }
  else
  {
    Instruction instr = {.raw = MRC_BASE_VALUE};
    instr.mcr.CRm = 13;
    instr.mcr.opc2 = 2;
    instr.mcr.coproc = 15;
    instr.mcr.Rt = reg;
    instr.mcr.CRn = 9;
    instr.mcr.opc1 = 0;
    instr.mcr.CRn = 13;
    instr.mcr.CRm = 0;
    instr.mcr.opc2 = 2;
    instr.mcr.Rt = reg;
    instr.mcr.cc = cond;
    addInstructionToBlock(ts, block, instr.raw);
  }
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
