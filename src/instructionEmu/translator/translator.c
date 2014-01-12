#include "cpuArch/constants.h"

#include "common/debug.h"
#include "common/linker.h"

#include "instructionEmu/blockLinker.h"
#include "instructionEmu/decoder/arm/structs.h"
#include "instructionEmu/scanner.h"
#include "instructionEmu/translator/translator.h"

#include "memoryManager/mmu.h"


void putBranch(u32int branchLocation, u32int branchTarget, u32int condition)
{
  u32int offset = branchTarget - (branchLocation + ARM_INSTRUCTION_SIZE*2);
  offset = (offset >> 2) & 0xFFFFFF;
  u32int branchInstruction = condition | BRANCH_BASE_VALUE | offset;
  *(u32int*)branchLocation = branchInstruction;

  mmuCleanDCacheByMVAtoPOU(branchLocation);
  mmuInvIcacheByMVAtoPOU(branchLocation);
}


bool isBranch(Instruction instr)
{
  return ((instr.branch.opc0) == 0x5) ? TRUE : FALSE;
}


bool branchLinks(Instruction instr)
{
  return instr.branch.link;
}

bool isServiceCall(Instruction instr)
{
  return ((instr.svc.opc0 == 0xf) && (instr.svc.cc != NV));
}


bool isConditional(Instruction instr)
{
  // can use any encoding
  return (instr.branch.cc != AL);
}



u32int findBlockIndexNumber(GCONTXT *context, u32int hostPC)
{
  bool found = FALSE;
  u32int* pc = (u32int*)hostPC;
  u32int index = 0;
  Instruction instr;
  while (!found)
  {
    instr.raw = *pc;
    if (isServiceCall(instr))
    {
      // lets get the code from the service call.
      index = instr.svc.imm24 - 0x100;
      found = TRUE;
    }
    else if (isBranch(instr))
    {
      // found our first branch. skip it.
      pc++;
      instr.raw = *pc;

      // one branch down. lets see next one. MUST be branch, svc or the index
      if (isBranch(instr))
      {
        // second branch! next word is the index 4 sure
        pc++;
        index = *pc;
      }
      else if (isServiceCall(instr))
      {
        // ok, branch followed by hypecall. get the code from it THIS TIME
        index = instr.svc.imm24 - 0x100;
      }
      else
      {
        // since we had one branch, and the next instruction is not a branch or hypercall
        // by the power of deduction and logical reasoning, we found the code.
        index = instr.raw;
      }
      // now we really found it.
      found = TRUE;
    } // found first branch ends
    // instr was not a branch neither a hypercall or undefined call
    pc++;
  }
  return index;
}


u32int hostpcToGuestpc(GCONTXT* context)
{
  u32int index = context->lastEntryBlockIndex;
  BasicBlock* block = getBasicBlockStoreEntry(context->translationStore, context->lastEntryBlockIndex);

  // this value we are trying to map
  u32int hostPC = context->R15;

  if (block->type == GB_TYPE_ARM)
  {
    // we are in group block! get the REAL block index number.
    // and unlink. will make life easier.
    index = findBlockIndexNumber(context, context->R15);
    block = getBasicBlockStoreEntry(context->translationStore, index);
    unlinkBlock(block, index);
  }

  // if we are at the first instruction of code store block, we really know the mapping
  if (hostPC == (u32int)block->codeStoreStart)
  {
    return (u32int)block->guestStart;
  }
  else
  {
    // well, there's work to do. lets rescan the block to find PC mapping
    return rescanBlock(context, index, block, hostPC);
  }
}
