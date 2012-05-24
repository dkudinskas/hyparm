#ifndef __VM__OMAP35XX__CONTROL_MODULE_INTERNALS_H__
#define __VM__OMAP35XX__CONTROL_MODULE_INTERNALS_H__

/**
 * IMPORTANT:
 * Linux kernel arch/arm/mach-omap2/id.c
 * really misidentifies more current OMAP revisions, thus linux register definitions
 * are completely wrong.
 **/

#define CONTROL_MOD_IDCODE            0x204
#define CONTROL_MOD_IDCODE_VALUE      0x4B7AE02F
#define CONTROL_MOD_RESERVED          0x208
#define CONTROL_MOD_RESERVED_VALUE    0x00000000
#define CONTROL_MOD_PROD_ID           0x20c
#define CONTROL_MOD_PROD_ID_600_430   0x00000000
#define CONTROL_MOD_PROD_ID_720_520   0x00000008

#endif /* __VM__OMAP35XX__CONTROL_MODULE_INTERNALS_H__ */
