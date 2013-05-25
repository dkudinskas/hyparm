APP_NAME = booter

CFLAGS   = -Wall
LDFLAGS  = 

CC       = $(CROSS_COMPILE)gcc
AS       = $(CROSS_COMPILE)as
LD       = $(CROSS_COMPILE)ld
OBJCOPY  = $(CROSS_COMPILE)objcopy
OBJDUMP  = $(CROSS_COMPILE)objdump

SRCS_C  = booter.c image.c loader.c
SRCS_S  = callKernel.S
OBJECTS = $(SRCS_C) $(SRCS_S)

.PHONY: all

all: $(APP_NAME)

$(APP_NAME):  $(OBJECTS) 
	$(CC) $(OBJECTS) $(LDFLAGS) -o $(APP_NAME).elf -mcpu=cortex-a8
	$(OBJCOPY) $(APP_NAME).elf $(APP_NAME).bin

clean:
	rm $(APP_NAME).elf 2> /dev/null || :
	rm $(APP_NAME).bin 2> /dev/null || :

