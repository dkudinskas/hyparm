#!/bin/sh

${CROSS_COMPILE}gcc -nostdlib -nostdinc -o mmutestL2.elf -Wl,-Ttext=0x80000000 mmutestL2.S
${CROSS_COMPILE}objcopy -O binary mmutestL2.elf mmutestL2.bin
