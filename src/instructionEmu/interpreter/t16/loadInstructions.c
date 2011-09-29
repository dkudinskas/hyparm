#include "common/debug.h"

#include "cpuArch/constants.h"

#include "instructionEmu/commonInstrFunctions.h"

#include "instructionEmu/interpreter/loadInstructions.h"

#include "memoryManager/globalMemoryMapper.h"


u32int t16LdrInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG(INTERPRETER_T16_LOAD, "t16LdrInstruction: %#.4x @ %#.8x" EOL, instruction, context->R15);

  u32int regSrc;
  u32int regDst;
  u32int offsetAddress;
  u32int baseAddress;
  u32int imm32;

  if( ( instruction & THUMB16_LDR_IMM5_MASK ) == THUMB16_LDR_IMM5 )
  {
    imm32 = ( instruction & 0x07C0 )>>6;
    regSrc = ( instruction & 0x0038 )>>3;
    regDst = ( instruction & 0x0007 );
    baseAddress = loadGuestGPR(regSrc,context);
    offsetAddress = baseAddress + imm32;
  }
  else if ( ( instruction & THUMB16_LDR_IMM8_MASK ) == THUMB16_LDR_IMM8 )
  {
    imm32 = ( instruction & 0x00FF );
    // Source register is SP
    regSrc = 0xD;
    regDst = ( instruction & 0x0700 )>>8;
    baseAddress = loadGuestGPR(regSrc,context);
    offsetAddress = baseAddress + imm32;
  }
  else if ( ( instruction & THUMB16_LDR_IMM8_LIT_MASK ) == THUMB16_LDR_IMM8_LIT )
  {
    imm32 = ( instruction & 0x00FF );
    regDst = ( instruction & 0x0700 )>>8;
    offsetAddress = imm32;
  }
  else if ( ( instruction & THUMB16_LDR_REG_MASK ) == THUMB16_LDR_REG )
  {
    u32int regSrc2 = ( instruction & 0x01C0 )>>6;
    regSrc = ( instruction & 0x0038 )>>3;
    regDst = ( instruction & 0x0003 );
    baseAddress = loadGuestGPR(regSrc, context);
    imm32 = loadGuestGPR(regSrc2, context);
    offsetAddress = baseAddress + imm32;
  }
  else
  {
    DIE_NOW(0, "Unimplemented thumb16 LDR instr");
  }

  u32int valueLoaded = context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD,
      offsetAddress);
  storeGuestGPR(regDst, valueLoaded, context);
  return context->R15 + T16_INSTRUCTION_SIZE;
}

u32int t16LdrbInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG(INTERPRETER_T16_LOAD, "t16LdrbInstruction: %#.4x @ %#.8x" EOL, instruction, context->R15);

  u32int regSrc = (instruction & 0x0078)>>3;
  u32int regDst  = (instruction & 0x0007);
  u32int offset = 0;

  if( ((instruction & THUMB16_LDRB_IMM5_MASK) == THUMB16_LDRB_IMM5))
  {
    offset = (instruction & 0x07C0)>>6;
  }
  else if (((instruction & THUMB16_LDRB_REG_MASK) == THUMB16_LDRB_REG))
  {
    u32int regSrc2 = (instruction & 0x01C)>>6;
    offset = loadGuestGPR(regSrc2, context);
  }
  else
  {
    DIE_NOW(context, "Unimplemented Thumb-16 LDRB instruction");
  }

  u32int baseAddress = loadGuestGPR(regSrc, context);
  u32int offsetAddress = baseAddress + offset;
  u32int valueLoaded = context->hardwareLibrary->loadFunction(context->hardwareLibrary, BYTE, offsetAddress) & 0xFF;
  storeGuestGPR(regDst, valueLoaded, context);
  return context->R15 + T16_INSTRUCTION_SIZE;
}

u32int t16LdrhImmediateInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG(INTERPRETER_T16_LOAD, "t16LdrhImmediateInstruction: %#.4x @ %#.8x" EOL, instruction,
      context->R15);

  DIE_NOW(context, "t16LdrhImmediateInstruction not implemented");
}

u32int t16LdmInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG(INTERPRETER_T16_LOAD, "t16LdmInstruction: %#.4x @ %#.8x" EOL, instruction, context->R15);

  u32int baseReg = 0;
  u32int regList = 0;
  u32int baseAddress = 0;

  u32int valueLoaded = 0;

  int i;
  // we trapped from Thumb mode. I assume the PC reg is in the list
  if((instruction & 0x100) == 0)
  {
    DIE_NOW(0,"Thumb POP instruction trapped but PC is not on the list...");
  }
  regList = ( ((instruction & 0x0100)>>8) << 15 ) | (instruction & 0x00FF);
  baseReg = 0xD; // hardcode SP register
  baseAddress = loadGuestGPR(baseReg, context);
  // for i = 0 to 7. POP accepts only low registers
  for (i = 7; i >= 0; i--)
  {
    // if current register set
    if ( ((regList >> i) & 0x1) == 0x1)
    {
      valueLoaded = context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, baseAddress);
      storeGuestGPR(i, valueLoaded, context);
      baseAddress = baseAddress + 4;
    }
  } // for ends
  // and now take care of the PC
  //thumb always update the SP
  if( ( instruction & 0x0100))
  {
    valueLoaded = context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, baseAddress);
    storeGuestGPR(0xF, valueLoaded, context);
    baseAddress += 4;
  }

  storeGuestGPR(baseReg, baseAddress, context);
  if (!(context->R15 & 0x1)) // In which mode are we returning to?
  {
    context->CPSR &= ~PSR_T_BIT;
  }
  context->R15 &= ~0x1;
  return context->R15;
}
