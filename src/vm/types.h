#ifndef __VM__TYPES_H__
#define __VM__TYPES_H__

#include "common/types.h"


typedef struct genericDevice device;
struct guestContext;

typedef u32int (*LOAD_FUNCTION)(struct guestContext *, device *, ACCESS_SIZE, u32int, u32int);
typedef void (*STORE_FUNCTION)(struct guestContext *, device *, ACCESS_SIZE, u32int, u32int, u32int);

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

typedef struct EmulatedVirtualMachine virtualMachine;

#endif /* __VM__TYPES_H__ */
