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
	.text :
	{
		src/startup.S.o (.text)
		*(.text)
	}
	.data :
	{
		*(.data)
	}
	.bss :
	{
		*(.bss)
	}
}

