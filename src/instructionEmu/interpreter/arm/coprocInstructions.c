#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/arm/coprocInstructions.h"

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
  DEBUG(INTERPRETER_ARM_COPROC, "armMcrInstruction: %#.8x @ %#.8x" EOL, instruction, getRealPC(context));

  if (evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    u32int coprocNum = (instruction & 0x00000F00) >> 8;
    ASSERT(coprocNum == 0xF, "unknown coprocessor");

    // control coprocessor 15. good. reg op2 and source registers
    u32int cRegSrc1 = (instruction & 0x000F0000) >> 16;
    u32int cRegSrc2 = (instruction & 0x0000000F);
    u32int opcode1 = (instruction & 0x00E00000) >> 21;
    u32int opcode2 = (instruction & 0x000000E0) >> 5;
    u32int regSrcNr = (instruction & 0x0000F000) >> 12;
    u32int srcVal = loadGuestGPR(regSrcNr, context);
    setCregVal(context, CRB_INDEX(cRegSrc1, opcode1, cRegSrc2, opcode2), srcVal);
  }

  return getRealPC(context) + ARM_INSTRUCTION_SIZE;
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
  DEBUG(INTERPRETER_ARM_COPROC, "armMrcInstruction: %#.8x @ %#.8x" EOL, instruction, getRealPC(context));

  if (evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    u32int coprocNum = (instruction & 0x00000F00) >> 8;
    ASSERT(coprocNum == 0xF, "unknown coprocessor");

    // control coprocessor 15. good. reg op2 and source registers
    u32int cRegSrc1 = (instruction & 0x000F0000) >> 16;
    u32int cRegSrc2 = (instruction & 0x0000000F);
    u32int opcode1 = (instruction & 0x00E00000) >> 21;
    u32int opcode2 = (instruction & 0x000000E0) >> 5;
    u32int cregVal = getCregVal(context->coprocRegBank, CRB_INDEX(cRegSrc1, opcode1, cRegSrc2, opcode2));
    u32int regDestNr = (instruction & 0x0000F000) >> 12;
    ASSERT(regDestNr != 0xF, ERROR_NOT_IMPLEMENTED);
    storeGuestGPR(regDestNr, cregVal, context);
  }

  return getRealPC(context) + ARM_INSTRUCTION_SIZE;
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
