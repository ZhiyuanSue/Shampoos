#ifndef _SHAMPOOS_TRAP_H_
#define _SHAMPOOS_TRAP_H_
#include <common/types.h>
#include <arch/aarch64/gic/gic_v2.h>
#include "trap_def.h"

#define AARCH64_IRQ_TO_TRAP_ID(irq_number) (irq_number + AARCH64_IRQ_OFFSET)
#define AARCH64_TRAP_ID_TO_IRQ(trap_id)    (trap_id - AARCH64_IRQ_OFFSET)

#define TRAP_ID(trap_info)  (trap_info & AARCH64_TRAP_ID_MASK)
#define TRAP_SRC(trap_info) (trap_info & AARCH64_TRAP_SRC_MASK)
#define TRAP_CPU(trap_info) \
        ((trap_info & AARCH64_TRAP_CPU_MASK) >> AARCH64_TRAP_CPU_SHIFT)
#define AARCH64_TRAP_GET_SRC_EL(trap_info) \
        ((trap_info & AARCH64_TRAP_SRC_EL_MASK) >> AARCH64_TRAP_SRC_EL_SHIFT)

struct trap_frame {
#define AARCH64_TRAP_SRC_EL_SHIFT (56)
#define AARCH64_TRAP_SRC_EL_MASK  (0xfUL << AARCH64_TRAP_SRC_EL_SHIFT)
#define AARCH64_TRAP_SRC_EL_0     (0UL)
#define AARCH64_TRAP_SRC_EL_1     (1UL)
        u64 trap_info;
        u64 REGS[31];

        u64 SPSR;
        u64 ELR;
        u64 SP;
        u64 ESR;
        u64 FAR;
        u64 HPFAR;
        u64 TPIDR_EL0;
};
#define AARCH64_ESR_EC_MASK (0xffUL << AARCH64_ESR_EC_SHIFT)
#define AARCH64_ESR_GET_EC(esr_value) \
        (((u64)(esr_value)&AARCH64_ESR_EC_MASK) >> AARCH64_ESR_EC_SHIFT)

void arch_init_interrupt(void);
void arch_unknown_trap_handler(struct trap_frame *tf);
void arch_eoi_irq(union irq_source source);
#endif