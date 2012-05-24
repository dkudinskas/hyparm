#include "instructionEmu/loadStoreDecode.h"

#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/t16/loadInstructions.h"


u32int t16LdrInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T16_LOAD, context, instruction);

  u32int regDst;
  u32int offsetAddress;
  u32int baseAddress;
  u32int imm32;

  if( ( instruction & THUMB16_LDR_IMM5_MASK ) == THUMB16_LDR_IMM5 )
  {
    imm32 = ( instruction & 0x07C0 )>>6;
    u32int regSrc = ( instruction & 0x0038 )>>3;
    regDst = ( instruction & 0x0007 );
    baseAddress = getLowGPRegister(context, regSrc);
    offsetAddress = baseAddress + imm32;
  }
  else if ( ( instruction & THUMB16_LDR_IMM8_MASK ) == THUMB16_LDR_IMM8 )
  {
    imm32 = ( instruction & 0x00FF );
    // Source register is SP
    regDst = ( instruction & 0x0700 )>>8;
    baseAddress = getGPRegister(context, GPR_SP);
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
    u32int regSrc = ( instruction & 0x0038 )>>3;
    regDst = ( instruction & 0x0003 );
    baseAddress = getLowGPRegister(context, regSrc);
    imm32 = getLowGPRegister(context, regSrc2);
    offsetAddress = baseAddress + imm32;
  }
  else
  {
    DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
  }

  setGPRegister(context, regDst, vmLoad(context, WORD, offsetAddress));
  return context->R15 + T16_INSTRUCTION_SIZE;
}

u32int t16LdrbInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T16_LOAD, context, instruction);

  u32int regSrc = (instruction & 0x0038)>>3;
  u32int regDst  = (instruction & 0x0007);
  u32int offset = 0;

  if( ((instruction & THUMB16_LDRB_IMM5_MASK) == THUMB16_LDRB_IMM5))
  {
    offset = (instruction & 0x07C0)>>6;
  }
  else if (((instruction & THUMB16_LDRB_REG_MASK) == THUMB16_LDRB_REG))
  {
    u32int regSrc2 = (instruction & 0x01C)>>6;
    offset = getLowGPRegister(context, regSrc2);
  }
  else
  {
    DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
  }

  u32int baseAddress = getLowGPRegister(context, regSrc);
  u32int offsetAddress = baseAddress + offset;
  setGPRegister(context, regDst, vmLoad(context, BYTE, offsetAddress) & 0xFF);
  return context->R15 + T16_INSTRUCTION_SIZE;
}

u32int t16LdrhImmediateInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T16_LOAD, context, instruction);
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int t16LdmInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T16_LOAD, context, instruction);

  u32int nextPC;
  u32int regList = 0;
  u32int baseAddress = 0;

  int i;
  // we trapped from Thumb mode. I assume the PC reg is in the list
  ASSERT((instruction & 0x100) != 0, "trapped but PC is not on the list...");

  regList = (((instruction & 0x0100) >> 8) << 15) | (instruction & 0x00FF);

  // Get baseAddress from SP
  baseAddress = getGPRegister(context, GPR_SP);
  // for i = 0 to 7. POP accepts only low registers
  for (i = 7; i >= 0; i--)
  {
    // if current register set
    if (((regList >> i) & 0x1) == 0x1)
    {
      setGPRegister(context, i, vmLoad(context, WORD, baseAddress));
      baseAddress = baseAddress + 4;
    }
  } // for ends

  // and now take care of the PC
  nextPC = vmLoad(context, WORD, baseAddress);
  baseAddress += 4;

  // thumb always updates the SP
  setGPRegister(context, GPR_SP, baseAddress);

  /*
   * Return to ARM mode if the LSB is not set; also make sure the target address is word-aligned.
   */
  if (nextPC & 1)
  {
    nextPC ^= 1;
  }
  else if (!(nextPC & 2))
  {
    context->CPSR ^= PSR_T_BIT;
  }
  else
  {
    DIE_NOW(context, "unpredictable branch to unaligned ARM address");
  }

  return nextPC;
}
