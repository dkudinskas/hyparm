#include "common/debug.h"
#include "common/stddef.h"

#include "cpuArch/constants.h"

#include "instructionEmu/decoder.h"
#include "instructionEmu/interpreter.h"

#include "instructionEmu/decoder/arm/tables.inc.c"

#ifdef CONFIG_THUMB2
#include "instructionEmu/decoder/t16/tables.inc.c"
#include "instructionEmu/decoder/t32/tables.inc.c"
#endif


static armInstruction* decode(struct TopLevelCategory *categories, u32int instruction)
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
  armInstruction *table = categories->table;
  if (!table)
  {
    printf("decode: cannot classify instruction %#.8x", instruction);
    DIE_NOW(NULL, "undefined instruction");
  }
  while ((instruction & table->mask) != table->value)
  {
    table++;
  }
  DEBUG(DECODER, "decode: instruction = %#.8x, replace = %x, handler = %p" EOL, instruction, table->replace, table->handler);
  /*
   * If the mask is zero at this point, we have hit the end of the decoding table. This means we
   * do not know what to do with this instruction; dump it...
   */
  if (table->mask == 0)
  {
    printf("decode: cannot decode instruction %#.8x classified as '%s'" EOL, instruction,
        table->instructionString);
    DIE_NOW(NULL, "undefined instruction");
  }
  return table;
}

armInstruction* decodeArmInstruction(u32int instruction)
{
  return decode(armCategories, instruction);
}


#ifdef CONFIG_THUMB2

instructionHandler decodeThumbInstruction(u32int instruction)
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
  return decode(categories, instruction);
}

#endif /* CONFIG_THUMB2 */
