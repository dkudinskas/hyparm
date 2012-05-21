/*
 * Linker script for the OMAP3530.
 *
 *
 * The alignment constraints enforced here are rigorously documented in:
 * - src/startup.S (for .bss);
 * - src/memoryManager/pageTable.c (in mapHypervisorMemory, for the rest).
 *
 * For general information regarding ELF sections see src/common/linker.h.
 *
 * WARNING: if you change the ordering or the alignment constraints below, you MUST update the code
 *          used to protect the hypervisor's memory in src/memoryManager/{addressing,pageTable}.c.
 */
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
	. = ALIGN(1024K);

	__HYPERVISOR_BEGIN__ = .;

	.text . :
	{
		__TEXT_BEGIN__ = ABSOLUTE(.);
		src/startup.S.o (.text)
		src/debug.S.o (.text)
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

	/*
	 * Align the '.bss' section to a 32-byte boundary to simplify zero-filling (see startup.S).
	 */
	. = ALIGN(32);
	.bss . :
	{
		__DATA_END__ = ABSOLUTE(.);
		__BSS_BEGIN__ = ABSOLUTE(.);
		*(.bss)
		__BSS_END__ = ABSOLUTE(.);
	} > RAM

	/*
	 * Code and statically allocated hypervisor stuff ends here.
	 */
	__HYPERVISOR_END__ = .;

	/*
	 * Executable dynamic allocation pool (for code caches). This pool must be mapped separately,
	 * so its size must be a multiple of the small page size.
	 */
	. = ALIGN(4K);
	__RAM_CODE_CACHE_POOL_BEGIN__ = .;
	. += 8K;
	__RAM_CODE_CACHE_POOL_END__ = .;

	/*
	 * Non-executable dynamic allocation pool. This pool MUST be mapped in sections, so align this
	 * to a 1 MB boundary.
	 */
	. = ALIGN(1M);
	__RAM_XN_POOL_BEGIN__ = .;
	__RAM_XN_POOL_END__ = 0x90000000;
}

