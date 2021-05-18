ENTRY(_start);

SECTIONS
{
	.	= 0x400000;
	_vaddr_base	= .;


	.text	: ALIGN(2M)
	{
		*(.text)
	}

	.rodata	: ALIGN(2M)
	{
		PROVIDE_HIDDEN(__preinit_array_start = .);
		KEEP(*(.preinit_array*));
		PROVIDE_HIDDEN(__preinit_array_end = .);

		PROVIDE_HIDDEN(__init_array_start = .);
		KEEP(*SORT(.init_array.*));
		KEEP(*(.init_array*));
		PROVIDE_HIDDEN(__init_array_end = .);

		PROVIDE_HIDDEN(__fini_array_start = .);
		KEEP(*SORT(.fini_array.*));
		KEEP(*(.fini_array*));
		PROVIDE_HIDDEN(__fini_array_end = .);

		*(.rodata)
	}

	.data	: ALIGN(2M)
	{
		*(.data)
	}

	.bss	: ALIGN(2M)
	{
		*(COMMON)
		*(.bss)
	}
}
