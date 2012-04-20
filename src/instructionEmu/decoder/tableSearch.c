#include "common/debug.h"
#include "common/stddef.h"

#include "cpuArch/constants.h"

#include "instructionEmu/decoder.h"
#include "instructionEmu/interpreter.h"


struct decodingTable
{
  u32int mask;             /* Recognise if (instr & mask) == value.  */
  u32int value;
  struct decodingTableEntry *table;
};

struct decodingTableEntry
{
  instructionReplaceCode replace;
  instructionHandler handler;
  u32int value;            /* If arch == 0 then value is a sentinel.  */
  u32int mask;             /* Recognise inst if (op & mask) == value.  */
  const char *instructionString; /* How to disassemble this insn.  */
};


#include "instructionEmu/decoder/arm/tables.inc.c"

#ifdef CONFIG_THUMB2
#include "instructionEmu/decoder/t16/tables.inc.c"
#include "instructionEmu/decoder/t32/tables.inc.c"
#endif


static instructionReplaceCode decode(struct TopLevelCategory *categories, u32int instruction, instructionHandler *handler);


static instructionReplaceCode decode(struct TopLevelCategory *categories, u32int instruction, instructionHandler *handler)
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
  struct decodingTableEntry *entry = categories->table;
  if (!entry)
  {
    printf("decode: cannot classify instruction %#.8x", instruction);
    DIE_NOW(NULL, "undefined instruction");
  }
  while ((instruction & entry->mask) != entry->value)
  {
    entry++;
  }
  DEBUG(DECODER, "decode: instruction = %#.8x, replace = %x, handler = %p, instr = %s" EOL, instruction, entry->replace, entry->handler, entry->instructionString);
  /*
   * If the mask is zero at this point, we have hit the end of the decoding table. This means we
   * do not know what to do with this instruction; dump it...
   */
  if (entry->mask == 0)
  {
    printf("decode: cannot decode instruction %#.8x classified as '%s'" EOL, instruction,
        entry->instructionString);
    DIE_NOW(NULL, "undefined instruction");
  }
  *handler = entry->handler;
  return entry->replace;
}

instructionReplaceCode decodeArmInstruction(u32int instruction, instructionHandler *handler)
{
  return decode(armCategories, instruction, handler);
}


#ifdef CONFIG_THUMB2

instructionReplaceCode decodeThumbInstruction(u32int instruction, instructionHandler *handler)
{
  /*
   * For Thumb, we still need to determine which table of top-level categories to use
   */
  struct decodingTable *categories;
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
  return decode(categories, instruction, handler);
}

#endif /* CONFIG_THUMB2 */
