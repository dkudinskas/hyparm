HYPARM_SRCS_C-y += drivers/beagle/be32kTimer.c
HYPARM_SRCS_C-y += drivers/beagle/beClockMan.c
HYPARM_SRCS_C-y += drivers/beagle/beGPTimer.c
HYPARM_SRCS_C-y += drivers/beagle/beIntc.c
HYPARM_SRCS_C-y += drivers/beagle/beUart.c

HYPARM_SRCS_C-$(CONFIG_GUEST_FREERTOS) += drivers/beagle/beGPIO.c

HYPARM_SRCS_C-$(CONFIG_MMC) += drivers/beagle/beMMC.c
