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

#define GPMC_CONFIG1_0                                 0x00000060
#define GPMC_CONFIG2_0                                 0x00000064
#define GPMC_CONFIG3_0                                 0x00000068
#define GPMC_CONFIG4_0                                 0x0000006c
#define GPMC_CONFIG5_0                                 0x00000070
#define GPMC_CONFIG6_0                                 0x00000074
#define GPMC_CONFIG7_0                                 0x00000078
#define GPMC_NAND_COMMAND_0                            0x0000007c
#define GPMC_NAND_ADDRESS_0                            0x00000080
#define GPMC_NAND_DATA_0                               0x00000084

#define GPMC_CONFIG1_1                                 0x00000090
#define GPMC_CONFIG2_1                                 0x00000094
#define GPMC_CONFIG3_1                                 0x00000098
#define GPMC_CONFIG4_1                                 0x0000009c
#define GPMC_CONFIG5_1                                 0x000000a0
#define GPMC_CONFIG6_1                                 0x000000a4
#define GPMC_CONFIG7_1                                 0x000000a8
#define GPMC_NAND_COMMAND_1                            0x000000ac
#define GPMC_NAND_ADDRESS_1                            0x000000b0
#define GPMC_NAND_DATA_1                               0x000000b4

#define GPMC_CONFIG1_2                                 0x000000c0
#define GPMC_CONFIG2_2                                 0x000000c4
#define GPMC_CONFIG3_2                                 0x000000c8
#define GPMC_CONFIG4_2                                 0x000000cc
#define GPMC_CONFIG5_2                                 0x000000d0
#define GPMC_CONFIG6_2                                 0x000000d4
#define GPMC_CONFIG7_2                                 0x000000d8
#define GPMC_NAND_COMMAND_2                            0x000000dc
#define GPMC_NAND_ADDRESS_2                            0x000000e0
#define GPMC_NAND_DATA_2                               0x000000e4

#define GPMC_CONFIG1_3                                 0x000000f0
#define GPMC_CONFIG2_3                                 0x000000f4
#define GPMC_CONFIG3_3                                 0x000000f8
#define GPMC_CONFIG4_3                                 0x000000fc
#define GPMC_CONFIG5_3                                 0x00000100
#define GPMC_CONFIG6_3                                 0x00000104
#define GPMC_CONFIG7_3                                 0x00000108
#define GPMC_NAND_COMMAND_3                            0x0000010c
#define GPMC_NAND_ADDRESS_3                            0x00000110
#define GPMC_NAND_DATA_3                               0x00000114

#define GPMC_CONFIG1_4                                 0x00000120
#define GPMC_CONFIG2_4                                 0x00000124
#define GPMC_CONFIG3_4                                 0x00000128
#define GPMC_CONFIG4_4                                 0x0000012c
#define GPMC_CONFIG5_4                                 0x00000130
#define GPMC_CONFIG6_4                                 0x00000134
#define GPMC_CONFIG7_4                                 0x00000138
#define GPMC_NAND_COMMAND_4                            0x0000013c
#define GPMC_NAND_ADDRESS_4                            0x00000140
#define GPMC_NAND_DATA_4                               0x00000144

#define GPMC_CONFIG1_5                                 0x00000150
#define GPMC_CONFIG2_5                                 0x00000154
#define GPMC_CONFIG3_5                                 0x00000158
#define GPMC_CONFIG4_5                                 0x0000015c
#define GPMC_CONFIG5_5                                 0x00000160
#define GPMC_CONFIG6_5                                 0x00000164
#define GPMC_CONFIG7_5                                 0x00000168
#define GPMC_NAND_COMMAND_5                            0x0000016c
#define GPMC_NAND_ADDRESS_5                            0x00000170
#define GPMC_NAND_DATA_5                               0x00000174

#define GPMC_CONFIG1_6                                 0x00000180
#define GPMC_CONFIG2_6                                 0x00000184
#define GPMC_CONFIG3_6                                 0x00000188
#define GPMC_CONFIG4_6                                 0x0000018c
#define GPMC_CONFIG5_6                                 0x00000190
#define GPMC_CONFIG6_6                                 0x00000194
#define GPMC_CONFIG7_6                                 0x00000198
#define GPMC_NAND_COMMAND_6                            0x0000019c
#define GPMC_NAND_ADDRESS_6                            0x000001a0
#define GPMC_NAND_DATA_6                               0x000001a4

