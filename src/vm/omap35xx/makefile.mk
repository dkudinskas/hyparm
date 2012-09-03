HYPARM_SRCS_C-y += vm/omap35xx/clockManager.c
HYPARM_SRCS_C-y += vm/omap35xx/controlModule.c
HYPARM_SRCS_C-y += vm/omap35xx/cp15coproc.c
HYPARM_SRCS_C-y += vm/omap35xx/gpio.c
HYPARM_SRCS_C-y += vm/omap35xx/gpmc.c
HYPARM_SRCS_C-y += vm/omap35xx/gptimer.c
HYPARM_SRCS_C-y += vm/omap35xx/hardwareLibrary.c
HYPARM_SRCS_C-y += vm/omap35xx/intc.c
HYPARM_SRCS_C-y += vm/omap35xx/prm.c
HYPARM_SRCS_C-y += vm/omap35xx/sdma.c
HYPARM_SRCS_C-y += vm/omap35xx/sdram.c
HYPARM_SRCS_C-y += vm/omap35xx/sramInternal.c
HYPARM_SRCS_C-y += vm/omap35xx/sysControlModule.c
HYPARM_SRCS_C-y += vm/omap35xx/timer32k.c
HYPARM_SRCS_C-y += vm/omap35xx/uart.c

HYPARM_SRCS_C-$(CONFIG_GUEST_ANDROID) += vm/omap35xx/dmtimer.c
HYPARM_SRCS_C-$(CONFIG_GUEST_ANDROID) += vm/omap35xx/twl4030.c
HYPARM_SRCS_C-$(CONFIG_GUEST_ANDROID) += vm/omap35xx/i2c.c
HYPARM_SRCS_C-$(CONFIG_GUEST_ANDROID) += vm/omap35xx/mmc.c
HYPARM_SRCS_C-$(CONFIG_GUEST_ANDROID) += vm/omap35xx/pm.c
HYPARM_SRCS_C-$(CONFIG_GUEST_ANDROID) += vm/omap35xx/sdrc.c
HYPARM_SRCS_C-$(CONFIG_GUEST_ANDROID) += vm/omap35xx/sms.c
HYPARM_SRCS_C-$(CONFIG_GUEST_ANDROID) += vm/omap35xx/wdtimer.c

HYPARM_SRCS_C-$(CONFIG_PROFILER) += vm/omap35xx/profiler.c
