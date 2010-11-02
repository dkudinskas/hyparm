#ifndef __FRAME_ALLOCATOR_H__
#define __FRAME_ALLOCATOR_H__

#include "types.h"
#include "memFunctions.h"

//uncomment for frameAllocator debug: #define FRAME_ALLOC_DBG


//All defines are in bytes unless otherwise stated
//knowing the size of the Hypervisor code we can better specify the location of the frame table
#define FRAME_TABLE_ENTRIES (TOTAL_MACHINE_RAM / FRAME_TABLE_CHUNK_SIZE)

//putting this on a 4KB chunk boundary makes things nice and pretty
#define FRAME_TABLE_START_ADDRESS ((HYPERVISOR_END_ADDR + FRAME_TABLE_CHUNK_SIZE-1) & ~0xFFF)


//create a frameTable, only call this once!
void initialiseFrameTable(void);

//returns 0 if free, 1 if not
u8int isFrameFree(u32int addr);

//Allocate a single frame, returns 0 if failed, otherwise the address of the allocated frame
u32int* allocFrame(u8int domain);

//Attempt to allocate multiple contiguous frames
u32int* allocMultipleFrames(u32int numFrames, u8int domain);

//Allocate a single specific frame by physical addr, returns 0 if suceeded
u32int* allocFrameAddr(u32int phyAddr);

//free a single frame, returns non-zero if failed, 0 if sucessful
u8int freeFrame(u32int phyAddr);

//free multiple frames in contiguous space, returns 0 if completed, non-zero if failed
u8int freeMultipleFrames(u32int framePtrStart, u32int framePtrEnds);

//Address manipulation
u32int addrToOffset(u32int phyAddr);
u32int* offsetToAddr(u32int offset);

//Check domain value is in range
u8int isDomainValid(u8int domain);
u32int getDomainOffset(u8int domain);
u32int getDomainMaxOffset(u8int domain);

//bitmap frame table manipulation
u8int markFrameUsed_offset(u32int offset);
u8int freeFrame_offset(u32int offset);
u8int isFrameFree_offset(u32int offset);
u32int getFreeFrames(u32int numFrames, u8int domain);

//Initialisor, no protection against this being called twice.
void newFrameAllocationTable(void);


/* Usefull for testing */
#ifdef FRAME_TABLE_ALLOC_TEST
u32int getHypervisorStartAddr(void);
u32int getHypervisorSize(void);
u32int getTotalMachineRam(void);
u32int getFrameTableChunkSize(void);
u32int getFrameTableEntries(void);
#endif


#endif