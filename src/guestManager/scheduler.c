#include "guestManager/scheduler.h"

#include "hardware/serial.h"
#include "hardware/intc.h"

#include "cpuArch/cpu.h"

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
  serial_newline();
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

  while (!isIrqPending())
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
