#include <shampoos/common.h>
#include <shampoos/mm/buddy_pmm.h>
#include <shampoos/mm/map_handler.h>
#include <shampoos/mm/nexus.h>
#include <shampoos/mm/spmalloc.h>
#include <shampoos/percpu.h>
extern struct buddy buddy_pmm;
extern int BSP_ID;
DEFINE_PER_CPU(struct map_handler, Map_Handler);
DEFINE_PER_CPU(struct nexus_node *, nexus_root);
error_t phy_mm_init(struct setup_info *arch_setup_info)
{
        // memory part init
        buddy_pmm.pmm_init(arch_setup_info);
        return 0;
}
error_t virt_mm_init(int cpu_id)
{
        init_map(&per_cpu(Map_Handler, cpu_id),
                 cpu_id,
                 (struct pmm *)&buddy_pmm);
        per_cpu(nexus_root, cpu_id) = init_nexus(&per_cpu(Map_Handler, cpu_id));
        sp_init(per_cpu(nexus_root, cpu_id),
                per_cpu(Map_Handler, cpu_id).cpu_id);
        return 0;
}