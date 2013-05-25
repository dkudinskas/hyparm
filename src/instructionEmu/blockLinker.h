#ifndef __INSTRUCTION_EMU__BLOCK_LINKER_H__
#define __INSTRUCTION_EMU__BLOCK_LINKER_H__

#include "common/types.h"

#include "guestManager/basicBlockStore.h"
#include "guestManager/guestContext.h"

void linkBlock(GCONTXT *context, u32int nextPC, u32int lastPC, BasicBlock* lastBlock);
void unlinkBlock(BasicBlock* block, u32int index);
void unlinkAllBlocks(GCONTXT *context);

#endif
