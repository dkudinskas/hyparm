#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/t16/miscInstructions.h"


u32int t16ItInstruction(GCONTXT *context, u32int instruction)
{
  // Get ITSTATE from instruction
  u8int ITSTATE = instruction & 0xFF;
  u32int offset = 0;
  u16int* end = (u16int *) context->R15;
  end++;

  /*
   * ITAdvance()
   * Instructions that will not be executed are immediately skipped.
   */
  while (!evaluateConditionCode(context, (ITSTATE & 0xF0) >> 4))
  {
    u32int instruction = *end;
    if ((ITSTATE & 0x7) == 0)
    {
      ITSTATE = 0;
    }
    else
    {
      ITSTATE = (ITSTATE & 0xE0) | ((ITSTATE << 1) & 0x1F);
    }

    switch (instruction & THUMB32)
    {
      case THUMB32_1:
      case THUMB32_2:
      case THUMB32_3:
        end++;
        break;
    }
    end++;
  }

  // Update ITSTATE in CPSR
  context->CPSR = (context->CPSR & ~(PSR_ITSTATE_7_2 | PSR_ITSTATE_1_0));
  context->CPSR = context->CPSR | ((ITSTATE & 0xFC) << 8) | ((ITSTATE & 0x3) << 25);

  return (u32int) end;
}

u32int t16UxtbInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "not implemented");
}

u32int t16UxthInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "not implemented");
}

u32int t16BkptInstruction(GCONTXT *context, u32int instruction)
{
#ifdef CONFIG_GUEST_TEST
  u32int val = instruction & 0x00FF;

  evalBkptVal(context, val);
#else
  DIE_NOW(context, "not implemented");
#endif
}
