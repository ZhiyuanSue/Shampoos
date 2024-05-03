#ifndef _SHAMPOOS_ARCH_SETUP_H_
#define _SHAMPOOS_ARCH_SETUP_H_
#include <common/types.h>
#define KERNEL_VIRT_OFFSET 0xffffffffc0000000

struct boot_header{
	u32	code0;
	u32	code1;
	u64	text_offset;
	u64	image_size;
	u64	flags;
	u64	res2;
	u64	res3;
	u64	res4;
	u32	magic;
	u32	res5;
} __attribute__((packed));

struct setup_info{
	u64	dtb_ptr;
	u64 log_buffer_addr;
} __attribute__((packed));

int	start_arch (struct setup_info* arch_setup_info);
#endif