#ifndef _SHAMPOOS_ARCH_SETUP_H_
#define _SHAMPOOS_ARCH_SETUP_H_
#include <common/types.h>

#ifndef KERNEL_VIRT_OFFSET
#define KERNEL_VIRT_OFFSET 0xffff800000000000
#endif
#define BIOS_MEM_UPPER 0x100000

struct setup_info {
	u32 multiboot_magic;
	u32 multiboot_info_struct_ptr;
	u32 phy_addr_width;
	u32 vir_addr_width;
	u64 log_buffer_addr;
} __attribute__((packed));

#define GET_MULTIBOOT_INFO(setup_info)                                         \
	((struct multiboot_info *)(arch_setup_info->multiboot_info_struct_ptr +    \
							   KERNEL_VIRT_OFFSET))

error_t start_arch(struct setup_info *arch_setup_info);
#endif