#define GPMC_CONFIG1_7                                 0x000001b0
#define GPMC_CONFIG2_7                                 0x000001b4
#define GPMC_CONFIG3_7                                 0x000001b8
#define GPMC_CONFIG4_7                                 0x000001bc
#define GPMC_CONFIG5_7                                 0x000001c0
#define GPMC_CONFIG6_7                                 0x000001c4
#define GPMC_CONFIG7_7                                 0x000001c8
#define GPMC_NAND_COMMAND_7                            0x000001cc
#define GPMC_NAND_ADDRESS_7                            0x000001d0
#define GPMC_NAND_DATA_7                               0x000001d4

#define GPMC_PREFETCH_CONFIG1                          0x000001e0
#define GPMC_PREFETCH_CONFIG2                          0x000001e4
#define GPMC_PREFETCH_CONTROL                          0x000001ec
#define GPMC_PREFETCH_STATUS                           0x000001f0
#define GPMC_ECC_CONFIG                                0x000001f4
#define GPMC_ECC_CONTROL                               0x000001f8
#define GPMC_ECC_SIZE_CONFIG                           0x000001fc

#define GPMC_ECC1_RESULT                               0x00000200
#define GPMC_ECC2_RESULT                               0x00000204
#define GPMC_ECC3_RESULT                               0x00000208
#define GPMC_ECC4_RESULT                               0x0000020c
#define GPMC_ECC5_RESULT                               0x00000210
#define GPMC_ECC6_RESULT                               0x00000214
#define GPMC_ECC7_RESULT                               0x00000218
#define GPMC_ECC8_RESULT                               0x0000021c
#define GPMC_ECC9_RESULT                               0x00000220

#define GPMC_BCH_RESULT0_0                             0x00000240
#define GPMC_BCH_RESULT1_0                             0x00000244
#define GPMC_BCH_RESULT2_0                             0x00000248
#define GPMC_BCH_RESULT3_0                             0x0000024c

#define GPMC_BCH_RESULT0_1                             0x00000250
#define GPMC_BCH_RESULT1_1                             0x00000254
#define GPMC_BCH_RESULT2_1                             0x00000258
#define GPMC_BCH_RESULT3_1                             0x0000025c

#define GPMC_BCH_RESULT0_2                             0x00000260
#define GPMC_BCH_RESULT1_2                             0x00000264
#define GPMC_BCH_RESULT2_2                             0x00000268
#define GPMC_BCH_RESULT3_2                             0x0000026c

#define GPMC_BCH_RESULT0_3                             0x00000270
#define GPMC_BCH_RESULT1_3                             0x00000274
#define GPMC_BCH_RESULT2_3                             0x00000278
#define GPMC_BCH_RESULT3_3                             0x0000027c

#define GPMC_BCH_RESULT0_4                             0x00000280
#define GPMC_BCH_RESULT1_4                             0x00000284
#define GPMC_BCH_RESULT2_4                             0x00000288
#define GPMC_BCH_RESULT3_4                             0x0000028c

#define GPMC_BCH_RESULT0_5                             0x00000290
#define GPMC_BCH_RESULT1_5                             0x00000294
#define GPMC_BCH_RESULT2_5                             0x00000298
#define GPMC_BCH_RESULT3_5                             0x0000029c

#define GPMC_BCH_RESULT0_6                             0x000002a0
#define GPMC_BCH_RESULT1_6                             0x000002a4
#define GPMC_BCH_RESULT2_6                             0x000002a8
#define GPMC_BCH_RESULT3_6                             0x000002ac

#define GPMC_BCH_RESULT0_7                             0x000002b0
#define GPMC_BCH_RESULT1_7                             0x000002b4
#define GPMC_BCH_RESULT2_7                             0x000002b8
#define GPMC_BCH_RESULT3_7                             0x000002bc

#define GPMC_BCH_SWDATA                                0x000002d0


/**************************
 * STATIC REGISTER VALUES *
 **************************/
#define GPMC_REVISION_VALUE                            0x00000000


/***********************
 * REGISTER BIT VALUES *
 ***********************/
#define GPMC_SYSCONFIG_MASK                            0x00000019
#define GPMC_SYSCONFIG_SOFTRESET                       0x00000002

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

  u32int gpmcConfig7_0;
  u32int gpmcConfig7_1;
  u32int gpmcConfig7_2;
  u32int gpmcConfig7_3;
  u32int gpmcConfig7_4;
  u32int gpmcConfig7_5;
  u32int gpmcConfig7_6;
  u32int gpmcConfig7_7;

  u32int gpmcPrefetchConfig1;
  u32int gpmcPrefetchConfig2;
  u32int gpmcPrefetchControl;
  u32int gpmcPrefetchStatus;
  u32int gpmcEccConfig;
  u32int gpmcEccControl;
  u32int gpmcEccSizeConfig;
};


#endif
