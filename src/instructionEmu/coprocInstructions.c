#include "common/debug.h"

#include "instructionEmu/commonInstrFunctions.h"
#include "instructionEmu/coprocInstructions.h"

#include "memoryManager/cp15coproc.h"


u32int mcrrInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "MCRR instruction unimplemented");
}

u32int mrrcInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "MRRC instruction unimplemented");
}

u32int cdpInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "CDP instruction unimplemented");
}

u32int mrcInstruction(GCONTXT *context, u32int instr)
{
  u32int nextPC = 0;

  int instrCC = (instr >> 28) & 0xF;

#ifdef COPROC_INSTR_TRACE
  printf("MRC instr %#.8x @ %#.8x" EOL, instr, context->R15);
#endif

  if (evaluateConditionCode(context, instrCC))
  {
    u32int coprocNum = (instr & 0x00000F00) >> 8;
    if (coprocNum == 0xF)
    {
      // control coprocessor 15. good. reg op2 and source registers
      u32int cRegSrc1 = (instr & 0x000F0000) >> 16;
      u32int cRegSrc2 = (instr & 0x0000000F);
      u32int opcode1 = (instr & 0x00E00000) >> 21;
      u32int opcode2 = (instr & 0x000000E0) >> 5;
      u32int cregVal = getCregVal(cRegSrc1, opcode1, cRegSrc2, opcode2, context->coprocRegBank);
      u32int regDestNr = (instr & 0x0000F000) >> 12;
      if (regDestNr == 0xF)
      {
        DIE_NOW(context, "mrcInstruction: unimplemented load from CP15 to PC");
      }
      storeGuestGPR(regDestNr, cregVal, context);
    }
    else
    {
      DIE_NOW(context, "mrcInstruction: unknown coprocessor number");
    }
  }

  nextPC = context->R15 + 4;
  return nextPC;
}


u32int mcrInstruction(GCONTXT *context, u32int instr)
{
  u32int nextPC = 0;

  int instrCC = (instr >> 28) & 0xF;

#ifdef COPROC_INSTR_TRACE
  printf("MCR instr %#.8x @ %#.8x" EOL, instr, context->R15);
#endif

  if (evaluateConditionCode(context, instrCC))
  {
    u32int coprocNum = (instr & 0x00000F00) >> 8;
    if (coprocNum == 0xF)
    {
      // control coprocessor 15. good. reg op2 and source registers
      u32int cRegSrc1 = (instr & 0x000F0000) >> 16;
      u32int cRegSrc2 = (instr & 0x0000000F);
      u32int opcode1 = (instr & 0x00E00000) >> 21;
      u32int opcode2 = (instr & 0x000000E0) >> 5;
      u32int regSrcNr = (instr & 0x0000F000) >> 12;
      u32int srcVal = loadGuestGPR(regSrcNr, context);
      setCregVal(cRegSrc1, opcode1, cRegSrc2, opcode2, context->coprocRegBank, srcVal);
    }
    else
    {
      DIE_NOW(context, "mcrInstruction: unknown coprocessor number");
    }
  }

  nextPC = context->R15 + 4;
  return nextPC;
}

u32int stcInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "STC instruction unimplemented");
}

u32int ldcInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "LDC instruction unimplemented");
}

/* V6 coprocessor instructions.  */
u32int mrrc2Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "MRRC2 instruction unimplemented");
}

u32int mcrr2Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "MCRR2 instruction unimplemented");
}

/* V5 coprocessor instructions.  */
u32int ldc2Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "LDC2 instruction unimplemented");
}

u32int stc2Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "STC2 instruction unimplemented");
}

u32int cdp2Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "CDP2 instruction unimplemented");
}

u32int mcr2Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "MCR2 instruction unimplemented");
}

u32int mrc2Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "MRC2 instruction unimplemented");
}
