OUTPUT_ARCH("arm")
OUTPUT_FORMAT("elf32-littlearm")
TARGET("elf32-littlearm")
ENTRY("_start")

MEMORY
{
	RAM (rwx) : ORIGIN = 0x8f000000 LENGTH = 1048576
}

SECTIONS
{
	. = 0x8f000000;
	. = ALIGN(4);
	_start_marker = .;
	.text :
	{
		src/startup.S.o (.text)
		*(.text)
	}
	.rodata :
	{
		*(.rodata)
	}
	.data :
	{
		*(.data)
	}
	.bss :
	{
		*(.bss)
	}
	_end_marker = .;
}

