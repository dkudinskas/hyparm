#include "coprocInstructions.h"
#include "commonInstrFunctions.h"
#include "debug.h"
#include "serial.h"
#include "cp15coproc.h"

u32int* mcrrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "mcrr PCFunct unfinished\n");
  return 0;
}

u32int mcrrInstruction(GCONTXT * context)
{
  DIE_NOW(0, "mcrrInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "MCRR instruction unimplemented");
}

u32int* mrrcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "mrrc PCFunct unfinished\n");
  return 0;
}

u32int mrrcInstruction(GCONTXT * context)
{
  DIE_NOW(0, "mrrcInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "MRRC instruction unimplemented");
}

u32int* cdpPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "cdp PCFunct unfinished\n");
  return 0;
}

u32int cdpInstruction(GCONTXT * context)
{
  DIE_NOW(0, "cdpInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "CDP instruction unimplemented");
}

u32int* mrcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "mrcPCInstruction is called but should always be critical");
  return 0;
}

u32int mrcInstruction(GCONTXT * context)
{
  //mrcInstruction only reads from coprocessor registers and cannot write to PC -> only adapt nextPC
  u32int nextPC = 0;
  u32int instr = context->endOfBlockInstr;
  int instrCC = (instr >> 28) & 0xF;
  u32int cpsrCC = (context->CPSR >> 28) & 0xF;
  bool conditionMet = evalCC(instrCC, cpsrCC);
#ifdef COPROC_INSTR_TRACE
  serial_putstring("MRC instr ");
  serial_putint(instr);
  serial_putstring(" @ ");
  serial_putint(context->PCOfLastInstruction);
  serial_newline();
#endif
  if (conditionMet)
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
        DIE_NOW(context, "unimplemented load from CP15 to PC");
      }
      storeGuestGPR(regDestNr, cregVal, context);
    }
    else
    {
      invalid_instruction(instr, "Unknown coprocessor number");
    }
  }
  nextPC = context->PCOfLastInstruction + 4;
  return nextPC;
}

u32int* mcrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "mcr PCFunct unfinished\n");
  return 0;
}

u32int mcrInstruction(GCONTXT * context)
{
  DIE_NOW(0, "mcrInstruction is executed but not yet checked for blockCopyCompatibility");
  u32int nextPC = 0;
  u32int instr = context->endOfBlockInstr;
  int instrCC = (instr >> 28) & 0xF;
  u32int cpsrCC = (context->CPSR >> 28) & 0xF;
  bool conditionMet = evalCC(instrCC, cpsrCC);
#ifdef COPROC_INSTR_TRACE
  serial_putstring("MCR instr ");
  serial_putint(instr);
  serial_putstring(" @ ");
  serial_putint(context->R15);
  serial_newline();
#endif
  if (conditionMet)
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
      invalid_instruction(instr, "Unknown coprocessor number");
    }
  }
  nextPC = context->R15 + 4;
  return nextPC;
}

u32int* stcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "stc PCFunct unfinished\n");
  return 0;
}

u32int stcInstruction(GCONTXT * context)
{
  DIE_NOW(0, "stcInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "STC instruction unimplemented");
}

u32int* ldcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "ldc PCFunct unfinished\n");
  return 0;
}

u32int ldcInstruction(GCONTXT * context)
{
  DIE_NOW(0, "ldcInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "LDC instruction unimplemented");
}
/* V6 coprocessor instructions.  */

u32int* mrrc2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "mrrc2 PCFunct unfinished\n");
  return 0;
}

u32int mrrc2Instruction(GCONTXT * context)
{
  DIE_NOW(0, "mrrc2Instruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "MRRC2 instruction unimplemented");
}

u32int* mcrr2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "mcrr2 PCFunct unfinished\n");
  return 0;
}

u32int mcrr2Instruction(GCONTXT * context)
{
  DIE_NOW(0, "mcrr2Instruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "MCRR2 instruction unimplemented");
}
/* V5 coprocessor instructions.  */

u32int* ldc2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "ldc2 PCFunct unfinished\n");
  return 0;
}

u32int ldc2Instruction(GCONTXT * context)
{
  DIE_NOW(0, "ldc2Instruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "LDC2 instruction unimplemented");
}

u32int* stc2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "stc2 PCFunct unfinished\n");
  return 0;
}

u32int stc2Instruction(GCONTXT * context)
{
  DIE_NOW(0, "stc2Instruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "STC2 instruction unimplemented");
}

u32int* cdp2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "cdp2 PCFunct unfinished\n");
  return 0;
}

u32int cdp2Instruction(GCONTXT * context)
{
  DIE_NOW(0, "cdp2Instruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "CDP2 instruction unimplemented");
}

u32int* mcr2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "mcr2 PCFunct unfinished\n");
  return 0;
}

u32int mcr2Instruction(GCONTXT * context)
{
  DIE_NOW(0, "mcr2Instruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "MCR2 instruction unimplemented");
}

u32int* mrc2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "mrc2 PCFunct unfinished\n");
  return 0;
}

u32int mrc2Instruction(GCONTXT * context)
{
  DIE_NOW(0, "mrc2Instruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "MRC2 instruction unimplemented");
}
