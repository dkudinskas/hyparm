#include "common/debug.h"

#include "cpuArch/constants.h"

#include "instructionEmu/commonInstrFunctions.h"

#include "instructionEmu/interpreter/branchInstructions.h"


u32int t32BImmediate17Instruction(GCONTXT *context, u32int instruction)
{
  /*
   * NOTE: this Thumb instruction contains a condition code field!
   */
  if (!evaluateConditionCode(context, (instruction & 0x03c00000) >> 20))
  {
    /*
     * FIXME: why T16?
     */
    return context->R15 + T16_INSTRUCTION_SIZE;
  }
  u32int sign = (instruction & 0x04000000) >> 6;
  u32int bitJ1 = (instruction & 0x00002000) << 5;
  u32int bitJ2 = (instruction & 0x00000800) << 8;
  u32int offset = sign | bitJ2 | bitJ1
      | /* imm6 */  ((instruction & 0x003F0000) >> 4)
      | /* imm11 */ ((instruction & 0x000007FF) << 1);
  /*
   * Sign extend 20-bit offset to 32-bit
   */
  if (sign)
  {
    offset |= 0xFFE00000;
  }
  /*
   * FIXME: why T16?
   */
  return context->R15 + T16_INSTRUCTION_SIZE + offset;
}

u32int t32BImmediate21Instruction(GCONTXT *context, u32int instruction)
{
  u32int sign = (instruction & 0x04000000) >> 2;
  u32int bitI1 = (instruction & 0x00002000) << 11;
  u32int bitI2 = (instruction & 0x00000800) << 13;
  bitI1 = ((~(bitI1 ^ sign)) & 0x01000000) >> 1;
  bitI2 = ((~(bitI2 ^ sign)) & 0x01000000) >> 2;
  u32int offset = sign | bitI1 | bitI2
      | /* imm10 */ ((instruction & 0x03FF0000) >> 4)
      | /* imm11 */ ((instruction & 0x000007FF) << 1);
  /*
   * Sign extend 24-bit offset to 32-bit
   */
  if (sign)
  {
    offset |= 0xFF000000;
  }
  /*
   * FIXME: why T16?
   */
  return context->R15 + T16_INSTRUCTION_SIZE + offset;
}

u32int t32BlInstruction(GCONTXT *context, u32int instruction)
{
  u32int sign = (instruction & 0x04000000) >> 2;
  u32int bitI1 = (instruction & 0x00002000) << 11;
  u32int bitI2 = (instruction & 0x00000800) << 13;
  bitI1 = ((~(bitI1 ^ sign)) & 0x01000000) >> 1;
  bitI2 = ((~(bitI2 ^ sign)) & 0x01000000) >> 2;
  u32int offset = sign | bitI1 | bitI2
      | /* imm10 */ ((instruction & 0x03FF0000) >> 4)
      | /* imm11 */ ((instruction & 0x000007FF) << 1);
  /*
   * Sign extend 24-bit offset to 32-bit
   */
  if (sign)
  {
    offset |= 0xFF000000;
  }
  /*
   * FIXME: why T16?
   */
  u32int currentPC = context->R15 + T16_INSTRUCTION_SIZE;
  storeGuestGPR(14, currentPC | 1, context);
  return currentPC + offset;
}

u32int t32BlxImmediateInstruction(GCONTXT *context, u32int instruction)
{
  /*
   * NOTE: this instruction always switches to ARM mode.
   */
  u32int sign = (instruction & 0x04000000) >> 2;
  u32int bitI1 = (instruction & 0x00002000) << 11;
  u32int bitI2 = (instruction & 0x00000800) << 13;
  bitI1 = ((~(bitI1 ^ sign)) & 0x01000000) >> 1;
  bitI2 = ((~(bitI2 ^ sign)) & 0x01000000) >> 2;
  u32int offset = sign | bitI1 | bitI2
      /* imm10H */ | (instruction & 0x03FF0000) >> 4
      /* imm10L */ | (instruction & 0x000007FE) << 1;
  /*
   * Sign extend 23-bit offset to 32-bit
   */
  if (sign)
  {
    offset |= 0xFF800000;
  }
  /*
   * Switch to ARM mode
   */
  context->CPSR ^= PSR_T_BIT;
  /*
   * In Thumb-32, R15 points to the first halfword, so LR must be 4+1(T) bytes ahead
   */
  u32int currPC = context->R15 + T16_INSTRUCTION_SIZE;
  storeGuestGPR(GPR_LR, currPC + 1, context);
  /*
   * Make sure to return a word-aligned address
   */
  return (currPC & ~2) + offset;
}
