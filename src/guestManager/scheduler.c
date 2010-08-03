#include "scheduler.h"

void scheduleGuest()
{
  // TO IMPLEMENT:
  // pre-conditions:
  //   - IRQ fired
  //   - decode number
  //   - if tick, save guest context
  //   - call handler...
  // this: called once we receive a tick
  // check if we have more than one guest
  // if we have only one guest, adjust guest state to deliver tick
  // post-actions
  serial_putstring("scheduler: scheduleGuest() implement me!");
  //   - return to irq handler
  //   - restore guest state
  //   - return to guest...
  return;
}
