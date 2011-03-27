#ifndef __MEMORY_MANAGER__MEMORY_CONSTANTS_H__
#define __MEMORY_MANAGER__MEMORY_CONSTANTS_H__

/* These constants should be sourced from somewhere more appropriate
   The hypervisor size should be calculated using symbols
*/
#define MEMORY_START_ADDR 0x80000000
#define HYPERVISOR_START_ADDR 0x8C000000
#define HYPERVISOR_END_ADDR 0x8d000000 //as long as this is after the bss section in the hypervisr.map we are OK.
#define HYPERVISOR_SIZE (HYPERVISOR_END_ADDR - HYPERVISOR_START_ADDR) //This is a hack for now
#define HYPERVISOR_FA_DOMAIN 3 //frame allocator domain

//RAM in machine (bytes) -> this should probably be calculated dynamically. Static for simplicity to begin with
#define TOTAL_MACHINE_RAM (256 * 1024 * 1024)

//Smallest chunksize we are going to allocate (in bytes)
#define FRAME_TABLE_CHUNK_SIZE 4096

#define BEAGLE_RAM_START  0x80000000
#define BEAGLE_RAM_END    0x8FFFFFFF

#endif
