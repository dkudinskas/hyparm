#ifndef __SRAM_INTERNAL_H__
#define __SRAM_INTERNAL_H__

#include "types.h"
#include "guestContext.h"
#include "serial.h"
#include "hardwareLibrary.h"

// uncomment me to enable sdram debug: #define SRAM_INTERNAL_DBG

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

