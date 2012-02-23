HYPARM_SRCS_C-y += guestBoot/linux.c
HYPARM_SRCS_C-y += guestBoot/loader.c
HYPARM_SRCS_C-y += guestBoot/image.c

HYPARM_SRCS_C-$(CONFIG_GUEST_FREERTOS) += guestBoot/freertos.c

HYPARM_SRCS_C-$(CONFIG_GUEST_TEST) += guestBoot/test.c

HYPARM_SRCS_SX-y += guestBoot/callKernel.S
