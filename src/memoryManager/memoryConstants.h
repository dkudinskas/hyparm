#ifndef __MEMORY_MANAGER__MEMORY_CONSTANTS_H__
#define __MEMORY_MANAGER__MEMORY_CONSTANTS_H__

/* These constants should be sourced from somewhere more appropriate
   The hypervisor size should be calculated using symbols
*/
#ifdef CONFIG_BOARD_TI_BEAGLE_BOARD
#define MEMORY_START_ADDR            BE_RAM
#define MEMORY_END_ADDR             (BE_RAM+BE_RAM_SIZE)
#else
#error "MemoryConstants: undefined target!"
#endif

#endif
