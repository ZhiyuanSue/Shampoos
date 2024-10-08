#ifndef _SHAMPOOS_VMM_H_
#define _SHAMPOOS_VMM_H_
#include "pmm.h"

#ifdef _AARCH64_
#include <arch/aarch64/mm/vmm.h>
#elif defined _LOONGARCH_
#include <arch/loongarch/mm/vmm.h>
#elif defined _RISCV64_
#include <arch/riscv64/mm/vmm.h>
#elif defined _X86_64_
#include <arch/x86_64/mm/vmm.h>
#else
#include <arch/x86_64/mm/vmm.h>
#endif
#include <common/dsa/rb_tree.h>

void arch_set_L0_entry(paddr p, vaddr v, union L0_entry* pt_addr,
                       ARCH_PFLAGS_t flags);
void arch_set_L1_entry(paddr p, vaddr v, union L1_entry* pt_addr,
                       ARCH_PFLAGS_t flags);
void arch_set_L2_entry(paddr p, vaddr v, union L2_entry* pt_addr,
                       ARCH_PFLAGS_t flags);
void arch_set_L3_entry(paddr p, vaddr v, union L3_entry* pt_addr,
                       ARCH_PFLAGS_t flags);
/*use those functions to set page entry flags for every page entry*/
ARCH_PFLAGS_t arch_decode_flags(int entry_level, ENTRY_FLAGS_t ENTRY_FLAGS);
ENTRY_FLAGS_t arch_encode_flags(int entry_level, ARCH_PFLAGS_t ARCH_PFLAGS);

#define map_pages 0xFFFFFFFFFFE00000
/*
        we use last 2M page as the set of the map used pages virtual addr
        we use one 4K page during the mapping stage
        but consider the multi-core, we use last 2M as this per cpu map page set
        and we think we should not have more than 512 cores
*/
struct map_handler {
        int cpu_id;
        vaddr map_vaddr[4];
        struct pmm* pmm;
};
void init_map(struct map_handler* handler, int cpu_id, struct pmm* pmm);
/*
        kernel might try to mapping one page to a different vspace
        and if the vspace is not exist, it should try to alloc a new one
*/
error_t map(paddr* vspace_root_paddr, u64 ppn, u64 vpn, int level,
            ENTRY_FLAGS_t eflags, struct map_handler* handler);
/*
        here we think the vspace root paddr must exist.
        and we expect the vpn and the page number we need to unmap
*/
error_t unmap(paddr vspace_root_paddr, u64 vpn, struct map_handler* handler);

#define MM_COMMON                                                       \
        struct allocator* (*init)(struct nexus_node * nexus_root);      \
        void* (*m_alloc)(struct allocator * allocator_p, size_t Bytes); \
        void (*m_free)(struct allocator * allocator_p, void* p);        \
        int allocator_id
struct allocator {
        MM_COMMON;
};

#endif