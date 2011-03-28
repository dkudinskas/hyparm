#include "common/debug.h"

#include "vm/omap35xx/serial.h"

#include "instructionEmu/commonInstrFunctions.h"
#include "instructionEmu/coprocInstructions.h"

#include "memoryManager/cp15coproc.h"

#ifdef CONFIG_BLOCK_COPY
u32int* mcrrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "mcrr PCFunct unfinished\n");
  return 0;
}
#endif

u32int mcrrInstruction(GCONTXT * context)
{
  DIE_NOW(context, "MCRR instruction unimplemented");
}

#ifdef CONFIG_BLOCK_COPY
u32int* mrrcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "mrrc PCFunct unfinished\n");
  return 0;
}
#endif

u32int mrrcInstruction(GCONTXT * context)
{
  DIE_NOW(context, "MRRC instruction unimplemented");
}

#ifdef CONFIG_BLOCK_COPY
u32int* cdpPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "cdp PCFunct unfinished\n");
  return 0;
}
#endif

u32int cdpInstruction(GCONTXT * context)
{
  DIE_NOW(context, "CDP instruction unimplemented");
}

#ifdef CONFIG_BLOCK_COPY
u32int* mrcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "mrcPCInstruction is called but should always be critical");
  return 0;
}
#endif

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
  #ifdef CONFIG_BLOCK_COPY
  nextPC = context->PCOfLastInstruction + 4;
  #else
  nextPC = context->R15+4;
  #endif
  return nextPC;
}

#ifdef CONFIG_BLOCK_COPY
u32int* mcrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "mcr PCFunct shouldn't be launched since mcrPCInstruction is always emulated!\n");
  return 0;
}
#endif

u32int mcrInstruction(GCONTXT * context)
{
  //This function shouldn't pose a problem no inputRegisters are used
  u32int nextPC = 0;
  u32int instr = context->endOfBlockInstr;
  int instrCC = (instr >> 28) & 0xF;
  u32int cpsrCC = (context->CPSR >> 28) & 0xF;
  bool conditionMet = evalCC(instrCC, cpsrCC);
#ifdef COPROC_INSTR_TRACE
  serial_putstring("MCR instr ");
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
      u32int regSrcNr = (instr & 0x0000F000) >> 12;
      u32int srcVal = loadGuestGPR(regSrcNr, context);
      setCregVal(cRegSrc1, opcode1, cRegSrc2, opcode2, context->coprocRegBank, srcVal);
    }
    else
    {
      invalid_instruction(instr, "Unknown coprocessor number");
    }
  }
  #ifdef CONFIG_BLOCK_COPY
  nextPC = context->PCOfLastInstruction + 4;
  #else
  nextPC = context->R15+4;
  #endif
  return nextPC;
}

#ifdef CONFIG_BLOCK_COPY
u32int* stcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "stc PCFunct unfinished\n");
  return 0;
}
#endif

u32int stcInstruction(GCONTXT * context)
{
  DIE_NOW(0, "stcInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "STC instruction unimplemented");
}

#ifdef CONFIG_BLOCK_COPY
u32int* ldcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ldc PCFunct unfinished\n");
  return 0;
}
#endif

u32int ldcInstruction(GCONTXT * context)
{
  DIE_NOW(context, "LDC instruction unimplemented");
}
/* V6 coprocessor instructions.  */

#ifdef CONFIG_BLOCK_COPY
u32int* mrrc2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "mrrc2 PCFunct unfinished\n");
  return 0;
}
#endif

u32int mrrc2Instruction(GCONTXT * context)
{
  DIE_NOW(context, "MRRC2 instruction unimplemented");
}

#ifdef CONFIG_BLOCK_COPY
u32int* mcrr2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "mcrr2 PCFunct unfinished\n");
  return 0;
}
#endif

u32int mcrr2Instruction(GCONTXT * context)
{
  DIE_NOW(context, "MCRR2 instruction unimplemented");
}
/* V5 coprocessor instructions.  */

#ifdef CONFIG_BLOCK_COPY
u32int* ldc2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ldc2 PCFunct unfinished\n");
  return 0;
}
#endif

u32int ldc2Instruction(GCONTXT * context)
{
  DIE_NOW(context, "LDC2 instruction unimplemented");
}

#ifdef CONFIG_BLOCK_COPY
u32int* stc2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "stc2 PCFunct unfinished\n");
  return 0;
}
#endif

u32int stc2Instruction(GCONTXT * context)
{
  DIE_NOW(context, "STC2 instruction unimplemented");
}

#ifdef CONFIG_BLOCK_COPY
u32int* cdp2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "cdp2 PCFunct unfinished\n");
  return 0;
}
#endif

u32int cdp2Instruction(GCONTXT * context)
{
  DIE_NOW(context, "CDP2 instruction unimplemented");
}

#ifdef CONFIG_BLOCK_COPY
u32int* mcr2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "mcr2 PCFunct unfinished\n");
  return 0;
}
#endif

u32int mcr2Instruction(GCONTXT * context)
{
  DIE_NOW(context, "MCR2 instruction unimplemented");
}

#ifdef CONFIG_BLOCK_COPY
u32int* mrc2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "mrc2 PCFunct unfinished\n");
  return 0;
}
#endif

u32int mrc2Instruction(GCONTXT * context)
{
  DIE_NOW(context, "MRC2 instruction unimplemented");
}
