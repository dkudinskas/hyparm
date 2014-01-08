#include "instructionEmu/decoder/arm/structs.h"
#include "instructionEmu/interpreter/common.h"
#include "instructionEmu/interpreter/internals.h"
#include "instructionEmu/interpreter/arm/coprocInstructions.h"

#include "perf/contextSwitchCounters.h"

#include "vm/omap35xx/cp15coproc.h"


u32int armCdpInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armCdp2Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armLdcInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armLdc2Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armMcrInstruction(GCONTXT *context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG(INTERPRETER_ARM_COPROC, "armMcrInstruction: %#.8x @ %#.8x\n", instruction, context->R15);
  countMcr(context);
  if (ConditionPassed(instr.mcr.cc))
  {
    // if coproc IN "101x" then SEE "Advanced SIMD and Floating-point";
    u8int Rt = instr.mcr.Rt;
    if ((Rt == GPR_PC) || ((Rt == GPR_SP) && (CurrentInstrSet() != InstrSet_ARM)))
      UNPREDICTABLE();

    u32int val = getGPRegister(context, Rt);
    setCregVal(context, CRB_INDEX(instr.mcr.CRn, instr.mcr.opc1,
                                  instr.mcr.CRm, instr.mcr.opc2), val);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armMcr2Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armMcrrInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armMcrr2Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armMrcInstruction(GCONTXT *context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG(INTERPRETER_ARM_COPROC, "armMcrInstruction: %#.8x @ %#.8x\n", instruction, context->R15);
  countMrc(context);
  // can reuse mcr encoding
  if (ConditionPassed(instr.mcr.cc))
  {
    // if coproc IN "101x" then SEE "Advanced SIMD and Floating-point";
    u8int Rt = instr.mcr.Rt;
    if ((Rt == GPR_SP) && (CurrentInstrSet() != InstrSet_ARM))
      UNPREDICTABLE();

    u32int val = getCregVal(context, CRB_INDEX(instr.mcr.CRn, instr.mcr.opc1,
                                               instr.mcr.CRm, instr.mcr.opc2));
    if (Rt == GPR_PC)
    {
      // can only set condition codes.
      val &= 0xf0000000;
    }
    setGPRegister(context, Rt, val);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armMrc2Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armMrrcInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armMrrc2Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armStcInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armStc2Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}
