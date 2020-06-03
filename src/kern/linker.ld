SECTIONS
{
	. = 0xFFFFFFFF80000000;
	_vaddr_base = .;

	. += 2M;

	.text	BLOCK(4K) : ALIGN(4K)
	{
		_kstart		= .;
		_mb2h_off	= . - _vaddr_base;
		*(.multiboot2)
		_loader_off	= . - _vaddr_base;
		_text_begin	= .;
		*(.text)
		_text_end	= .;
	}

	.data	BLOCK(4K) : ALIGN(4K)
	{
		_data_begin	= .;
		*(.rodata)
		*(.data)
		_date_end	= .;
		_loader_end_off	= . - _vaddr_base;
	}

	.bss	BLOCK(4K) : ALIGN(4K)
	{
		_bss_begin	= .;
		*(COMMON)
		*(.bss)
		_bss_end	= .;
		_bss_end_off	= . - _vaddr_base;
	}
	_kend		= .;
}
