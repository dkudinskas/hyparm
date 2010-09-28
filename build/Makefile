# Shell to use
SHELL = /bin/sh

# GNU-ARM tools directory
ifeq ($(CROSS_COMPILE),)
  ifeq ($(USER),kudinsd6)
    GNU_ARM = $(HOME)/ARM/compilers/arm-2009q1/bin/arm-none-eabi-
  else
    GNU_ARM = $(HOME)/arm-dev/compilers/arm-2009q1/bin/arm-none-eabi-
  endif
else
  GNU_ARM = $(CROSS_COMPILE)
endif
# The gcc compiler and linker
CC      = $(GNU_ARM)gcc
ASM     = $(GNU_ARM)as
LINK    = $(GNU_ARM)ld
OBJCOPY = $(GNU_ARM)objcopy

TARGET ?= beagle

ifneq ($(TARGET),beagle)
  ifneq ($(TARGET),tegra250)
    $(error bad target $(TARGET))
  endif
endif

APP_NAME = hypervisor
DEPTH = ..

include $(DEPTH)/build/config.$(TARGET)

ASMFLAGS = $(TARGET_ASMFLAGS)
CCFLAGS = -g -O3 -fno-common -ffixed-r8 -msoft-float -D__KERNEL__ -DTEXT_BASE=$(BASE_ADDRESS) -fno-builtin -ffreestanding -nostdinc -pipe -DCONFIG_ARM -D__ARM__ -marm -mabi=aapcs-linux -mno-thumb-interwork -march=armv5 -Wall -Wstrict-prototypes -fno-stack-protector -c -fno-toplevel-reorder $(TARGET_CCFLAGS)
LINKFLAGS = -g -Ttext $(BASE_ADDRESS) -e main -o $(APP_NAME).elf -Map $(APP_NAME).map --cref

INCLUDE_DIRS += -I$(DEPTH)/src/
INCLUDE_DIRS += -I$(DEPTH)/src/common/
INCLUDE_DIRS += -I$(DEPTH)/src/cpuArch/
INCLUDE_DIRS += -I$(DEPTH)/src/exceptions/
INCLUDE_DIRS += -I$(DEPTH)/src/guestManager/
INCLUDE_DIRS += -I$(DEPTH)/src/hardware/
INCLUDE_DIRS += -I$(DEPTH)/src/instructionEmu/
INCLUDE_DIRS += -I$(DEPTH)/src/linuxBoot/
INCLUDE_DIRS += -I$(DEPTH)/src/memoryManager/
INCLUDE_DIRS += -I$(DEPTH)/src/drivers/beagle/

include $(DEPTH)/src/makefile.in
include $(DEPTH)/src/common/makefile.in
include $(DEPTH)/src/cpuArch/makefile.in
include $(DEPTH)/src/exceptions/makefile.in
include $(DEPTH)/src/hardware/makefile.in
include $(DEPTH)/src/guestManager/makefile.in
include $(DEPTH)/src/linuxBoot/makefile.in
include $(DEPTH)/src/instructionEmu/makefile.in
include $(DEPTH)/src/memoryManager/makefile.in
include $(DEPTH)/src/drivers/beagle/makefile.in

OBJS = $(addprefix $(DEPTH)/, $(SRCS:.c=.o))
ASM_OBJS = $(addprefix $(DEPTH)/, $(RAW_ASMS:.s=.o))

$(APP_NAME): init_once $(RAW_ASMS) $(SRCS)
	$(LINK) $(LINKFLAGS) $(OBJS) $(ASM_OBJS) -o $(APP_NAME).elf
	$(OBJCOPY) -O binary $(APP_NAME).elf $(APP_NAME).bin

# Rule to build object files from .c sources - $@ contains the current target
$(SRCS): init_once
	@$(CC) $(CCFLAGS) $(INCLUDE_DIRS) -c $(DEPTH)/$@ -o $(DEPTH)/$(@:.c=.o)
	@echo $(@:.c=.o)

# Rule to build object files from .s sources - $@ contains the current target
$(RAW_ASMS): init_once
	@$(ASM) $(ASMFLAGS) $(DEPTH)/$@ -o $(DEPTH)/$(@:.s=.o)
	@echo $(@:.c=.o)

.PHONY: clean
.IGNORE: clean
clean:
	@rm $(OBJS) $(ASM_OBJS) $(APP_NAME).elf $(APP_NAME).map $(APP_NAME).bin 2>/dev/null

.PHONY: backup
backup:
	cd ..; make backup;

.PHONY: help
help:
	@echo TARGET=beagle,tegra250

.PHONY: init_once
init_once:
	@echo --------------------------------------------------------------------------------
	@echo Target: $(TARGET)
	@echo CC: $(CC)
	@echo --------------------------------------------------------------------------------
