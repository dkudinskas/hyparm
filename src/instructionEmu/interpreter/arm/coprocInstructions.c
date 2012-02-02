#include "common/debug.h"

#include "cpuArch/constants.h"

#include "instructionEmu/commonInstrFunctions.h"

#include "instructionEmu/interpreter/coprocInstructions.h"

#include "memoryManager/cp15coproc.h"


u32int armCdpInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "CDP instruction unimplemented");
}

u32int armCdp2Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "CDP2 instruction unimplemented");
}

u32int armLdcInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "LDC instruction unimplemented");
}

u32int armLdc2Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "LDC2 instruction unimplemented");
}

u32int armMcrInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG(INTERPRETER_ARM_COPROC, "armMcrInstruction: %#.8x @ %#.8x" EOL, instruction, getRealPC(context));

  if (evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    u32int coprocNum = (instruction & 0x00000F00) >> 8;
    if (coprocNum == 0xF)
    {
      // control coprocessor 15. good. reg op2 and source registers
      u32int cRegSrc1 = (instruction & 0x000F0000) >> 16;
      u32int cRegSrc2 = (instruction & 0x0000000F);
      u32int opcode1 = (instruction & 0x00E00000) >> 21;
      u32int opcode2 = (instruction & 0x000000E0) >> 5;
      u32int regSrcNr = (instruction & 0x0000F000) >> 12;
      u32int srcVal = loadGuestGPR(regSrcNr, context);
      setCregVal(cRegSrc1, opcode1, cRegSrc2, opcode2, context->coprocRegBank, srcVal);
    }
    else
    {
      DIE_NOW(context, "armMcrInstruction: unknown coprocessor number");
    }
  }

  return getRealPC(context) + ARM_INSTRUCTION_SIZE;
}

u32int armMcr2Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "MCR2 instruction unimplemented");
}

u32int armMcrrInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "MCRR instruction unimplemented");
}

u32int armMcrr2Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "MCRR2 instruction unimplemented");
}

u32int armMrcInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG(INTERPRETER_ARM_COPROC, "armMrcInstruction: %#.8x @ %#.8x" EOL, instruction, getRealPC(context));

  if (evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    u32int coprocNum = (instruction & 0x00000F00) >> 8;
    if (coprocNum == 0xF)
    {
      // control coprocessor 15. good. reg op2 and source registers
      u32int cRegSrc1 = (instruction & 0x000F0000) >> 16;
      u32int cRegSrc2 = (instruction & 0x0000000F);
      u32int opcode1 = (instruction & 0x00E00000) >> 21;
      u32int opcode2 = (instruction & 0x000000E0) >> 5;
      u32int cregVal = getCregVal(cRegSrc1, opcode1, cRegSrc2, opcode2, context->coprocRegBank);
      u32int regDestNr = (instruction & 0x0000F000) >> 12;
      if (regDestNr == 0xF)
      {
        DIE_NOW(context, "armMrcInstruction: unimplemented load from CP15 to PC");
      }
      storeGuestGPR(regDestNr, cregVal, context);
    }
    else
    {
      DIE_NOW(context, "armMrcInstruction: unknown coprocessor number");
    }
  }

  return getRealPC(context) + ARM_INSTRUCTION_SIZE;
}

u32int armMrc2Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "MRC2 instruction unimplemented");
}

u32int armMrrcInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "MRRC instruction unimplemented");
}

u32int armMrrc2Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "MRRC2 instruction unimplemented");
}

u32int armStcInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "STC instruction unimplemented");
}

u32int armStc2Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "STC2 instruction unimplemented");
}
