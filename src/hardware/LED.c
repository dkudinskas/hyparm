#include "hardware/LED.h"


ulong * ledPtrSet = (unsigned long int*)(DATA_SET_REG);
ulong * ledPtrClear = (unsigned long int*)(DATA_CLEAR_REG);

int turnOff()
{
	int y = 0;
	while (y < 100000)
	{
		y++;
		if (y == 99999)
		{
			*ledPtrSet = LED1 | LED2;
		}	  
	}
	return y;
}

int turnOn()
{
	int y = 0;
	while (y < 100000)
	{
		y++;
		if (y == 99999)
		{
			*ledPtrClear = LED1 | LED2;
		}	  
	}
	return y;
}
