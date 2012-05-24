#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/sms.h"
#include "vm/omap35xx/smsInternals.h"


void initSms(virtualMachine *vm)
{
  /**
   * initialization of SMS
   */
  struct Sms *const sms = (struct Sms *)calloc(1, sizeof(struct Sms));
  if (sms == NULL)
  {
    DIE_NOW(NULL, "Failed to allocate SMS.");
  }
  vm->sms = sms;

  DEBUG(VP_OMAP_35XX_SMS, "initSms @ %p" EOL, sms);

  sms->smsSysconfig = 0x1;
  sms->smsSysstatus = 0x1;

  sms->smsRgAtt0 = 0;
  sms->smsRgAtt1 = 0;
  sms->smsRgAtt2 = 0;
  sms->smsRgAtt3 = 0;
  sms->smsRgAtt4 = 0;
  sms->smsRgAtt5 = 0;
  sms->smsRgAtt6 = 0;
  sms->smsRgAtt7 = 0;

  sms->smsRgRdPerm0 = 0;
  sms->smsRgRdPerm1 = 0;
  sms->smsRgRdPerm2 = 0;
  sms->smsRgRdPerm3 = 0;
  sms->smsRgRdPerm4 = 0;
  sms->smsRgRdPerm5 = 0;
  sms->smsRgRdPerm6 = 0;
  sms->smsRgRdPerm7 = 0;

  sms->smsRgWrPerm0 = 0;
  sms->smsRgWrPerm1 = 0;
  sms->smsRgWrPerm2 = 0;
  sms->smsRgWrPerm3 = 0;
  sms->smsRgWrPerm4 = 0;
  sms->smsRgWrPerm5 = 0;
  sms->smsRgWrPerm6 = 0;
  sms->smsRgWrPerm7 = 0;
}

/* top load function */
u32int loadSms(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  ASSERT(size == WORD, ERROR_BAD_ACCESS_SIZE);

  struct Sms *const sms = context->vm.sms;
  u32int regOffset = phyAddr - Q1_L3_SMS;
  u32int value = 0;

  switch (regOffset)
  {
    case SMS_REVISION:
    {
      value = SMS_REVISION_VALUE;
      break;
    }
    case SMS_SYSCONFIG:
    {
      value = sms->smsSysconfig;
      break;
    }
    case SMS_SYSSTATUS:
    {
      value = sms->smsSysstatus;
      break;
    }
    case SMS_RG_ATT(0):
    {
      value = sms->smsRgAtt0;
      break;
    }
    case SMS_RG_ATT(1):
    {
      value = sms->smsRgAtt1;
      break;
    }
    case SMS_RG_ATT(2):
    {
      value = sms->smsRgAtt2;
      break;
    }
    case SMS_RG_ATT(3):
    {
      value = sms->smsRgAtt3;
      break;
    }
    case SMS_RG_ATT(4):
    {
      value = sms->smsRgAtt4;
      break;
    }
    case SMS_RG_ATT(5):
    {
      value = sms->smsRgAtt5;
      break;
    }
    case SMS_RG_ATT(6):
    {
      value = sms->smsRgAtt6;
      break;
    }
    case SMS_RG_ATT(7):
    {
      value = sms->smsRgAtt7;
      break;
    }
    case SMS_RG_RDPERM(0):
    {
      value = sms->smsRgRdPerm0;
      break;
    }
    case SMS_RG_RDPERM(1):
    {
      value = sms->smsRgRdPerm1;
      break;
    }
    case SMS_RG_RDPERM(2):
    {
      value = sms->smsRgRdPerm2;
      break;
    }
    case SMS_RG_RDPERM(3):
    {
      value = sms->smsRgRdPerm3;
      break;
    }
    case SMS_RG_RDPERM(4):
    {
      value = sms->smsRgRdPerm4;
      break;
    }
    case SMS_RG_RDPERM(5):
    {
      value = sms->smsRgRdPerm5;
      break;
    }
    case SMS_RG_RDPERM(6):
    {
      value = sms->smsRgRdPerm6;
      break;
    }
    case SMS_RG_RDPERM(7):
    {
      value = sms->smsRgRdPerm7;
      break;
    }
    case SMS_RG_WRPERM(0):
    {
      value = sms->smsRgRdPerm0;
      break;
    }
    case SMS_RG_WRPERM(1):
    {
      value = sms->smsRgWrPerm1;
      break;
    }
    case SMS_RG_WRPERM(2):
    {
      value = sms->smsRgWrPerm2;
      break;
    }
    case SMS_RG_WRPERM(3):
    {
      value = sms->smsRgWrPerm3;
      break;
    }
    case SMS_RG_WRPERM(4):
    {
      value = sms->smsRgWrPerm4;
      break;
    }
    case SMS_RG_WRPERM(5):
    {
      value = sms->smsRgWrPerm5;
      break;
    }
    case SMS_RG_WRPERM(6):
    {
      value = sms->smsRgWrPerm6;
      break;
    }
    case SMS_RG_WRPERM(7):
    {
      value = sms->smsRgWrPerm7;
      break;
    }
    default:
      printf("%s load from pAddr: %#.8x, vAddr: %#.8x, accSize %x" EOL, dev->deviceName, phyAddr,
          virtAddr, (u32int)size);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
  }

  DEBUG(VP_OMAP_35XX_SMS, "%s load from pAddr: %#.8x, vAddr: %#.8x, accSize %#x" EOL,
      dev->deviceName, phyAddr, virtAddr, (u32int)size);

  return value;
}

