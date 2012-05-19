#include "instructionEmu/loadStoreDecode.h"

#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/t32/storeInstructions.h"


u32int t32StrbInstruction(GCONTXT * context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T32_STORE, context, instruction);

  u32int regSrc = (instruction & 0x0000F000) >> 12;
  u32int regDst = (instruction & 0x000F0000) >> 16;
  u32int imm32 = (instruction & 0x000000FF);
  u32int baseAddress = loadGuestGPR(regDst, context);
  u32int valueToStore = loadGuestGPR(regSrc, context);

  u32int preOrPost;
  u32int incOrDec;
  u32int writeBack;
  u32int offsetAddress = 0;
  u32int address = 0;

  if ((instruction & THUMB32_STRB_IMM12_MASK) == THUMB32_STRB_IMM12)
  {
    writeBack = 0;
    incOrDec = 1;
    preOrPost = 1;

    offsetAddress = baseAddress + imm32;
    //just be compatible
    address = offsetAddress;
  }
  else if ((instruction & THUMB32_STRB_IMM8_MASK) == THUMB32_STRB_IMM8)
  {
    writeBack = (instruction & 0x00000100) >> 8;
    incOrDec = (instruction & 0x00000200) >> 9;
    preOrPost = (instruction & 0x00000400) >> 10;

    if (incOrDec != 0)
    {
      offsetAddress = baseAddress + imm32;
    }
    else
    {
      offsetAddress = baseAddress - imm32;
    }
    if (preOrPost != 0)
    {
      address = baseAddress;
    }
    else
    {
      address = offsetAddress;
    }
  }
  else
  {
    DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
  }

  DEBUG(INTERPRETER_T32_STORE, "t32StrbInstruction: regsrc=%x, regdst=%x, address=%#.8x, value="
      "%#.8x, P=%x, U=%x, W=%x" EOL, regSrc, regDst, address, valueToStore, preOrPost, incOrDec,
      writeBack);

  vmStore(context, BYTE, address, valueToStore & 0xFF);

  if (writeBack)
  {
    DEBUG(INTERPRETER_T32_STORE, "t32StrbInstruction: storing %#.8x to %#.8x" EOL, address, regDst);
    storeGuestGPR(regDst, valueToStore, context);
  }

  return context->R15 + T32_INSTRUCTION_SIZE;
}

u32int t32StrhImmediateInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T32_STORE, context, instruction);

  u32int regSrc = (instruction & 0x0000F000) >> 12;
  u32int regDst = (instruction & 0x000F0000) >> 16;

  u32int imm12 = (instruction & 0x00000FFF);
  u32int baseAddress = loadGuestGPR(regDst, context);
  u32int offsetAddress = baseAddress + imm12;
  u32int valueToStore = loadGuestGPR(regSrc, context);

  vmStore(context, HALFWORD, offsetAddress, valueToStore & 0xFFFF);
  return context->R15 + T32_INSTRUCTION_SIZE;
}

u32int t32StrhRegisterInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T32_STORE, context, instruction);

  u32int regSrc = (instruction & 0x0000F000) >> 12;
  u32int regDst = (instruction & 0x000F0000) >> 16;

  u32int regDst2 = (instruction & 0x0000000F);
  u8int shift = (instruction & 0x00000030) >> 4;
  u32int baseAddress = loadGuestGPR(regDst, context);
  u32int offsetAddress = loadGuestGPR(regDst2, context);
  offsetAddress = baseAddress + (offsetAddress << shift);

  vmStore(context, HALFWORD, offsetAddress, loadGuestGPR(regSrc, context) & 0xFFFF);
  return context->R15 + T32_INSTRUCTION_SIZE;
}

u32int t32StrdImmediateInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T32_STORE, context, instruction);

  u32int prePost = (instruction & 0x01000000)>>24;
  u32int upDown = (instruction & 0x00800000)>>23;
  u32int writeback = (instruction & 0x00200000)>>21;
  u32int regSrc = (instruction & 0x0000F000)>>12;
  u32int regSrc2 = (instruction & 0x00000F00)>>8;
  u32int regDst = (instruction & 0x000F0000)>>16;
  u32int imm8 = (instruction & 0x000000FF)<<2;

  u32int baseAddress = loadGuestGPR(regDst, context);
  u32int valueToStore = loadGuestGPR(regSrc, context);
  u32int valueToStore2 = loadGuestGPR(regSrc2, context);


  u32int offsetAddress = baseAddress;


  // if 1 then increment, else decrement
  if (upDown)
  {
    offsetAddress += imm8;
  }
  else
  {
    offsetAddress -= imm8;
  }

  u32int address = prePost ? offsetAddress : baseAddress;

  DEBUG(INTERPRETER_T32_STORE, "t32StrdImmediateInstruction: store val1 = %x@%#.8x store val2 = "
      "%x@%#.8x" EOL, valueToStore, address, valueToStore2, address + 4);

  vmStore(context, WORD, address, valueToStore);
  vmStore(context, WORD, address + 4, valueToStore2);

  if (writeback)
  {
    storeGuestGPR(regDst, offsetAddress, context);
  }

  return context->R15 + T32_INSTRUCTION_SIZE;
}

u32int t32StrhtInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T32_STORE, context, instruction);
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}
