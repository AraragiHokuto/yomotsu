ENTRY(ig_mb2_entry)
SECTIONS
{
        . = 0xFFFFFFFF80000000;
        _vaddr_base = .;

        . += 0x200000;

        .text ALIGN(4K):
        {
                _kstart                   = .;
                _mb2h_off                 = . - _vaddr_base;
                *(.multiboot2)
		_loader_off = . - _vaddr_base;
                _text_begin               = .;
                *(.text)
		_text_end         = .;
        }

	.rodata ALIGN(4K):
	{
		_rodata_begin	=.;
		*(.rodata)
		_rodata_end	= .;
	}

        .data ALIGN(4K):
        {
                _data_begin                   = .;
		*(.data)
		_data_end = .;
                _loader_end_off               = . - _vaddr_base;
        }

        .bss ALIGN(4K):
        {
                _bss_begin                 = .;
                *(COMMON) * (.bss)_bss_end = .;
                _bss_end_off               = . - _vaddr_base;
        }
        _kend = .;
}
