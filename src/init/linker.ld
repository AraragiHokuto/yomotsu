ENTRY(__init_start);

SECTIONS
{
	.	= 0x400000;
	_vaddr_base	= .;


	.text	BLOCK(2M) : ALIGN(2M)
	{
		*(.text)
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
