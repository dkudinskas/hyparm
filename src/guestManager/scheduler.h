#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include "types.h"
#include "serial.h"
#include "guestContext.h"

// uncomment me to enable debug : #define SCHEDULER_DBG

void scheduleGuest(void);
void guestIdle(GCONTXT * context);

#endif

