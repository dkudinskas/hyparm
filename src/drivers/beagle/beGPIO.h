#ifndef __DRIVERS_BEAGLE_BE_GPIO_H__
#define __DRIVERS_BEAGLE_BE_GPIO_H__

#include "common/types.h"

/* GPIO base registers */
#define GPIO1_BASE	0x48310000
#define GPIO2_BASE	0x49050000
#define GPIO3_BASE	0x49052000
#define GPIO4_BASE	0x49054000
#define GPIO5_BASE	0x49056000
#define GPIO6_BASE	0x49058000

void beStoreGPIO(u32int regOffs, u32int value, u32int gpid);
u32int beGetGPIO(u32int regOffs, u32int gpid);
u32int beGetGPIOBaseAddr(u32int gpid);

#endif
