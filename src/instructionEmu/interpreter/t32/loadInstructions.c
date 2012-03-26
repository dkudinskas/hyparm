#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/t32/loadInstructions.h"


u32int t32LdrbInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T32_LOAD, context, instruction);
  DIE_NOW(context, "not implemented");
}

u32int t32LdrhImmediateInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T32_LOAD, context, instruction);
  DIE_NOW(context, "not implemented");
}

u32int t32LdrhLiteralInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T32_LOAD, context, instruction);
  DIE_NOW(context, "not implemented");
}

u32int t32LdrhRegisterInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T32_LOAD, context, instruction);
  DIE_NOW(context, "not implemented");
}

u32int t32LdrdInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T32_LOAD, context, instruction);
  DIE_NOW(context, "not implemented");
}

u32int t32LdrshImmediate8Instruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T32_LOAD, context, instruction);
  DIE_NOW(context, "not implemented");
}

u32int t32LdrshImmediate12Instruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T32_LOAD, context, instruction);

  u32int regSrc = (instruction & 0x000F0000)>>16;
  u32int regDst = (instruction & 0x0000F000)>>12;
  u32int imm12 = (instruction & 0x00000FFF);
  u32int baseAddress = loadGuestGPR(regSrc, context);
  u32int offsetAddress = baseAddress + imm12;

  storeGuestGPR(regDst, vmLoad(HALFWORD, offsetAddress) & 0xFFFF, context);
  return context->R15 + T32_INSTRUCTION_SIZE;
}

u32int t32LdrshLiteralInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T32_LOAD, context, instruction);
  DIE_NOW(context, "not implemented");
}

u32int t32LdrshRegisterInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T32_LOAD, context, instruction);
  DIE_NOW(context, "not implemented");
}
