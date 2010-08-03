# Shell to use
SHELL = /bin/sh

# GNU-ARM tools directory
ifeq ($(USER),alex)
  GNU_ARM = /export/compilers/arm-2009q3-68-arm-none-eabi/
else
  ifeq ($(USER),niels)
    GNU_ARM = $(HOME)/Desktop/arm-2009q1
  else
    GNU_ARM = $(HOME)/arm-dev/compilers/arm-2009q1
  endif
endif
# The gcc compiler and linker
CC      = $(GNU_ARM)/bin/arm-none-eabi-gcc
ASM     = $(GNU_ARM)/bin/arm-none-eabi-as
LINK    = $(GNU_ARM)/bin/arm-none-eabi-ld
OBJCOPY = $(GNU_ARM)/bin/arm-none-eabi-objcopy

APP_NAME = hypervisor
ARM_CPU  = cortex-a8

CCFLAGS = -g -O3 -fno-common -ffixed-r8 -msoft-float -D__KERNEL__ -DTEXT_BASE=0x8C000000 -fno-builtin -ffreestanding -nostdinc -pipe -DCONFIG_ARM -D__ARM__ -marm -mabi=aapcs-linux -mno-thumb-interwork -march=armv5 -Wall -Wstrict-prototypes -fno-stack-protector -c -fno-toplevel-reorder
ASMFLAGS = -mcpu=$(ARM_CPU)
LINKFLAGS = -g -Ttext 0x8C000000 -e main -o $(APP_NAME).elf -Map $(APP_NAME).map --cref

DEPTH = ..

INCLUDE_DIRS += -I$(DEPTH)/src/
INCLUDE_DIRS += -I$(DEPTH)/src/common/
INCLUDE_DIRS += -I$(DEPTH)/src/cpuArch/
INCLUDE_DIRS += -I$(DEPTH)/src/exceptions/
INCLUDE_DIRS += -I$(DEPTH)/src/guestManager/
INCLUDE_DIRS += -I$(DEPTH)/src/hardware/
INCLUDE_DIRS += -I$(DEPTH)/src/instructionEmu/
INCLUDE_DIRS += -I$(DEPTH)/src/linuxBoot/
INCLUDE_DIRS += -I$(DEPTH)/src/memoryManager/

include $(DEPTH)/src/makefile.in
include $(DEPTH)/src/common/makefile.in
include $(DEPTH)/src/cpuArch/makefile.in
include $(DEPTH)/src/exceptions/makefile.in
include $(DEPTH)/src/hardware/makefile.in
include $(DEPTH)/src/guestManager/makefile.in
include $(DEPTH)/src/linuxBoot/makefile.in
include $(DEPTH)/src/instructionEmu/makefile.in
include $(DEPTH)/src/memoryManager/makefile.in

OBJS = $(addprefix $(DEPTH)/, $(SRCS:.c=.o))
ASM_OBJS = $(addprefix $(DEPTH)/, $(RAW_ASMS:.s=.o))

$(APP_NAME): $(RAW_ASMS) $(SRCS)
	$(LINK) $(LINKFLAGS) $(OBJS) $(ASM_OBJS) -o $(APP_NAME).elf
	$(OBJCOPY) -O binary $(APP_NAME).elf $(APP_NAME).bin


# Rule to build object files from .c sources - $@ contains the current target
$(SRCS):
	@$(CC) $(CCFLAGS) $(INCLUDE_DIRS) -c $(DEPTH)/$@ -o $(DEPTH)/$(@:.c=.o)
	@echo $(@:.c=.o)

# Rule to build object files from .s sources - $@ contains the current target
$(RAW_ASMS):
	@$(ASM) $(ASMFLAGS) $(DEPTH)/$@ -o $(DEPTH)/$(@:.s=.o)
	@echo $(@:.c=.o)

.PHONY: clean
.IGNORE: clean
clean:
	@rm $(OBJS) $(ASM_OBJS) $(APP_NAME).elf $(APP_NAME).map $(APP_NAME).bin 2>/dev/null

.PHONY: backup
backup:
	cd ..; make backup;

