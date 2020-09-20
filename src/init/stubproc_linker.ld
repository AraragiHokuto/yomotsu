ENTRY(_start);

SECTIONS
{
	.	= 0x400000;
	_vaddr_base	= .;


	.text	BLOCK(2M) : ALIGN(2M)
	{
		*(.text)
	}

	.preinit_array :
	{
		PROVIDE_HIDDEN(__preinit_array_start = .);
		KEEP(*(.preinit_array*));
		PROVIDE_HIDDEN(__preinit_array_end = .);
	}

	.init_array :
	{
		PROVIDE_HIDDEN(__init_array_start = .);
		KEEP(*SORT(.init_array.*));
		KEEP(*(.init_array*));
		PROVIDE_HIDDEN(__init_array_end = .);
	}

	.fini_array :
	{
		PROVIDE_HIDDEN(__fini_array_start = .);
		KEEP(*SORT(.fini_array.*));
		KEEP(*(.fini_array*));
		PROVIDE_HIDDEN(__fini_array_end = .);
	}

	.data	BLOCK(2M) : ALIGN(2M)
	{
		*(.rodata)
		*(.data)
	}

	.bss	BLOCK(2M) : ALIGN(2M)
	{
		*(COMMON)
		*(.bss)
	}
}
