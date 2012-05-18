OUTPUT_ARCH("arm")
OUTPUT_FORMAT("elf32-littlearm")
TARGET("elf32-littlearm")
ENTRY("_start")

MEMORY
{
	RAM (rwx) : ORIGIN = 0x8f000000 LENGTH = 16M
}

SECTIONS
{
	. = 0x8f000000;
	. = ALIGN(4K);

	__HYPERVISOR_BEGIN__ = .;

	.text . :
	{
		__TEXT_BEGIN__ = ABSOLUTE(.);
		src/startup.S.o (.text)
		*(.text)
	} > RAM

	. = ALIGN(4K);

	.rodata . :
	{
		__TEXT_END__ = ABSOLUTE(.);
		__RODATA_BEGIN__ = ABSOLUTE(.);
		*(.rodata)
	} > RAM

	. = ALIGN(4K);

	.data . :
	{
		__RODATA_END__ = ABSOLUTE(.);
		__DATA_BEGIN__ = ABSOLUTE(.);
		*(.data)
	} > RAM

	. = ALIGN(32);

	.bss . :
	{
		__DATA_END__ = ABSOLUTE(.);
		__BSS_BEGIN__ = ABSOLUTE(.);
		*(.bss)
		__BSS_END__ = ABSOLUTE(.);
	} > RAM

	__HYPERVISOR_END__ = .;

	__RAM_CODE_CACHE_POOL_END__ = 0x90000000;
	__RAM_CODE_CACHE_POOL_BEGIN__ = __RAM_CODE_CACHE_POOL_END__ - 8K;
	__RAM_XN_POOL_END__ = __RAM_CODE_CACHE_POOL_BEGIN__ - 4K;
	__RAM_XN_POOL_BEGIN__ = ALIGN(4096);
}

