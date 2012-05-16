#include "instructionEmu/loadStoreDecode.h"

#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/t16/storeInstructions.h"


u32int t16StrInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T16_STORE, context, instruction);

  u32int regSrc = instruction & 0x7;
  u32int regDst = (instruction & 0x38) >> 3;
  u32int imm32 = ((instruction & 0x7C0) >> 6) << 2; //extend
  u32int baseAddress = loadGuestGPR(regDst, context);
  u32int valueToStore = loadGuestGPR(regSrc, context);
  u32int offsetAddress = baseAddress + imm32;

  DEBUG(INTERPRETER_T16_STORE, "t16StrInstruction: regsrc=%x, regdst=%x, address=%#.8x, value="
      "%#.8x" EOL, regSrc, regDst, offsetAddress, valueToStore);

  vmStore(WORD, offsetAddress, valueToStore);

  return context->R15 + T16_INSTRUCTION_SIZE;
}

u32int t16StrSpInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T16_STORE, context, instruction);

  u32int regSrc = (instruction & 0x700) >> 8;
  u32int imm32 = (instruction & 0xFF) << 2; //extend
  u32int baseAddress = loadGuestGPR(GPR_SP, context);
  u32int valueToStore = loadGuestGPR(regSrc, context);
  u32int offsetAddress = baseAddress + imm32;

  DEBUG(INTERPRETER_T16_STORE, "t16StrSpInstruction: regsrc=%x, imm32=%#.8x, address=%#.8x, value="
      "%#.8x" EOL, regSrc, imm32, offsetAddress, valueToStore);

  vmStore(WORD, offsetAddress, valueToStore);

  return context->R15 + T16_INSTRUCTION_SIZE;
}

u32int t16StrbInstruction(GCONTXT * context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T16_STORE, context, instruction);
  u32int imm32;
  u32int regSrc = (instruction & 0x0007);
  u32int regDst = (instruction & 0x0038)>>3;

  if (((instruction & THUMB16_STRB_IMM5_MASK) == THUMB16_STRB_IMM5))
  {
    imm32 = (instruction & 0x07C0)>>6;
  }
  else if (((instruction & THUMB16_STRB_REG_MASK) == THUMB16_STRB_REG))
  {
    u32int regDst2 = (instruction & 0x01C0)>>6;
    imm32 = loadGuestGPR(regDst2, context);
  }
  else
  {
    DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
  }
  u32int baseAddress = loadGuestGPR(regDst, context);
  u32int offsetAddress = baseAddress + imm32;
  u32int valueToStore = loadGuestGPR(regSrc, context);

  vmStore(BYTE, offsetAddress, valueToStore & 0xFF);

  return context->R15 + T16_INSTRUCTION_SIZE;
}

u32int t16StrhInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T16_STORE, context, instruction);
  u32int regSrc = (instruction & 0x0007);
  u32int regDst = (instruction & 0x0038)>>3;
  u32int offset = 0;

  if (((instruction & THUMB16_STRH_IMM5_MASK) == THUMB16_STRH_IMM5))
  {
    offset = (instruction & 0x07C0) >> 6;
  }
  else if (((instruction & THUMB16_STRH_REG_MASK) == THUMB16_STRH_REG))
  {
    u32int regDst2 = (instruction & 0x01C0) >> 6;
    offset = loadGuestGPR(regDst2, context);
  }
  else
  {
    DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
  }

  u32int baseAddress = loadGuestGPR(regDst, context);
  u32int offsetAddress = baseAddress + offset;
  u32int valueToStore = loadGuestGPR(regSrc, context);

  vmStore(HALFWORD, offsetAddress, valueToStore & 0xFFFF);

  return context->R15 + T16_INSTRUCTION_SIZE;
}

u32int t16PushInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T16_STORE, context, instruction);

  u32int regList = ((((instruction & 0x0100) >> 8) << 15)) | (instruction & 0x00FF);
  u32int address = loadGuestGPR(GPR_SP, context);
  address -= 4; // First item 4 bytes below the Stack pointer
  // Everything has to be stored in reverse order ( page 532 ).
  // Last item has to be just below the stack pointer

  u32int valueLoaded;

  // Is LR on the List?
  if (instruction & 0x0100) //LR is on the list
  {
    valueLoaded = loadGuestGPR(GPR_LR, context);
    validateCachePreChange(context->blockCache, address);
    vmStore(WORD, address, valueLoaded);
    address -= 4;
  }

  // for i = 7 to 0. PUSH accepts only low registers
  int i = 7;
  for (i = 7; i >= 0; i--)
  {
    // if current register set
    if (((regList >> i) & 0x1) == 0x1)
    {
      valueLoaded = loadGuestGPR(i, context);
      // emulating store. Validate cache if needed
      validateCachePreChange(context->blockCache, address);
      vmStore(WORD, address, valueLoaded);
      address -= 4;
    }
  } // for ends

  //thumb always update the SP to point to the start address
  address += 4; // FIX ME -> Not very smart, is it?
  storeGuestGPR(GPR_SP, address, context);

  u32int nextPC = context->R15 + T16_INSTRUCTION_SIZE;
  DEBUG(INTERPRETER_T16_STORE, "t16PushInstruction: restore PC = %#.8x" EOL, nextPC);
  return nextPC;
}
