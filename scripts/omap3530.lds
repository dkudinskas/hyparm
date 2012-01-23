OUTPUT_ARCH("arm")
OUTPUT_FORMAT("elf32-littlearm")
TARGET("elf32-littlearm")
ENTRY("_start")

MEMORY
{
	RAM (rwx) : ORIGIN = 0x8c000000 LENGTH = 16777216
}

SECTIONS
{
	. = 0x8c000000;
	. = ALIGN(4);
	__START_MARKER__ = .;
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
	__END_MARKER__ = .;
}

