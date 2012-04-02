#ifndef __COMMON__MARKERS_H__
#define __COMMON__MARKERS_H__

#include "common/types.h"


#define HYPERVISOR_IMAGE_START_ADDRESS  ((u32int)&(__START_MARKER__))
#define HYPERVISOR_IMAGE_END_ADDRESS    ((u32int)&(__END_MARKER__))


extern void *__START_MARKER__;
extern void *__END_MARKER__;

#endif
