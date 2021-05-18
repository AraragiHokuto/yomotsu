ENTRY(__init_start);

SECTIONS
{
	.	= 0x400000;
	_vaddr_base	= .;


	.text	: ALIGN(2M)
	{
		*(.text)
	}

	.rodata : ALIGN(2M)
	{
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
