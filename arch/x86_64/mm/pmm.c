#include <arch/x86_64/boot/arch_setup.h>
#include <arch/x86_64/mm/page_table.h>
#include <arch/x86_64/mm/vmm.h>
#include <arch/x86_64/power_ctrl.h>
#include <modules/log/log.h>
#include <shampoos/error.h>
#include <shampoos/limits.h>
#include <shampoos/mm/buddy_pmm.h>
extern char _start, _end; /*the kernel end virt addr*/
extern u64 L0_table, L1_table, L2_table;
extern struct buddy buddy_pmm;

extern u64 entry_per_bucket[BUDDY_MAXORDER + 1],
	pages_per_bucket[BUDDY_MAXORDER + 1];

static error_t arch_get_memory_regions(struct setup_info *arch_setup_info) {
	struct multiboot_info *mtb_info = GET_MULTIBOOT_INFO(arch_setup_info);
	struct multiboot_mmap_entry *mmap;
	vaddr add_ptr = mtb_info->mmap.mmap_addr + KERNEL_VIRT_OFFSET;
	u64 length = mtb_info->mmap.mmap_length;

	/* check the multiboot header */
	if (!MULTIBOOT_INFO_FLAG_CHECK(mtb_info->flags, MULTIBOOT_INFO_FLAG_MEM) ||
		!MULTIBOOT_INFO_FLAG_CHECK(mtb_info->flags, MULTIBOOT_INFO_FLAG_MMAP)) {
		pr_info("no mem info\n");
		goto arch_init_pmm_error;
	}
	/*generate the memory region info*/
	buddy_pmm.m_regions->region_count = 0;
	for (mmap = (struct multiboot_mmap_entry *)add_ptr;
		 ((vaddr)mmap) < (add_ptr + length);
		 mmap = (struct multiboot_mmap_entry *)((vaddr)mmap + mmap->size +
												sizeof(mmap->size))) {
		if (mmap->addr + mmap->len > BIOS_MEM_UPPER &&
			mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
			if (buddy_pmm.m_regions->memory_regions_insert(mmap->addr,
														   mmap->len)) {
				pr_error("we cannot manager toooo many memory regions\n");
				goto arch_init_pmm_error;
			}
		}
	}
	return 0;
arch_init_pmm_error:
	return -ENOMEM;
}
static void arch_map_buddy_data_space(paddr kernel_phy_start,
									  paddr kernel_phy_end,
									  paddr buddy_phy_start,
									  paddr buddy_phy_end) {
	paddr buddy_phy_start_addr = buddy_phy_start;
	paddr kernel_end_phy_addr_round_up =
		ROUND_UP(kernel_phy_end, MIDDLE_PAGE_SIZE);
	if (buddy_phy_start_addr < kernel_end_phy_addr_round_up)
		buddy_phy_start_addr =
			kernel_end_phy_addr_round_up; /*for we have mapped the 2m align
											 space of kernel*/
	for (; buddy_phy_start_addr < buddy_phy_end;
		 buddy_phy_start_addr += MIDDLE_PAGE_SIZE) {
		/*As pmm and vmm part is not usable now, we still use boot page table*/
		paddr buddy_start_round_down_2m =
			ROUND_DOWN(buddy_phy_start_addr, MIDDLE_PAGE_SIZE);
		arch_set_L2_entry_huge(buddy_start_round_down_2m,
							   KERNEL_PHY_TO_VIRT(buddy_start_round_down_2m),
							   &L2_table, (PDE_P | PDE_RW | PDE_G | PDE_PS));
	}
}
void arch_init_pmm(struct setup_info *arch_setup_info) {

	u64 buddy_total_pages = 0;

	bool can_load_kernel = false;
	paddr kernel_phy_start = KERNEL_VIRT_TO_PHY((vaddr)(&_start));
	paddr kernel_phy_end = KERNEL_VIRT_TO_PHY((vaddr)(&_end));
	paddr buddy_phy_start = ROUND_UP(kernel_phy_end, PAGE_SIZE);
	paddr buddy_phy_end = 0;

	if (arch_get_memory_regions(arch_setup_info) < 0)
		goto arch_init_pmm_error;

	/* calculate the end of the buddy_pmm avaliable_phy_addr */
	calculate_avaliable_phy_addr_end();

	/*calculate how mach the buddy data need*/
	for (int order = 0; order <= BUDDY_MAXORDER; ++order)
		entry_per_bucket[order] = pages_per_bucket[order] = 0;
	calculate_bucket_space();
	for (int order = 0; order <= BUDDY_MAXORDER; ++order)
		buddy_total_pages += pages_per_bucket[order];
	buddy_phy_end = buddy_phy_start + buddy_total_pages * PAGE_SIZE;
	pr_info("buddy start 0x%x end 0x%x\n", buddy_phy_start, buddy_phy_end);

	/*check whether the kernel can put in all*/
	/*try to check an avaliable region to place the kernel and buddy data*/
	/*as the buddy_node is presented by 30bits, it must be put less than
	 * 1G,check it*/
	if (buddy_phy_end > BUDDY_MAX_PHY_END) {
		pr_error("we cannot manager toooo many memory!\n");
		goto arch_init_pmm_error;
	}

	for (int i = 0; i < buddy_pmm.m_regions->region_count; i++) {
		if (buddy_pmm.m_regions->memory_regions_entry_empty(i))
			continue;
		struct region reg = buddy_pmm.m_regions->memory_regions[i];
		paddr sec_start_addr = reg.addr;
		paddr sec_end_addr = sec_start_addr + reg.len;
		/*hint, this end is not reachable,[ sec_end_addr , sec_end_addr) */
		if (sec_start_addr <= kernel_phy_start && sec_end_addr >= buddy_phy_end)
			can_load_kernel = true;
	}
	/*You need to check whether the kernel have been loaded all successfully*/
	if (!can_load_kernel) {
		pr_info("cannot load kernel\n");
		goto arch_init_pmm_error;
	}

	arch_map_buddy_data_space(kernel_phy_start, kernel_phy_end, buddy_phy_start,
							  buddy_phy_end);

	generate_buddy_bucket(kernel_phy_start, kernel_phy_end, buddy_phy_start,
						  buddy_phy_end);

	return;
arch_init_pmm_error:
	arch_shutdown();
}
