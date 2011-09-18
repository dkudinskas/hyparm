#include "common/debug.h"
#include "common/stddef.h"

#include "instructionEmu/asm-dis.h"
#include "instructionEmu/decoder.h"
#include "instructionEmu/coprocInstructions.h"
#include "instructionEmu/dataMoveInstr.h"
#include "instructionEmu/dataProcessInstr.h"
#include "instructionEmu/miscInstructions.h"


struct TopLevelCategory
{
  u32int mask;             /* Recognise if (instr & mask) == value.  */
  u32int value;
  struct instruction32bit *table;
};

struct instruction32bit
{
  s16int replace;
  instructionHandler handlerFunction;
  u32int value;            /* If arch == 0 then value is a sentinel.  */
  u32int mask;             /* Recognise inst if (op & mask) == value.  */
  const char * instructionString; /* How to disassemble this insn.  */
};


#include "decoder/armTables.inc.c"

#ifdef CONFIG_THUMB2
#include "decoder/t16Tables.inc.c"
#include "decoder/t32Tables.inc.c"
#endif


static instructionHandler decode(GCONTXT *context, struct TopLevelCategory *categories, u32int instruction);


static instructionHandler decode(GCONTXT *context, struct TopLevelCategory *categories, u32int instruction)
{
  /*
   * Find the top level category for this instruction
   */
  while ((instruction & categories->mask) != categories->value)
  {
    categories++;
  }
  /*
   * Get the decoding table for this category and decode the instruction
   */
  struct instruction32bit *table = categories->table;
  if (!table)
  {
    DIE_NOW(0, "decoder: UNDEFINED category");
  }
  while ((instruction & table->mask) != table->value)
  {
    table++;
  }
  /*
   * If the mask is zero at this point, we have hit the end of the decoding table. This means we
   * do not know what to do with this instruction; dump it...
   */
  if (table->mask == 0)
  {
    printf("%s: Instruction: %.8x ", table->instructionString, instruction);
#ifdef CONFIG_THUMB2
    dumpInstrString(context, instruction);
#else
    dumpInstrString(instruction);
#endif
    printf(EOL);
  }
  return table->replace ? table->handlerFunction : NULL;
}

instructionHandler decodeArmInstruction(GCONTXT *context, u32int instruction)
{
  return decode(context, armCategories, instruction);
}


#ifdef CONFIG_THUMB2

instructionHandler decodeThumbInstruction(GCONTXT *context, u32int instruction)
{
  /*
   * For Thumb, we still need to determine which table of top-level categories to use
   */
  struct TopLevelCategory *categories;
  switch(instruction & THUMB32 << 16)
  {
    case THUMB32_1 << 16:
    case THUMB32_2 << 16:
    case THUMB32_3 << 16:
      categories = t32Categories;
      break;
    default:
      instruction &= 0x0000FFFF;
      categories = t16Categories;
      break;
  }
  return decode(context, categories, instruction);
}

#endif /* CONFIG_THUMB2 */