/* top store function */
void storeSms(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  DEBUG(VP_OMAP_35XX_SMS, "%s store to pAddr: %#.8x, vAddr %#.8x, aSize %#x, val %#.8x" EOL,
      dev->deviceName, phyAddr, virtAddr, (u32int)size, value);

  struct Sms *const sms = context->vm.sms;
  u32int regOffset = phyAddr - Q1_L3_SMS;

  switch (regOffset)
  {
    case SMS_SYSCONFIG:
    {
      sms->smsSysconfig = value;
      break;
    }
    case SMS_RG_ATT(0):
    {
      sms->smsRgAtt0 = value;
      break;
    }
    case SMS_RG_ATT(1):
    {
      sms->smsRgAtt1 = value;
      break;
    }
    case SMS_RG_ATT(2):
    {
      sms->smsRgAtt2 = value;
      break;
    }
    case SMS_RG_ATT(3):
    {
      sms->smsRgAtt3 = value;
      break;
    }
    case SMS_RG_ATT(4):
    {
      sms->smsRgAtt4 = value;
      break;
    }
    case SMS_RG_ATT(5):
    {
      sms->smsRgAtt5 = value;
      break;
    }
    case SMS_RG_ATT(6):
    {
      sms->smsRgAtt6 = value;
      break;
    }
    case SMS_RG_ATT(7):
    {
      sms->smsRgAtt7 = value;
      break;
    }
    case SMS_RG_RDPERM(0):
    {
      sms->smsRgRdPerm0 = value;
      break;
    }
    case SMS_RG_RDPERM(1):
    {
      sms->smsRgRdPerm1 = value;
      break;
    }
    case SMS_RG_RDPERM(2):
    {
      sms->smsRgRdPerm2 = value;
      break;
    }
    case SMS_RG_RDPERM(3):
    {
      sms->smsRgRdPerm3 = value;
      break;
    }
    case SMS_RG_RDPERM(4):
    {
      sms->smsRgRdPerm4 = value;
      break;
    }
    case SMS_RG_RDPERM(5):
    {
      sms->smsRgRdPerm5 = value;
      break;
    }
    case SMS_RG_RDPERM(6):
    {
      sms->smsRgRdPerm6 = value;
      break;
    }
    case SMS_RG_RDPERM(7):
    {
      sms->smsRgRdPerm7 = value;
      break;
    }
    case SMS_RG_WRPERM(0):
    {
      sms->smsRgRdPerm0 = value;
      break;
    }
    case SMS_RG_WRPERM(1):
    {
      sms->smsRgWrPerm1 = value;
      break;
    }
    case SMS_RG_WRPERM(2):
    {
      sms->smsRgWrPerm2 = value;
      break;
    }
    case SMS_RG_WRPERM(3):
    {
      sms->smsRgWrPerm3 = value;
      break;
    }
    case SMS_RG_WRPERM(4):
    {
      sms->smsRgWrPerm4 = value;
      break;
    }
    case SMS_RG_WRPERM(5):
    {
      sms->smsRgWrPerm5 = value;
      break;
    }
    case SMS_RG_WRPERM(6):
    {
      sms->smsRgWrPerm6 = value;
      break;
    }
    case SMS_RG_WRPERM(7):
    {
      sms->smsRgWrPerm7 = value;
      break;
    }
    case SMS_REVISION:
    case SMS_SYSSTATUS:
    {
      DIE_NOW(NULL, "Sms: store to read-only register");
    }
    default:
      printf("%s store to pAddr: %#.8x, vAddr: %#.8x, accSize %x" EOL, dev->deviceName, phyAddr,
          virtAddr, (u32int)size);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
  }
}
