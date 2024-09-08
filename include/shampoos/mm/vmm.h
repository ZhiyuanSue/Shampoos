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

error_t map(paddr vspace_root_paddr, u64 ppn, u64 vpn, int level);
#define MM_COMMON                       \
        void (*init)(struct pmm * pmm); \
        void* (*m_alloc)(size_t Bytes); \
        void (*m_free)(void* p)
struct allocator {
        MM_COMMON;
};

struct vma_struct {
        struct mm_struct* mm;
        struct rb_node vma_rb;
};
struct mm_struct {
        struct rb_root mm_root;
};

struct mm_struct* create_mm();
struct vma_struct* create_vma();
void insert_vma(struct mm_struct* mm, struct vma_struct* vma);
#endif