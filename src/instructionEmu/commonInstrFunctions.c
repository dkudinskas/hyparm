#include "cpuArch/constants.h"

#include "instructionEmu/commonInstrFunctions.h"


/*
 * FIXME THIS HAS NOTHING TO DO WITH THE INTERPRETER
 */

#ifdef CONFIG_THUMB2

u32int fetchThumbInstr(u16int *currhwAddress)
{
  u16int narrowInstr = *currhwAddress;
  switch (narrowInstr & THUMB32)
  {
    case THUMB32_1:
    case THUMB32_2:
    case THUMB32_3:
      /*
       * 32-bit Thumb instruction -- need to fetch next halfword.
       */
      return (narrowInstr << 16) | *++currhwAddress;
    default:
      /*
       * 16-bit Thumb instruction (?)
       * FIXME check coverage of masks
       */
      return narrowInstr;
  }
}

#endif
