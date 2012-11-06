#include "instructionEmu/translator/translator.h"

#include "common/debug.h"

void translate(GCONTXT* context, BasicBlock* block, DecodedInstruction* decoding, u32int instruction)
{
  switch (decoding->code)
  {
    case IRC_SAFE:
    {
      // no translation required
      addInstructionToBlock(context->translationStore, block, instruction);
      break;
    }
    default:
    {
      DIE_NOW(context, "Unknown instruction decode code.\n");
    }
  }
}
