#ifndef __INSTRUCTION_EMU__LOOP_DETECTOR_H__
#define __INSTRUCTION_EMU__LOOP_DETECTOR_H__

#include "guestManager/guestContext.h"


#ifdef CONFIG_LOOP_DETECTOR

void runLoopDetector(GCONTXT *context);

/*
 * resetLoopDetector
 *
 * Resets the internal state of the loop detector; this function MUST be called whenever the
 * translator is disabled, e.g. when switching to guest userspace.
 */
void resetLoopDetector(GCONTXT *context);

#else

#define runLoopDetector(context)
#define resetLoopDetector(context)

#endif /* CONFIG_LOOP_DETECTOR */

#endif
