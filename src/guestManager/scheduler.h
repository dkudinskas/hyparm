#ifndef __GUEST_MANAGER__SCHEDULER_H__
#define __GUEST_MANAGER__SCHEDULER_H__

#include "guestManager/guestContext.h"

// uncomment me to enable debug : #define SCHEDULER_DBG

void scheduleGuest(void);
void guestIdle(GCONTXT * context);

#endif

