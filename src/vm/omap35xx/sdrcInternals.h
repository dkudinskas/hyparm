#ifndef __VM__OMAP_35XX__SDRC_INTERNALS_H__
#define __VM__OMAP_35XX__SDRC_INTERNALS_H__

/************************
 * REGISTER DEFINITIONS *
 ************************/

#define SDRC_REVISION           0x00
#define SDRC_SYSCONFIG          0x10
#define SDRC_SYSSTATUS          0x14
#define SDRC_CS_CFG             0x40
#define SDRC_SHARING            0x44
#define SDRC_ERR_ADDR           0x48
#define SDRC_ERR_TYPE           0x4C
#define SDRC_DLLA_CTRL          0x60
#define SDRC_DLLA_STATUS        0x64
#define SDRC_POWER_REG          0x70

#define SDRC_MCFG(i)            (0x80 + ((i)*0x30))
#define SDRC_MR(i)              (0x84 + ((i)*0x30))
#define SDRC_EMR2(i)            (0x8C + ((i)*0x30))
#define SDRC_ACTIM_CTRLA(i)     (0x9C + ((i)*0x28))
#define SDRC_ACTIM_CTRLB(i)     (0xA0 + ((i)*0x28))
#define SDRC_RFR_CTRL(i)        (0xA4 + ((i)*0x30))
#define SDRC_MANUAL(i)          (0xA8 + ((i)*0x30))


/**************************
 * STATIC REGISTER VALUES *
 **************************/

#define SDRC_REVISION_VALUE     0x00000040

#endif /* __VM__OMAP_35XX__SDRC_INTERNALS_H__ */
