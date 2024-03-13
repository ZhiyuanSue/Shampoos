	.section .boot
	.global _start
	.align 4
/* some definations */
	.set	kernel_virt_offset,0xffffffff80000000
	.set	boot_stack_size,0x10000
	.set	multiboot_header_magic,0x1BADB002
	.set	multiboot_header_flag_addr,0x00000002
	.set	multiboot_header_flag_vbe,0x00010000
.code32
_start:
	jmp multiboot_entry
	/* Align 32 bits boundary. */
	.align  4
multiboot_header:
	.long	multiboot_header_magic	/*magic*/
	.long	multiboot_header_flag_addr | multiboot_header_flag_vbe	/*flags,only memory info and load address infos*/
	.long	-(multiboot_header_magic + multiboot_header_flag_addr | multiboot_header_flag_vbe )   /*checksum*/
multiboot_header_address:	/*we use bin file, not elf file,so those are all need*/
	.long	multiboot_header - kernel_virt_offset
multiboot_load_address:
	.long	_start - kernel_virt_offset
multiboot_load_end_address:
	.long	_edata - kernel_virt_offset
multiboot_bss_end_address:
	.long	_end - kernel_virt_offset
multiboot_entry_address:
	.long	multiboot_entry - kernel_virt_offset
multiboot_header_end:
multiboot_entry:
	/*disable interrupt*/
	
	/*set sp*/
	movl 	$(boot_stack + boot_stack_size - kernel_virt_offset),%esp
	/*clear the flag register*/
	pushl	$0
	popf
	/*store the multiboot info*/
	movl	%ebp,setup_info
	/*check magic*/

	/*check CPUID whether it support IA-32e mode*/
cpuid_check:
	movl	$0x80000001,%eax	/*check the CPUID */
	cpuid	/*EDX.LM[bit29] must be support*/
	andl	$(1<<29),%edx
	cmp	$0,%edx
	je	ERROR_CPUID

	movl	$0x80000008,%eax	/*check the CPUID*/
	cpuid	/*EAX[7:0] (I think is al) means phy address width, and EAX[15:8] means linear address width*/

calculate_kernel_pages:

change_kernel_page_table:

load_kernel_page_table:


enable_pae:
	/*enable_pae*/

enable_lme:
	/*enable_lme*/


enable_pg:
	/*enable_pg*/

	call	cmain
	jmp 	hlt
hlt:
	call 	cpu_idle
ERROR_CPUID:
	jmp hlt

	
/* some infomation under 32 bit*/	
	.section .boot.data
	.global setup_info
setup_info:
	.long 0
multiboot_error_magic:
	.asciz "ERROR MAGIC NUM"
multiboot_hello:
	.asciz "HELLO SHAMPOOS"
	
	.section .boot_page
	.align 0x1000
boot_page_table_PML4:
	.quad boot_page_table_directory_ptr - kernel_virt_offset
	.zero 8*510
	.quad boot_page_table_directory_ptr - kernel_virt_offset

	.global boot_page_table_directory_ptr
boot_page_table_directory_ptr:
	.quad 0	/*TODO*/
	.zero 8*511

.code64
x86_64_entry:
	call 	cmain
	jmp 	hlt
stack_field:
	.align 0x1000
	.comm boot_stack,boot_stack_size
