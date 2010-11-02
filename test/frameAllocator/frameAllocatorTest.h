#ifndef __FRAME_ALLOCATOR_TEST_H__
#define __FRAME_ALLOCATOR_TEST_H__
#include "types.h"

//Need to add a hook into the frameAllocator in order to test the frame table
u32int* getFrameAllocatorTestAddress(void);

#endif