#include "common/debug.h"

#include "instructionEmu/coprocInstructions.h"
#include "instructionEmu/dataProcessInstr.h"
#include "instructionEmu/decoder.h"
#include "instructionEmu/miscInstructions.h"

#include "instructionEmu/interpreter/branchInstructions.h"
#include "instructionEmu/interpreter/loadInstructions.h"
#include "instructionEmu/interpreter/storeInstructions.h"


instructionHandler decodeArmInstruction(u32int instruction)
{
#include "instructionEmu/decoder/armGraph.inc.c"
  DIE_NOW(0, "decodeArmInstruction: control fell through");
}

#ifdef CONFIG_THUMB2

static inline __attribute__((always_inline))
  instructionHandler decodeT16Instruction(u32int instruction)
{
#include "instructionEmu/decoder/t16Graph.inc.c"
  DIE_NOW(0, "decodeT16Instruction: control fell through");
}

static inline __attribute__((always_inline))
  instructionHandler decodeT32Instruction(u32int instruction)
{
#include "instructionEmu/decoder/t32Graph.inc.c"
  DIE_NOW(0, "decodeT32Instruction: control fell through");
}

instructionHandler __attribute__((flatten)) decodeThumbInstruction(u32int instruction)
{
  /*
   * For Thumb, we still need to determine which table of top-level categories to use
   */
  switch(instruction & THUMB32 << 16)
  {
    case THUMB32_1 << 16:
    case THUMB32_2 << 16:
    case THUMB32_3 << 16:
      return decodeT32Instruction(instruction);
    default:
      instruction &= 0x0000FFFF;
      return decodeT16Instruction(instruction);
  }
}

#endif /* CONFIG_THUMB2 */
