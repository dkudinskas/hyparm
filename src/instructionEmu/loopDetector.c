#include "common/debug.h"

#include "instructionEmu/loopDetector.h"


/*
 * Detect single block loops in the guest.
 *
 * Ideally, we can wrap indices in the cache around with a bitwise AND operation, but then we need
 * to make sure that the size of the block cache is a power of two.
 */

static inline s32int wrapBlockTraceIndex(s32int index);


void runLoopDetector(GCONTXT *context)
{
  u32int currentBlock = context->blockTrace[context->blockTraceIndex];
  u32int lastBlock = context->blockTrace[wrapBlockTraceIndex((s32int)context->blockTraceIndex - 1)];

  if (currentBlock == lastBlock)
  {
    ++context->loopDetectorLoopCount;
    if (context->loopDetectorLoopCount > context->loopDetectorNextTreshold)
    {
      printf("Warning: single block loop in guest at %#.8x exceeds %#x iterations" EOL
          EOL, currentBlock, context->loopDetectorLoopCount);
      context->loopDetectorNextTreshold += CONFIG_LOOP_DETECTOR_WARN_TRESHOLD;
    }
    if (context->loopDetectorLoopCount > CONFIG_LOOP_DETECTOR_DIE_TRESHOLD)
    {
      printf("Error: single block loop in guest at %#.8x exceeds maximum number of iterations (%#x)"
          EOL, currentBlock, CONFIG_LOOP_DETECTOR_DIE_TRESHOLD);
      DIE_NOW(context, "maximum number of iterations exceeded");
    }
  }
  else
  {
    resetLoopDetector(context);
  }
}

void resetLoopDetector(GCONTXT *context)
{
  context->loopDetectorLoopCount = 0;
  context->loopDetectorNextTreshold = CONFIG_LOOP_DETECTOR_WARN_TRESHOLD;
}

static inline s32int wrapBlockTraceIndex(s32int index)
{
  if (unlikely(index > CONFIG_GUEST_CONTEXT_BLOCK_TRACE_SIZE))
  {
    do
    {
      index -= CONFIG_GUEST_CONTEXT_BLOCK_TRACE_SIZE;
    }
    while (unlikely(index > CONFIG_GUEST_CONTEXT_BLOCK_TRACE_SIZE));
  }
  else if (unlikely(index < 0))
  {
    do
    {
      index += CONFIG_GUEST_CONTEXT_BLOCK_TRACE_SIZE;
    }
    while (unlikely(index < 0));
  }
  return index;
}
