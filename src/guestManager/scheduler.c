#include "common/debug.h"

#include "guestManager/scheduler.h"

#include "vm/omap35xx/intc.h"

#include "cpuArch/armv7.h"


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
  printf("scheduler: scheduleGuest() implement me!" EOL);
  //   - return to irq handler
  //   - restore guest state
  //   - return to guest...
  return;
}

void guestIdle(GCONTXT * context)
{
  context->guestIdle = TRUE;

  // enable interrupts if they were disabled...
  enableInterrupts();

  while (!isIrqPending(context->vm.irqController))
  {
    // delay
    volatile u32int i = 0;
    while (i < 1000000)
    {
      i++;
    }
  }
  context->guestIdle = FALSE;
}
