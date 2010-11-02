#ifndef __BE_CM_H__
#define __BE_CM_H__

#include "types.h"

/*
 * This driver is for the CM Module of the TI OMAP 35xx only.
 * Sanity check!
 */
#ifndef CONFIG_CPU_TI_OMAP_35XX
#error Incompatible driver 
#endif

void cmDisableDssClocks(void);

#endif

