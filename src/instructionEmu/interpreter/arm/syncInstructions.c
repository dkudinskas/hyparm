#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/arm/syncInstructions.h"


u32int armClrexInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_ARM_SYNC, context, instruction);
  DEBUG(INTERPRETER_ARM_SYNC, "ignored!");

  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armLdrexInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  DEBUG_TRACE(INTERPRETER_ARM_LOAD_SYNC, context, instruction);

  u32int baseReg = (instruction & 0x000F0000) >> 16;
  u32int regDest = (instruction & 0x0000F000) >> 12;

  if (baseReg == GPR_PC || regDest == GPR_PC)
  {
    DIE_NOW(context, "unpredictable case (PC used).");
  }

  u32int baseVal = loadGuestGPR(baseReg, context);
  u32int value = vmLoad(WORD, baseVal);

  DEBUG(INTERPRETER_ARM_LOAD_SYNC, "armLdrexInstruction: baseVal = %#.8x loaded %#.8x store to "
      "%#.8x" EOL, baseVal, value, regDest);

  storeGuestGPR(regDest, value, context);

  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armLdrexbInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  DEBUG_TRACE(INTERPRETER_ARM_LOAD_SYNC, context, instruction);

  u32int baseReg = (instruction & 0x000F0000) >> 16;
  u32int regDest = (instruction & 0x0000F000) >> 12;

  if (baseReg == GPR_PC || regDest == GPR_PC)
  {
    DIE_NOW(context, "unpredictable case (PC used).");
  }

  u32int baseVal = loadGuestGPR(baseReg, context);
  // byte zero extended to word...
  u32int value = vmLoad(BYTE, baseVal) & 0xFF;
  storeGuestGPR(regDest, value, context);

  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armLdrexhInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  DEBUG_TRACE(INTERPRETER_ARM_LOAD_SYNC, context, instruction);

  u32int baseReg = (instruction & 0x000F0000) >> 16;
  u32int regDest = (instruction & 0x0000F000) >> 12;

  if (baseReg == GPR_PC || regDest == GPR_PC)
  {
    DIE_NOW(context, "unpredictable case (PC used).");
  }
  u32int baseVal = loadGuestGPR(baseReg, context);
  // halfword zero extended to word...
  u32int value = vmLoad(HALFWORD, baseVal) & 0xFFFF;
  storeGuestGPR(regDest, value, context);

  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armLdrexdInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  DEBUG_TRACE(INTERPRETER_ARM_LOAD_SYNC, context, instruction);

  u32int baseReg = (instruction & 0x000F0000) >> 16;
  u32int regDest = (instruction & 0x0000F000) >> 12;

  // must not be PC, destination must be even and not link register
  if ((baseReg == GPR_PC) || ((regDest % 2) != 0) || (regDest == GPR_LR))
  {
    DIE_NOW(context, "unpredictable case (invalid registers).");
  }
  u32int baseVal = loadGuestGPR(baseReg, context);

  u32int value = vmLoad(WORD, baseVal);
  u32int value2 = vmLoad(WORD, baseVal+4);
  storeGuestGPR(regDest, value, context);
  storeGuestGPR(regDest + 1, value2, context);

  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armStrexInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  DEBUG_TRACE(INTERPRETER_ARM_STORE_SYNC, context, instruction);

  u32int regN = (instruction & 0x000F0000) >> 16;
  u32int regD = (instruction & 0x0000F000) >> 12;
  u32int regT = (instruction & 0x0000000F);

  if ((regN == GPR_PC) || (regD == GPR_PC) || (regT == GPR_PC))
  {
    DIE_NOW(context, "unpredictable case (PC used)");
  }
  if ((regD == regN) || (regD == regT))
  {
    DIE_NOW(context, "unpredictable case (invalid register use)");
  }

  u32int address = loadGuestGPR(regN, context);
  u32int valToStore = loadGuestGPR(regT, context);
  DEBUG(INTERPRETER_ARM_STORE_SYNC, "armStrexInstruction: address = %#.8x, valToStore = %#.8x, "
      "valueFromReg %#.8x" EOL, address, valToStore, regT);

  vmStore(WORD, address, valToStore);
  // operation succeeded updating memory, flag regD (0 - updated, 1 - fail)
  storeGuestGPR(regD, 0, context);

  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armStrexbInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  DEBUG_TRACE(INTERPRETER_ARM_STORE_SYNC, context, instruction);

  u32int regN = (instruction & 0x000F0000) >> 16;
  u32int regD = (instruction & 0x0000F000) >> 12;
  u32int regT = (instruction & 0x0000000F);

  if ((regN == GPR_PC) || (regD == GPR_PC) || (regT == GPR_PC))
  {
    DIE_NOW(context, "unpredictable case (PC used)");
  }
  if ((regD == regN) || (regD == regT))
  {
    DIE_NOW(context, "unpredictable case (invalid register use)");
  }

  u32int address = loadGuestGPR(regN, context);

  u32int valToStore = loadGuestGPR(regT, context);
  vmStore(BYTE, address, valToStore & 0xFF);

  storeGuestGPR(regD, 0, context);

  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armStrexhInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  DEBUG_TRACE(INTERPRETER_ARM_STORE_SYNC, context, instruction);

  u32int condcode = (instruction & 0xF0000000) >> 28;
  u32int regN = (instruction & 0x000F0000) >> 16;
  u32int regD = (instruction & 0x0000F000) >> 12;
  u32int regT = (instruction & 0x0000000F);

  if (!evaluateConditionCode(context, condcode))
  {
    // condition not met! allright, we're done here. next instruction...
    return context->R15 + 4;
  }

  if ((regN == 15) || (regD == 15) || (regT == 15))
  {
    DIE_NOW(context, "unpredictable case (PC used)");
  }
  if ((regD == regN) || (regD == regT))
  {
    DIE_NOW(context, "unpredictable case (invalid register use)");
  }

  u32int address = loadGuestGPR(regN, context);

  u32int valToStore = loadGuestGPR(regT, context);
  vmStore(HALFWORD, address, valToStore & 0xFFFF);
  storeGuestGPR(regD, 0, context);

  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armStrexdInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  DEBUG_TRACE(INTERPRETER_ARM_STORE_SYNC, context, instruction);

  u32int condcode = (instruction & 0xF0000000) >> 28;
  u32int regN = (instruction & 0x000F0000) >> 16;
  u32int regD = (instruction & 0x0000F000) >> 12;
  u32int regT = (instruction & 0x0000000F);

  if (!evaluateConditionCode(context, condcode))
  {
    // condition not met! allright, we're done here. next instruction...
    return context->R15 + 4;
  }

  if (regD == GPR_PC || (regT % 2) || regT == GPR_LR || regN == GPR_PC || regD == regN
      || regD == regT || regD == (regT + 1))
  {
    DIE_NOW(context, "unpredictable");
  }

  u32int address = loadGuestGPR(regN, context);

  // Create doubleword to store such that R[t] will be stored at addr and R[t2] at addr+4.
  u32int valToStore1 = loadGuestGPR(regT, context);
  u32int valToStore2 = loadGuestGPR(regT + 1, context);

  /*
   * FIXME STREXD: assuming littl endian
   */
  DIE_NOW(context, "assuming littlendian!");

  bool littleEndian = TRUE;
  if (littleEndian)
  {
    vmStore(WORD, address, valToStore2);
    vmStore(WORD, address+4, valToStore1);
  }
  else
  {
    vmStore(WORD, address, valToStore1);
    vmStore(WORD, address+4, valToStore2);
  }
  storeGuestGPR(regD, 0, context);

  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armSwpInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "not implemented");
}
