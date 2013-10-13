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
    ARM_ldm_stm push = {.value = LDM_STM_BASE_VALUE};
    push.fields.register_list = 1 << reg;
    push.fields.Rn = 13;
    push.fields.L  = 0;
    push.fields.W  = 1;
    push.fields.S  = 0;
    push.fields.U  = 0;
    push.fields.P  = 1;
    push.fields.cond = cond;
    addInstructionToBlock(ts, block, push.value);
  }
  else
  {
    // spill value to a spare reg in CP15
    ARMMcrInstruction mcr;
    mcr.value = MCR_BASE_VALUE;
    mcr.fields.CRm = 13;
    mcr.fields.opc2 = 2;
    mcr.fields.coproc = 15;
    mcr.fields.Rt = reg;
    mcr.fields.CRn = 9;
    mcr.fields.opc1 = 0;
    mcr.fields.CRn = 13;
    mcr.fields.CRm = 0;
    mcr.fields.opc2 = 2;
    mcr.fields.Rt = reg;
    mcr.fields.cc = cond;
    addInstructionToBlock(ts, block, mcr.value);
  }
}


void armRestoreRegister(TranslationStore* ts, BasicBlock* block, u32int cond, u32int reg)
{
  ASSERT(cond <= AL, "invalid condition code");
  ASSERT(reg < GPR_PC, "invalid temporary register");

  if (getActiveGuestContext()->virtAddrEnabled)
  {
    // pop scratch register.
    ARM_ldm_stm pop = {.value = LDM_STM_BASE_VALUE};
    pop.fields.register_list = 1 << reg;
    pop.fields.Rn = 13;
    pop.fields.L  = 1;
    pop.fields.W  = 1;
    pop.fields.S  = 0;
    pop.fields.U  = 1;
    pop.fields.P  = 0;
    pop.fields.cond = cond;
    addInstructionToBlock(ts, block, pop.value);
  }
  else
  {
    ARMMrcInstruction mrc;
    mrc.value = MRC_BASE_VALUE;
    mrc.fields.CRm = 13;
    mrc.fields.opc2 = 2;
    mrc.fields.coproc = 15;
    mrc.fields.Rt = reg;
    mrc.fields.CRn = 9;
    mrc.fields.opc1 = 0;
    mrc.fields.CRn = 13;
    mrc.fields.CRm = 0;
    mrc.fields.opc2 = 2;
    mrc.fields.Rt = reg;
    mrc.fields.cc = cond;
    addInstructionToBlock(ts, block, mrc.value);
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
