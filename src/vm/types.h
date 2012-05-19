#ifndef __VM__TYPES_H__
#define __VM__TYPES_H__

#include "common/types.h"


typedef enum loadStoreAccessSize
{
  BYTE,
  HALFWORD,
  WORD,
} ACCESS_SIZE;

typedef struct genericDevice device;

typedef u32int (*LOAD_FUNCTION)(device *, ACCESS_SIZE, u32int, u32int);
typedef void (*STORE_FUNCTION)(device *, ACCESS_SIZE, u32int, u32int, u32int);

#define MAX_NR_ATTACHED  20

struct genericDevice
{
  const char *deviceName;
  bool isBus;
  u32int startAddressMapped;
  u32int endAddressMapped;
  device *parentDevice;
  u32int nrOfAttachedDevs;
  device *attachedDevices[MAX_NR_ATTACHED];
  LOAD_FUNCTION loadFunction;
  STORE_FUNCTION storeFunction;
};

#endif /* __VM__TYPES_H__ */
