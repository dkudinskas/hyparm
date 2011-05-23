
SHELL = /bin/sh


AS      = $(CROSS_COMPILE)as
LD      = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

TARGET_ASMFLAGS = \
  --defsym TARGET_BEAGLE=1 \
  --defsym CONFIG_CPU_ARCH_ARMV7_A=1 \
  --defsym CONFIG_CPU_TI_OMAP_35XX=1 \
  --defsym CONFIG_CPU_HAS_ARM_SEC_EXT=1 \
  -mcpu=cortex-a8

TARGET_BASE_ADDRESS=80300000

ASMFLAGS = $(TARGET_ASMFLAGS)
LDFLAGS1 = -g -Ttext $(TARGET_BASE_ADDRESS) -e main -o testcase1.elf -Map testcase1.map --cref
LDFLAGS2 = -g -Ttext $(TARGET_BASE_ADDRESS) -e main -o testcase2.elf -Map testcase2.map --cref

testcase1:
	@$(AS) $(ASMFLAGS) testcase1.s -o testcase1.o
	@echo $(@:.s=.o)
	$(LD) $(LDFLAGS1) testcase1.o -o testcase1.elf
	$(CROSS_COMPILE)objcopy -O binary -R .note -R .note.gnu.build-id -R .comment -S testcase1.elf Imagetestcase1
	mkimage -A arm -O linux -T kernel -C none -a 0x80008000 -e 0x80008000 -n 'Linux-2.6.28.1' -d Imagetestcase1 uImage1
	$(OBJDUMP) -d testcase1.elf > testcase1.dump

testcase2:
	@$(AS) $(ASMFLAGS) testcase2.s -o testcase2.o
	@echo $(@:.s=.o)
	$(LD) $(LDFLAGS2) testcase2.o -o testcase2.elf
	$(CROSS_COMPILE)objcopy -O binary -R .note -R .note.gnu.build-id -R .comment -S testcase2.elf Imagetestcase2
	mkimage -A arm -O linux -T kernel -C none -a 0x80008000 -e 0x80008000 -n 'Linux-2.6.28.1' -d Imagetestcase2 uImage2
	$(OBJDUMP) -d testcase2.elf > testcase2.dump
all:testcase1 testcase2
clean:
	@rm Imagetestcase1 Imagetestcase2 uImage1 uImage2 testcase2.o testcase1.o testcase2.dump testcase1.dump testcase2.elf testcase1.elf testcase2.map testcase1.map 2>/dev/null || :
