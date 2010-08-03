#ifndef __EXCEPTION_HANDLERS_H__
#define __EXCEPTION_HANDLERS_H__

#include "types.h"
#include "guestContext.h"

#define SWI_END_OF_BLOCK      0x1

// uncomment me to enable exception handler debug : #define EXC_HDLR_DBG

void do_software_interrupt(u32int code);

void do_data_abort(void);
void do_data_abort_hypervisor(void);
void do_undefined(void);
void do_undefined_hypervisor(void);
void do_prefetch_abort(void);
void do_prefetch_abort_hypervisor(void);
void do_monitor_mode(void);
void do_monitor_mode_hypervisor(void);
void do_irq(void);
void do_fiq(void);


#endif

