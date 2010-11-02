#ifndef __HARDWARE_LIBRARY_H__
#define __HARDWARE_LIBRARY_H__

#include "types.h"
#include "serial.h"

// uncomment me to enable hardwareLibrary debug : #define HARDWARE_LIB_DBG

#define QUARTER_SIZE               0x40000000

#define QUARTER0                   0x00000000

#define QUARTER1                   0x40000000
#define Q1_ON_CHIP_MEMORY            0x40000000
#define Q1_ON_CHIP_MEMORY_SIZE       0x08000000
#define Q1_OCM_BOOT_ROM_SECURE         0x40000000
#define Q1_OCM_BOOT_ROM_SECURE_SIZE    0x00014000
#define Q1_OCM_BOOT_ROM_PUBLIC         0x40014000
#define Q1_OCM_BOOT_ROM_PUBLIC_SIZE    0x00008000
#define Q1_OCM_RESERVED1               0x4001c000
#define Q1_OCM_RESERVED1_SIZE          0x001e4000
#define Q1_OCM_SRAM_INTERNAL           0x40200000
#define Q1_OCM_SRAM_INTERNAL_SIZE      0x00010000
#define Q1_OCM_RESERVED2               0x40210000
#define Q1_OCM_RESERVED2_SIZE          0x07df0000
#define Q1_L4_INTERCONNECT           0x48000000
#define Q1_L4_INTERCONNECT_SIZE      0x08000000
#define Q1_L4_INT_CORE                 0x48000000
#define Q1_L4_INT_CORE_SIZE            0x01000000
#define SYS_CONTROL_MODULE               0x48002000
#define SYS_CONTROL_MODULE_SIZE          0x00002000
#define CLOCK_MANAGER                    0x48004000
#define CLOCK_MANAGER_SIZE               0x00004000
#define SDMA                             0x48056000
#define SDMA_SIZE                        0x00002000
#define UART1                            0x4806a000
#define UART1_SIZE                       0x00002000
#define UART2                            0x4806c000
#define UART2_SIZE                       0x00002000
#define INTERRUPT_CONTROLLER             0x48200000
#define INTERRUPT_CONTROLLER_SIZE        0x00001000
#define L4_CORE_WAKEUP_INT               0x48300000
#define L4_CORE_WAKEUP_INT_SIZE          0x00040FFF
#define PRM                                0x48306000
#define PRM_SIZE                           0x00004000
#define GPIO1                              0x48310000
#define GPIO1_SIZE                         0x00002000
#define WDTIMER2                           0x48314000
#define WDTIMER2_SIZE                      0x00002000
#define GPTIMER1                           0x48318000
#define GPTIMER1_SIZE                      0x00002000
#define TIMER_32K                          0x48320000
#define TIMER_32K_SIZE                     0x00002000
#define Q1_L4_INT_PER                  0x49000000
#define Q1_L4_INT_PER_SIZE             0x00100000
#define UART3                            0x49020000
#define UART3_SIZE                       0x00002000
#define GPIO2                            0x49050000
#define GPIO2_SIZE                       0x00002000
#define GPIO3                            0x49052000
#define GPIO3_SIZE                       0x00002000
#define GPIO4                            0x49054000
#define GPIO4_SIZE                       0x00002000
#define GPIO5                            0x49056000
#define GPIO5_SIZE                       0x00002000
#define GPIO6                            0x49058000
#define GPIO6_SIZE                       0x00002000
#define Q1_L4_INT_PER_SIZE             0x00100000
#define Q1_L4_INT_RESERVED             0x49100000
#define Q1_L4_INT_RESERVED_SIZE        0x06F00000
#define Q1_SLAVE_GRAPHICS            0x50000000
#define Q1_SLAVE_GRAPHICS_SIZE       0x04000000
#define Q1_L4_EMULATION              0x54000000
#define Q1_L4_EMULATION_SIZE         0x04000000
#define Q1_RESERVED1                 0x58000000
#define Q1_RESERVED1_SIZE            0x04000000
#define Q1_IVA22_IMMU                0x5c000000
#define Q1_IVA22_IMMU_SIZE           0x04000000
#define Q1_RESERVED2                 0x60000000
#define Q1_RESERVED2_SIZE            0x08000000
#define Q1_L3_INTERCONNECT           0x68000000
#define Q1_L3_INTERCONNECT_SIZE      0x08000000
#define Q1_L3_GPMC                     0x6E000000
#define Q1_L3_GPMC_SIZE                0x01000000
#define Q1_SDRC_SMS                  0x70000000
#define Q1_SDRC_SMS_SIZE             0x10000000

#define QUARTER2                   0x80000000
#define Q2_SDRC_SMS                  0x80000000
#define Q2_SDRC_SMS_SIZE             0x40000000

#define QUARTER3                   0xC000000
#define Q3_RESERVED                  0xC0000000
#define Q3_RESERVED_SIZE             0x20000000
#define Q3_SDRC_SMS                  0xE0000000
#define Q3_SDRC_SMS_SIZE             0x20000000




#define MAX_NR_ATTACHED  20

struct genericDevice;
typedef struct genericDevice device;

enum loadStoreAccessSize;
typedef enum loadStoreAccessSize ACCESS_SIZE;

typedef u32int(*LOAD_FUNCTION)(device*, ACCESS_SIZE, u32int);
typedef void(*STORE_FUNCTION)(device*, ACCESS_SIZE, u32int, u32int);


enum loadStoreAccessSize
{
  BYTE,
  HALFWORD,
  WORD,
};


struct genericDevice
{
  char * deviceName;
  bool isBus;
  u32int startAddressMapped;
  u32int endAddressMapped;
  struct genericDevice * parentDevice;
  u32int nrOfAttachedDevs;
  struct genericDevice * attachedDevices[MAX_NR_ATTACHED];
  LOAD_FUNCTION loadFunction;
  STORE_FUNCTION storeFunction;
//  u32int (*loadFunction)(device * dev, ACCESS_SIZE size, u32int address);
//  void (*storeFunction)(device * dev, ACCESS_SIZE size, u32int address, u32int value);
};

device * initialiseHardwareLibrary(void);

void initialiseDevice(device * dev, char * devName, bool isBus,
                      u32int addrStart, u32int addrEnd,
                      device * parent, LOAD_FUNCTION ldFn, STORE_FUNCTION stFn);

bool isAddressInDevice(u32int address, device * dev);
bool attachDevice(device * parent, device * child);

u32int loadGeneric(device * dev, ACCESS_SIZE size, u32int address);
void storeGeneric(device * dev, ACCESS_SIZE size, u32int address, u32int value);




#endif
