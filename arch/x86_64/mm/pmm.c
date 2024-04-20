#include <shampoos/mm/pmm.h>
#include <modules/log/log.h>
#include <arch/x86_64/boot/arch_setup.h>
#include <arch/x86_64/power_ctrl.h>
extern char _end;	/*the kernel end virt addr*/
bool check_region(u64* sec_start_addr,u64* sec_end_addr)
{
	bool can_load_kernel=false;
	u64	kernel_end_phy_addr=(u64)(&_end) & KERNEL_VIRT_OFFSET_MASK;
	/*the end must bigger then BIOS_MEM_UPPER*/
	if((*sec_start_addr)<BIOS_MEM_UPPER)
		*sec_start_addr=BIOS_MEM_UPPER;
	/* as we set the memeory layout 
		* the 1 - BIOS_MEM_UPPER is bios.
		* the BIOS_MEM_UPPER - _end is kernel
		* so no need to check some cases,such as the split, etc.
		* */
	if((*sec_start_addr)<=BIOS_MEM_UPPER \
			&& (*sec_end_addr) >= kernel_end_phy_addr)
	{
		can_load_kernel=true;
		*sec_start_addr=kernel_end_phy_addr;
	}
	/* as we decided the section start and end, just build pmm modules
		* here we need to find the size of the entry
		* */
	*sec_start_addr=ROUND_UP((*sec_start_addr),PAGE_SIZE);
	*sec_end_addr=ROUND_DOWN((*sec_end_addr),PAGE_SIZE);
	return can_load_kernel;
}
void arch_init_pmm(struct setup_info* arch_setup_info)
{
	struct multiboot_info* mtb_info = \
		GET_MULTIBOOT_INFO(arch_setup_info);
	struct multiboot_mmap_entry* mmap;
	u64 add_ptr = mtb_info->mmap.mmap_addr + KERNEL_VIRT_OFFSET;
	u64 length = mtb_info->mmap.mmap_length;
	bool can_load_kernel=false;
	u64	total_avaliable_memory=0;
	u64 buddy_record_pages=0;
		/*how many pages that must be used to maintain the buddy, we use static alloc*/

	if( !MULTIBOOT_INFO_FLAG_CHECK(mtb_info->flags,MULTIBOOT_INFO_FLAG_MEM) || \
	 !MULTIBOOT_INFO_FLAG_CHECK(mtb_info->flags,MULTIBOOT_INFO_FLAG_MMAP))
	{
		pr_info("no mem info\n");
		goto arch_init_pmm_error;
	}
	pr_info("the mem lower is 0x%x ,the upper is 0x%x\n",mtb_info->mem.mem_lower,mtb_info->mem.mem_upper);
	

	for(mmap = (struct multiboot_mmap_entry*)add_ptr;
		((u64)mmap) < (add_ptr+length);
		mmap = (struct multiboot_mmap_entry*)((u64)mmap + mmap->size + sizeof(mmap->size)))
	{
		if( mmap->addr+mmap->len<=BIOS_MEM_UPPER || mmap->type != MULTIBOOT_MEMORY_AVAILABLE )
			;
		else{
			/*pr_info("Aviable Mem:size is 0x%x, base_phy_addr is 0x%x, length = 0x%x, type = 0x%x\n"	\
				,mmap->size,mmap->addr,mmap->len,mmap->type);*/
			u64 sec_start_addr=mmap->addr;
			u64 sec_end_addr=sec_start_addr+mmap->len;
					/*remember, this end is not reachable,[ sec_end_addr , sec_end_addr ) */
			if(check_region(&sec_start_addr,&sec_end_addr))
				can_load_kernel=true;
			total_avaliable_memory+=sec_end_addr-sec_start_addr;
		}
	}
	/*You need to check whether the kernel have been loaded all successfully*/
	if(!can_load_kernel)
	{
		pr_info("cannot load kernel\n");
		goto arch_init_pmm_error;
	}
	/*calculate and static alloc the pmm data struct 
	* remember,we must find a avaliable phy region that is large enough to place those management data
	* if we cannot find one , another error
	*/
	pr_info("total avaliable memory are 0x%x\n",total_avaliable_memory);
	
	return;
arch_init_pmm_error:
	arch_shutdown();
}
