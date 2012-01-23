#include "common/debug.h"
#include "common/stddef.h"


#ifdef CONFIG_BUILD_SSP

void __attribute__((externally_visible,noreturn)) __stack_chk_fail(void)
{
  DIE_NOW(NULL, "SSP error");
}

#endif
