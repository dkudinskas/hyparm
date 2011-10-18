#ifndef __INSTRUCTION_EMU__COMMON_INSTR_FUNCTIONS_H_
#define __INSTRUCTION_EMU__COMMON_INSTR_FUNCTIONS_H_

#include "common/types.h"

#ifdef CONFIG_THUMB2

// decode thumb instruction into 32-bit format
u32int fetchThumbInstr(u16int * address);

#endif /* CONFIG_THUMB2 */

#endif
