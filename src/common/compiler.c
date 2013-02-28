#include "common/debug.h"
#include "common/stddef.h"


#ifdef CONFIG_BUILD_SSP

void __lto_preserve__ __attribute__((noreturn)) __stack_chk_fail(void)
{
  DIE_NOW(NULL, "SSP error");
}

#endif
