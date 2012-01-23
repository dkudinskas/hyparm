#ifndef __COMMON__MARKERS_H__
#define __COMMON__MARKERS_H__

#define HYPERVISOR_IMAGE_START_ADDRESS  (&(__START_MARKER__))
#define HYPERVISOR_IMAGE_END_ADDRESS    (&(__END_MARKER__))


extern void *__START_MARKER__;
extern void *__END_MARKER__;

#endif
