#include "common/debug.h"
#include "common/stddef.h"

#include "instructionEmu/decoder.h"
#include "instructionEmu/interpreter.h"

#ifdef CONFIG_BLOCK_COPY
#include "instructionEmu/translator.h"
#endif


struct decodingTable
{
  u32int mask;             /* Recognise if (instr & mask) == value.  */
  u32int value;
  struct decodingTableEntry *table;
};


#include "instructionEmu/decoder/arm/tables.inc.c"


static struct decodingTableEntry *decode(struct decodingTable *categories, u32int instruction);


static struct decodingTableEntry *decode(struct decodingTable *categories, u32int instruction)
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
  DEBUG(DECODER, "decode: instruction = %#.8x, replace = %x, handler = %p" EOL, instruction, entry->replace, entry->handler);
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
  return entry;
}

struct decodingTableEntry *decodeArmInstruction(u32int instruction)
{
  return decode(armCategories, instruction);
}
