#ifndef __VM__OMAP_35XX__SRAM_INTERNAL_H__
#define __VM__OMAP_35XX__SRAM_INTERNAL_H__

#include "common/types.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/hardwareLibrary.h"


#define UNDEFINED_EXCEPTION_INSTR    0x4020FFC8 // Undefined PC = [0x4020FFE4]
#define SWI_EXCEPTION_INSTR          0x4020FFCC // SWI PC = [0x4020FFE8]
#define PREFETCH_ABT_EXCEPTION_INSTR 0x4020FFD0 // Pre-fetch abort PC = [0x4020FFEC]
#define DATA_ABT_EXCEPTION_INSTR     0x4020FFD4 // Data abort PC = [0x4020FFF0]
#define UNUSED_EXCEPTION_INSTR       0x4020FFD8 // Unused PC = [0x4020FFF4]
#define IRQ_EXCEPTION_INSTR          0x4020FFDC // IRQ PC = [0x4020FFF8]
#define FIQ_EXCEPTION_INSTR          0x4020FFE0 // FIQ PC = [0x4020FFFC]

#define UNDEFINED_EXCEPTION_ADDR      0x4020FFE4
#define SWI_EXCEPTION_ADDR            0x4020FFE8
#define PREFETCH_ABT_EXCEPTION_ADDR   0x4020FFEC
#define DATA_ABT_EXCEPTION_ADDR       0x4020FFF0
#define UNUSED_EXCEPTION_ADDR         0x4020FFF4
#define IRQ_EXCEPTION_ADDR            0x4020FFF8
#define FIQ_EXCEPTION_ADDR            0x4020FFFC


u32int loadSramInternal(device * dev, ACCESS_SIZE size, u32int address);
void storeSramInternal(device * dev, ACCESS_SIZE size, u32int address, u32int value);

void registerGuestHandler(GCONTXT * gc, u32int address, u32int value);

#endif

