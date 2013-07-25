#include "common/debug.h"
#include "cpuArch/constants.h"

#include "instructionEmu/blockLinker.h"
#include "instructionEmu/decoder/arm/structs.h"
#include "instructionEmu/scanner.h"
#include "instructionEmu/translator/translator.h"

#include "memoryManager/mmu.h"


void linkBlock(GCONTXT *context, u32int nextPC, u32int lastPC, BasicBlock* lastBlock)
{
  DEBUG(LINKER, "linkBlock: nextPC=%08x, lastPC %08x, lastBlock %p type %x"
        EOL, nextPC, lastPC, lastBlock, lastBlock->type);

  if (lastBlock->type != BB_TYPE_ARM)
  {
    // last block is not for linking
    return;
  }

//  if (lastBlock->hotness < 5)
  {
    // last block aint hot enough
//    return;
  }

  // get next block
  BlockInfo blockinfo = getBlockInfo(context->translationStore, nextPC);
  if (blockinfo.blockFound == FALSE)
  {
    // next block not scanned yet. nothing to link to.
    return;
  }
  BasicBlock* nextBlock = blockinfo.blockPtr;

  // must check wether it was the first or second hypercall
  // to do that we get the address of the last word of the block in host cache
  u32int eobInstruction = *(lastBlock->guestEnd);
  u32int lastInstrOfHostBlock = (u32int)lastBlock->codeStoreStart +
                                (lastBlock->codeStoreSize-1) * ARM_INSTRUCTION_SIZE;
  // this last word is DATA (block index). the word before is control flow
  lastInstrOfHostBlock -= ARM_INSTRUCTION_SIZE;
  u32int conditionCode = 0xE0000000;
  if (lastPC != lastInstrOfHostBlock)
  {
    conditionCode = eobInstruction & 0xF0000000;
  }

  // must try to put a branch at lastPC direct to nextPC
  putBranch(lastPC, (u32int)nextBlock->codeStoreStart, conditionCode);

  // make sure both blocks are marked as a group-block.
  lastBlock->type = GB_TYPE_ARM;
  nextBlock->type = GB_TYPE_ARM;
}


void unlinkBlock(BasicBlock* block, u32int index)
{
  DEBUG(LINKER, "unlinkBlock: block %p, index %x" EOL, block, index);

  u32int hypercall = INSTR_SWI | (index + 0x100);
  u32int lastInstrOfHostBlock = (u32int)block->codeStoreStart +
                                (block->codeStoreSize-1) * ARM_INSTRUCTION_SIZE;
  lastInstrOfHostBlock -= ARM_INSTRUCTION_SIZE;

  // remove 'last' hypercall (or the only one if there werent more)
  *(u32int*)lastInstrOfHostBlock = hypercall;
  mmuCleanDCacheByMVAtoPOU(lastInstrOfHostBlock);
  mmuInvIcacheByMVAtoPOU(lastInstrOfHostBlock);

  if (!block->oneHypercall)
  {
    // two hypercalls! fix up condition code.
    u32int condition = *(u32int*)(lastInstrOfHostBlock-ARM_INSTRUCTION_SIZE);
    condition &= 0xF0000000;
    hypercall = (hypercall & 0x0FFFFFFF) | condition;

    *(u32int*)(lastInstrOfHostBlock-ARM_INSTRUCTION_SIZE) = hypercall;
    mmuCleanDCacheByMVAtoPOU(lastInstrOfHostBlock-ARM_INSTRUCTION_SIZE);
    mmuInvIcacheByMVAtoPOU(lastInstrOfHostBlock-ARM_INSTRUCTION_SIZE);
  }
  block->type = BB_TYPE_ARM;
}


void unlinkAllBlocks(GCONTXT *context)
{
  u32int i = 0;
  /* we traverse the complete block translation store
   * inside the loop we unlink all current group-blocks. */ 
  for (i = 0; i < BASIC_BLOCK_STORE_SIZE; i++)
  {
    if (context->translationStore->basicBlockStore[i].type == GB_TYPE_ARM)
    {
      unlinkBlock(&context->translationStore->basicBlockStore[i], i);
    }
  }
}
