#ifndef __VM__OMAP_35XX__SMS_H__
#define __VM__OMAP_35XX__SMS_H__

#include "common/types.h"

#include "vm/omap35xx/hardwareLibrary.h"


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

void initSms(void);

/* top load function */
u32int loadSms(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);

/* top store function */
void storeSms(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);


struct Sms
{
  u32int smsSysconfig;
  u32int smsSysstatus;

  u32int smsRgAtt0;
  u32int smsRgAtt1;
  u32int smsRgAtt2;
  u32int smsRgAtt3;
  u32int smsRgAtt4;
  u32int smsRgAtt5;
  u32int smsRgAtt6;
  u32int smsRgAtt7;

  u32int smsRgRdPerm0;
  u32int smsRgRdPerm1;
  u32int smsRgRdPerm2;
  u32int smsRgRdPerm3;
  u32int smsRgRdPerm4;
  u32int smsRgRdPerm5;
  u32int smsRgRdPerm6;
  u32int smsRgRdPerm7;

  u32int smsRgWrPerm0;
  u32int smsRgWrPerm1;
  u32int smsRgWrPerm2;
  u32int smsRgWrPerm3;
  u32int smsRgWrPerm4;
  u32int smsRgWrPerm5;
  u32int smsRgWrPerm6;
  u32int smsRgWrPerm7;
};


#endif /* __VM__OMAP_35XX__SMS_H__ */

