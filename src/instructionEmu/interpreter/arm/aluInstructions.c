#include "instructionEmu/interpreter/common.h"
#include "instructionEmu/interpreter/internals.h"
#include "instructionEmu/interpreter/arm/aluInstructions.h"

#include "perf/contextSwitchCounters.h"

u32int armAluImmInstruction(GCONTXT *context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_ALU, context, instruction);
  countAluInstruction(context, instr);

  if (ConditionPassed(instr.aluImm.cc))
  {
    // we can only get to this function if Rd == PC
    u8int Rn = instr.aluImm.Rn;

    u32int operand1 = getGPRegister(context, Rn);
    u32int operand2 = armExpandImm(instr.aluImm.imm12, context->CPSR.bits.C);
    u32int result = 0;
    switch (instr.aluImm.opc0)
    {
      case ADD: result = operand1 + operand2; break;
      default:
        printf("armAluImmInstruction: unimplemented opc0 %d\n", instr.aluReg.opc0);
        DIE_NOW(context, "stop");
    }

    if (instr.aluReg.S)
      CPSRWriteByInstr(context, SPSR(context).value, 0xF, TRUE);
    ALUWritePC(context, result & ~3);
    return context->R15;
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armAluRegInstruction(GCONTXT *context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_ALU, context, instruction);
  countAluInstruction(context, instr);
  if (ConditionPassed(instr.aluReg.cc))
  {
    // we can only get to this function if Rd == PC
    u8int Rn = instr.aluReg.Rn;

    u32int shiftAmount = 0;
    u32int shiftType = decodeShiftImmediate(instr.aluReg.type, 
                                            instr.aluReg.imm5, &shiftAmount);
    u32int Rm = getGPRegister(context, instr.aluReg.Rm);
    u32int operand1 = getGPRegister(context, Rn);
    u32int operand2 = shiftVal(Rm, shiftType, shiftAmount);
    u32int result = 0;
    switch (instr.aluReg.opc1)
    {
      case AND: result = operand1 & operand2; break;
      case ADD: result = operand1 + operand2; break;
      case MOV:
        if (instr.aluReg.imm5 != 0)
          DIE_NOW(context, "movReg: shamt != 0");
        result = Rm;
        break;
      default:
        printf("armAluRegInstruction: unimplemented opc1 %x\n", instr.aluReg.opc1);
        DIE_NOW(context, "stop");
    }

    if (instr.aluReg.S)
      CPSRWriteByInstr(context, SPSR(context).value, 0xF, TRUE);

    ALUWritePC(context, result & ~3);
    return context->R15;
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}
