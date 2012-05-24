#ifndef __VM__OMAP_35XX__SMS_INTERNALS_H__
#define __VM__OMAP_35XX__SMS_INTERNALS_H__

/************************
 * REGISTER DEFINITIONS *
 ************************/

#define SMS_REVISION                0
#define SMS_SYSCONFIG            0x10
#define SMS_SYSSTATUS            0x14
#define SMS_RG_ATT(i)            (0x48 + ((i)*0x20))
#define SMS_RG_RDPERM(i)         (0x50 + ((i)*0x20))
#define SMS_RG_WRPERM(i)         (0x58 + ((i)*0x20))

/**************************
 * STATIC REGISTER VALUES *
 **************************/

#define SMS_REVISION_VALUE       0x30

#endif /* __VM__OMAP_35XX__SMS_INTERNALS_H__ */
