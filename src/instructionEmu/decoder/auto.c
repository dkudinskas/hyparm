#include "common/debug.h"
#include "common/stddef.h"

#include "cpuArch/constants.h"

#include "instructionEmu/decoder.h"
#include "instructionEmu/interpreter.h"

#include "instructionEmu/translator/arm/pcHandlers.h"


instructionReplaceCode decodeArmInstruction(u32int instruction, AnyHandler *handler)
{
#include "instructionEmu/decoder/arm/graph.inc.c"
}

#ifdef CONFIG_THUMB2

static inline __attribute__((always_inline))
  instructionReplaceCode decodeT16Instruction(u32int instruction, InstructionHandler *handler)
{
#include "instructionEmu/decoder/t16/graph.inc.c"
}

static inline __attribute__((always_inline))
  instructionReplaceCode decodeT32Instruction(u32int instruction, InstructionHandler *handler)
{
#include "instructionEmu/decoder/t32/graph.inc.c"
}

instructionReplaceCode __attribute__((flatten)) decodeThumbInstruction(u32int instruction, InstructionHandler *handler)
{
  /*
   * For Thumb, we still need to determine which table of top-level categories to use
   */
  switch(instruction & THUMB32 << 16)
  {
    case THUMB32_1 << 16:
    case THUMB32_2 << 16:
    case THUMB32_3 << 16:
      return decodeT32Instruction(instruction, handler);
    default:
      instruction &= 0x0000FFFF;
      return decodeT16Instruction(instruction, handler);
  }
}

#endif /* CONFIG_THUMB2 */
