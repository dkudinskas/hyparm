#ifndef __VM__OMAP_35XX__SMS_H__
#define __VM__OMAP_35XX__SMS_H__

#include "common/compiler.h"
#include "common/types.h"

#include "vm/types.h"


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


void initSms(virtualMachine *vm) __cold__;
u32int loadSms(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);
void storeSms(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);

#endif /* __VM__OMAP_35XX__SMS_H__ */

