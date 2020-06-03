ENTRY(_start);

SECTIONS
{
	.	= 0x400000;
	_vaddr_base	= .;


	.text	BLOCK(4K) : ALIGN(4K)
	{
		*(.text)
	}

	.data	BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
		*(.data)
	}

	.bss	BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}
}
