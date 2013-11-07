#include "perf/contextSwitchCounters.h"

void countBranch(GCONTXT *context, Instruction instr)
{
  if (instr.branch.cc == AL)
  {
    context->branchNonconditional++;
  }
  else
  {
    context->branchConditional++;
  }
  context->branchImmediate++;
  context->branchNonlink++;
}

void countBL(GCONTXT *context, Instruction instr)
{
  if (instr.branch.cc == AL)
  {
    context->branchNonconditional++;
  }
  else
  {
    context->branchConditional++;
  }
  context->branchImmediate++;
  context->branchLink++;
}
