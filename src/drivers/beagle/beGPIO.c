#include "drivers/beagle/beGPIO.h"
#include "common/debug.h"
void beStoreGPIO(u32int regOffs, u32int value, u32int gpid)
{
	volatile u32int *regPtr = (u32int*)(beGetGPIOBaseAddr(gpid) | regOffs);
	*regPtr = value;
}

u32int beGetGPIO(u32int regOffs, u32int gpid)
{
	volatile u32int *regPtr = (u32int*)(beGetGPIOBaseAddr(gpid) | regOffs);
	return *regPtr;
}

u32int beGetGPIOBaseAddr(u32int id)
{
    /* We count from 0 not from 1 !! */
	switch(id)
	{
		case 0:
			return GPIO1_BASE;
		case 1:
			return GPIO2_BASE;
		case 2:
			return GPIO3_BASE;
		case 3:
			return GPIO4_BASE;
		case 4:
			return GPIO5_BASE;
		case 5:
			return GPIO6_BASE;
		default:
			DIE_NOW(0, "beGetGPIOBaseAddr: invalid base id");
	}
	return -1;
}
