ENTRY(_start)
OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
SECTIONS {
	kernel_virt_offset = 0xffffffffc0000000 ;
	kernel_start_offset = 0x100000 ;
	. = kernel_virt_offset + kernel_start_offset;
	_kstart = .;
	_start = .;
	.text : 
	{
		*(.boot)
		*(.boot_64)
		*(.text .text.*)
		*(.trap .trap.*)
	}
	. = ALIGN(4K);
	_sdata = .;
	.data :
	{
		*(.boot.page)
		*(.boot.log)
		*(.boot.data)
		*(.boot_stack)
		*(.data .data.*)
	}
	_edata = .;
	. = ALIGN(4K);
	_bss_start = .;
	.bss :
	{
		*(.bss .bss.*)
	}
	_bss_end = .;
	_end = .;
}
