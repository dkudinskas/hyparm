#ifndef __VM__OMAP_35XX__PM_INTERNALS_H__
#define __VM__OMAP_35XX__PM_INTERNALS_H__

/******************
 * BASE REGISTERS *
 ******************/

#define PM_RT                 0x68010000  // Register target
#define PM_GPMC               0x68012400  // General-purpose memory controller
#define PM_OCM_RAM            0x68012800  // On-Chip RAM
#define PM_OCM_ROM            0x68012C00  // On-Chip ROM
#define PM_MAD2D              0x68013000
#define PM_IVA2               0x68014000  // IVA2.2 subsystem

/************************
 * REGISTER DEFINITIONS *
 ************************/

#define PM_ERROR_LOG                0x20
#define PM_CONTROL                  0x28
#define PM_ERROR_CLEAR_SINGLE       0x30
#define PM_ERROR_CLEAR_MULTI        0x38

// PM_REQ_INFO_PERMISSION
#define PM_REQ_INFO_PERMISSION_0    0x48
#define PM_REQ_INFO_PERMISSION_1    0x68
#define PM_REQ_INFO_PERMISSION_2    0x88
#define PM_REQ_INFO_PERMISSION_3    0xA8
#define PM_REQ_INFO_PERMISSION_4    0xC8
#define PM_REQ_INFO_PERMISSION_5    0xE8
#define PM_REQ_INFO_PERMISSION_6    0x108
#define PM_REQ_INFO_PERMISSION_7    0x128

// PM_READ_PERMISSION
#define PM_READ_PERMISSION_0        0x50
#define PM_READ_PERMISSION_1        0x70
#define PM_READ_PERMISSION_2        0x90
#define PM_READ_PERMISSION_3        0xB0
#define PM_READ_PERMISSION_4        0xD0
#define PM_READ_PERMISSION_5        0xF0
#define PM_READ_PERMISSION_6        0x110
#define PM_READ_PERMISSION_7        0x130

// PM_WRITE_PERMISSION
#define PM_WRITE_PERMISSION_0       0x58
#define PM_WRITE_PERMISSION_1       0x78
#define PM_WRITE_PERMISSION_2       0x98
#define PM_WRITE_PERMISSION_3       0xB8
#define PM_WRITE_PERMISSION_4       0xD8
#define PM_WRITE_PERMISSION_5       0xF8
#define PM_WRITE_PERMISSION_6       0x118
#define PM_WRITE_PERMISSION_7       0x138

// PM_ADDR_MATCH
#define PM_ADDR_MATCH_0             0x60
#define PM_ADDR_MATCH_1             0x80
#define PM_ADDR_MATCH_2             0xA0
#define PM_ADDR_MATCH_3             0xC0
#define PM_ADDR_MATCH_4             0xE0
#define PM_ADDR_MATCH_5             0x100
#define PM_ADDR_MATCH_6             0x120
#define PM_ADDR_MATCH_7             0x140

#endif /* __VM__OMAP_35XX__PM_INTERNALS_H__ */
