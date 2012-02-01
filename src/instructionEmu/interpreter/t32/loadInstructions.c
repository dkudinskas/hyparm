#include "common/debug.h"

#include "cpuArch/constants.h"

#include "instructionEmu/commonInstrFunctions.h"

#include "instructionEmu/interpreter/loadInstructions.h"


u32int t32LdrbInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "t32LdrbInstruction not implemented");
}

u32int t32LdrhImmediateInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "t32LdrhImmediateInstruction not implemented");
}

u32int t32LdrhLiteralInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "t32LdrhLiteralInstruction not implemented");
}

u32int t32LdrhRegisterInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "t32LdrhRegisterInstruction not implemented");
}

u32int t32LdrdInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "t32LdrdInstruction not implemented");
}

u32int t32LdrshImmediate8Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "t32LdrshImmediate8Instruction not implemented");
}

u32int t32LdrshImmediate12Instruction(GCONTXT *context, u32int instruction)
{
  u32int regSrc = (instruction & 0x000F0000)>>16;
  u32int regDst = (instruction & 0x0000F000)>>12;
  u32int imm12 = (instruction & 0x00000FFF);

  u32int baseAddress = loadGuestGPR(regSrc, context);
  u32int offsetAddress = baseAddress + imm12;
  u32int valueLoaded = context->hardwareLibrary->loadFunction(context->hardwareLibrary, HALFWORD, offsetAddress);
  storeGuestGPR(regDst, valueLoaded, context);

  return context->R15 + T32_INSTRUCTION_SIZE;
}

u32int t32LdrshLiteralInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "t32LdrshLiteralInstruction not implemented");
}

u32int t32LdrshRegisterInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "t32LdrshRegisterInstruction not implemented");
}
