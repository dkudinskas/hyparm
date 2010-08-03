#ifndef __GPMC_H__
#define __GPMC_H__

#include "types.h"
#include "serial.h"
#include "hardwareLibrary.h"

// uncomment me to enable debug : 
#define GPMC_DBG


/************************
 * REGISTER DEFINITIONS *
 ************************/
#define GPMC_REVISION                                  0x00000000
#define GPMC_SYSCONFIG                                 0x00000010
#define GPMC_SYSSTATUS                                 0x00000014
#define GPMC_IRQSTATUS                                 0x00000018
#define GPMC_IRQENABLE                                 0x0000001c
#define GPMC_TIMEOUT_CONTROL                           0x00000040
#define GPMC_ERR_ADDRESS                               0x00000044
#define GPMC_ERR_TYPE                                  0x00000048
#define GPMC_CONFIG                                    0x00000050
#define GPMC_STATUS                                    0x00000054
#define GPMC_PREFETCH_CONFIG1                          0x000001e0
#define GPMC_PREFETCH_CONFIG2                          0x000001e4
#define GPMC_PREFETCH_CONTROL                          0x000001ec
#define GPMC_PREFETCH_STATUS                           0x000001f0
#define GPMC_ECC_CONFIG                                0x000001f4
#define GPMC_ECC_CONTROL                               0x000001f8
#define GPMC_ECC_SIZE_CONFIG                           0x000001fc
// TODO:addmore

#define GPMC_REVISION_VALUE                            0x00000000

void initGpmc(void);

/* top load function */
u32int loadGpmc(device * dev, ACCESS_SIZE size, u32int address);


/* top store function */
void storeGpmc(device * dev, ACCESS_SIZE size, u32int address, u32int value);


struct Gpmc
{
  u32int gpmcSysConfig;
  u32int gpmcSysStatus;
  u32int gpmcIrqStatus;
  u32int gpmcIrqEnable;
  u32int gpmcTimeoutControl;
  u32int gpmcErrAddress;
  u32int gpmcErrType;
  u32int gpmcConfig;
  u32int gpmcStatus;
  u32int gpmcPrefetchConfig1;
  u32int gpmcPrefetchConfig2;
  u32int gpmcPrefetchControl;
  u32int gpmcPrefetchStatus;
  u32int gpmcEccConfig;
  u32int gpmcEccControl;
  u32int gpmcEccSizeConfig;
};


#endif
