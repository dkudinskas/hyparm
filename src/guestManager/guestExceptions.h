#ifndef __GUEST_INTERRUPTS_H__
#define __GUEST_INTERRUPTS_H__

#include "types.h"
#include "serial.h"


// uncomment me to enable debug : #define GUEST_EXCEPTIONS_DBG

#define CPSR_NEG_CCFLAG       0x80000000
#define CPSR_ZER_CCFLAG       0x40000000
#define CPSR_CAR_CCFLAG       0x20000000
#define CPSR_OVF_CCFLAG       0x10000000
#define CPSR_CUM_SAT_FLAG     0x08000000
#define CPSR_IF_THEN_0_1      0x06000000
#define CPSR_JAZELLE_MODE     0x01000000
#define CPSR_RESERVED         0x00f00000
#define CPSR_GTE_FLAGS_SIMD   0x000f0000
#define CPSR_IF_THEN_2_7      0x0000fc00
#define CPSR_ENDIANNESS       0x00000200 
#define CPSR_ASYNC_ABT_DIS    0x00000100
#define CPSR_IRQ_DIS          0x00000080
#define CPSR_FIQ_DIS          0x00000040
#define CPSR_THUMB_MODE       0x00000020
#define CPSR_MODE             0x0000001f
#define CPSR_MODE_USR    0x10
#define CPSR_MODE_FIQ    0x11
#define CPSR_MODE_IRQ    0x12
#define CPSR_MODE_SVC    0x13
#define CPSR_MODE_ABT    0x17
#define CPSR_MODE_UND    0x1b
#define CPSR_MODE_SYS    0x1f

// no need to throw a service call to guest context.
// hypercall handler deals with it.
void deliverServiceCall(void);

void throwInterrupt(u32int irqNumber);
void deliverInterrupt(void);

void throwDataAbort(u32int address, u32int faultType, bool isWrite, u32int domain);
void deliverDataAbort(void);

void throwPrefetchAbort(u32int address, u32int faultType);
void deliverPrefetchAbort(void);

#endif
