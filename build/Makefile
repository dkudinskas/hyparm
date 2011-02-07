# Shell to use
SHELL = /bin/sh

# The gcc compiler and linker
CC      = $(CROSS_COMPILE)gcc
AS      = $(CROSS_COMPILE)as
LD      = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

TARGET ?= beagle

ifeq ($(wildcard config.$(TARGET)),) 
  $(error Cannot find config.$(TARGET))
endif

APP_NAME = hypervisor
DEPTH = ..

include $(DEPTH)/build/config.$(TARGET)

ASMFLAGS = $(TARGET_ASMFLAGS)
# -fno-common forces the compiler to put global variables in one data section in the ELF file
CCFLAGS = -g -O3 -fno-common -ffixed-r8 -msoft-float -D__KERNEL__ -DTEXT_BASE=$(TARGET_BASE_ADDRESS) -fno-builtin -ffreestanding -nostdinc -pipe -DCONFIG_ARM -D__ARM__ -marm -mabi=aapcs-linux -mno-thumb-interwork -Wall -Wstrict-prototypes -Winline -fno-stack-protector -c -fno-toplevel-reorder $(TARGET_CCFLAGS)
LDFLAGS = -g -Ttext $(TARGET_BASE_ADDRESS) -e main -o $(APP_NAME).elf -Map $(APP_NAME).map --cref

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

.PHONY: binary
binary: $(APP_NAME).bin

.PHONY: dump
dump: $(APP_NAME).dump

$(APP_NAME).elf: init_once $(RAW_ASMS) $(SRCS)
	$(LD) $(LDFLAGS) $(OBJS) $(ASM_OBJS) -o $(APP_NAME).elf

$(APP_NAME).bin: $(APP_NAME).elf
	$(OBJCOPY) -O binary $(APP_NAME).elf $(APP_NAME).bin

$(APP_NAME).dump: $(APP_NAME).elf
	$(OBJDUMP) -d $(APP_NAME).elf > $(APP_NAME).dump

# Rule to build object files from .c sources - $@ contains the current target
$(SRCS): init_once
	@$(CC) $(CCFLAGS) $(INCLUDE_DIRS) -c $(DEPTH)/$@ -o $(DEPTH)/$(@:.c=.o)
	@echo $(@:.c=.o)

# Rule to build object files from .s sources - $@ contains the current target
$(RAW_ASMS): init_once
	@$(AS) $(ASMFLAGS) $(DEPTH)/$@ -o $(DEPTH)/$(@:.s=.o)
	@echo $(@:.s=.o)

.PHONY: clean
clean:
	@rm $(OBJS) $(ASM_OBJS) $(APP_NAME).dump $(APP_NAME).elf $(APP_NAME).map $(APP_NAME).bin 2>/dev/null || :

.PHONY: help
help:
	@echo TARGET=beagle,tegra250

.PHONY: init_once
init_once:
	@echo --------------------------------------------------------------------------------
	@echo Target: $(TARGET)
	@echo CROSS_COMPILE = $(CROSS_COMPILE)
	@echo --------------------------------------------------------------------------------

