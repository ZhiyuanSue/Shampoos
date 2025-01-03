#include <arch/x86_64/PIC/APIC.h>
#include <shampoos/mm/map_handler.h>
#include <arch/x86_64/PIC/IRQ.h>
#include <common/types.h>
#include <arch/x86_64/cpuid.h>
#include <shampoos/mm/vmm.h>
#include <modules/log/log.h>
#include <common/bit.h>
#include <arch/x86_64/msr.h>
#include <arch/x86_64/sys_ctrl.h>
extern struct cpuinfo_x86 cpuinfo;
extern struct map_handler Map_Handler;
inline bool xAPIC_support(void)
{
        return (cpuinfo.feature_2 & X86_CPUID_FEATURE_EDX_APIC);
}
inline bool x2APIC_support(void)
{
        return (cpuinfo.feature_1 & X86_CPUID_FEATURE_ECX_x2APIC);
}
inline bool TSC_DDL_support(void)
{
        return (cpuinfo.feature_1 & X86_CPUID_FEATURE_ECX_TSC_Deadline);
}
inline bool ARAT_support(void)
{
        /*
                TODO: cpuid is 0x06 and eax.bit2 == 1 means the apic timer will
           not be changed regardless of P-state
        */
}
inline void enable_xAPIC(void)
{
        u64 APIC_BASE_val;

        APIC_BASE_val = rdmsr(IA32_APIC_BASE_addr);
        APIC_BASE_val = set_mask(APIC_BASE_val, IA32_APIC_BASE_X_ENABLE);
        wrmsr(IA32_APIC_BASE_addr, APIC_BASE_val);
}
inline void enable_x2APIC(void)
{
        u64 APIC_BASE_val;

        APIC_BASE_val = rdmsr(IA32_APIC_BASE_addr);
        APIC_BASE_val =
                set_mask(APIC_BASE_val,
                         IA32_APIC_BASE_X_ENABLE | IA32_APIC_BASE_X2_ENABLE);
        wrmsr(IA32_APIC_BASE_addr, APIC_BASE_val);
}
inline void disable_APIC(void)
{
        u64 APIC_BASE_val;

        APIC_BASE_val = rdmsr(IA32_APIC_BASE_addr);
        APIC_BASE_val = clear_mask(APIC_BASE_val,
                                   (IA32_APIC_BASE_X_ENABLE
                                    & IA32_APIC_BASE_X2_ENABLE));
        wrmsr(IA32_APIC_BASE_addr, APIC_BASE_val);
}
void reset_xAPIC_LDR(void)
{
        // DFR is not used in x2APIC mode, and LDR is read only in x2APIC mode
        xAPIC_WR_REG(DFR, KERNEL_VIRT_OFFSET, 0xFFFFFFFF);
        u32 ldr_val = APIC_RD_REG(LDR, KERNEL_VIRT_OFFSET);
        ldr_val = set_mask(ldr_val, 0xFFFFFF);
        ldr_val = ldr_val | 1;
        xAPIC_WR_REG(LDR, KERNEL_VIRT_OFFSET, ldr_val);
}
void reset_APIC(void)
{
        APIC_WR_REG(LVT_TIME, KERNEL_VIRT_OFFSET, APIC_LVT_MASKED);
        APIC_WR_REG(LVT_PERF, KERNEL_VIRT_OFFSET, APIC_LVT_DEL_MODE_NMI);
        APIC_WR_REG(LVT_LINT_0, KERNEL_VIRT_OFFSET, APIC_LVT_MASKED);
        APIC_WR_REG(LVT_LINT_0, KERNEL_VIRT_OFFSET, APIC_LVT_MASKED);
        APIC_WR_REG(TPR, KERNEL_VIRT_OFFSET, 0);
}
void software_enable_APIC(void)
{
        u32 spurious_vec_reg_val;
        u32 spurious_vec_irq_num = _8259A_MASTER_IRQ_NUM_ + _8259A_LPT_1_;
        spurious_vec_reg_val = APIC_RD_REG(SVR, KERNEL_VIRT_OFFSET);

        spurious_vec_reg_val =
                set_mask(spurious_vec_reg_val, APIC_SVR_SW_ENABLE);
        spurious_vec_reg_val =
                clear_mask(spurious_vec_reg_val, APIC_SVR_VEC_MASK);
        spurious_vec_reg_val =
                set_mask(spurious_vec_reg_val, spurious_vec_irq_num);

        APIC_WR_REG(SVR, KERNEL_VIRT_OFFSET, spurious_vec_reg_val);
}
bool map_LAPIC(void)
{
        if (arch_irq_type != xAPIC_IRQ)
                return false; /*only xAPIC need mmio*/
        paddr vspace_root = get_current_kernel_vspace_root();
        paddr lapic_phy_page = xAPIC_MMIO_BASE;
        vaddr lapic_virt_page = KERNEL_PHY_TO_VIRT(xAPIC_MMIO_BASE);
        if (map(&vspace_root,
                PPN(lapic_phy_page),
                VPN(lapic_virt_page),
                3,
                PAGE_ENTRY_UNCACHED, // the LAPIC should be set as uncached
                &Map_Handler)) {
                pr_error("[ LAPIC ] ERROR: map error\n");
                return false;
        }
        return true;
}
u64 isr_reg[8] = {
        APIC_REG_ISR_0,
        APIC_REG_ISR_1,
        APIC_REG_ISR_2,
        APIC_REG_ISR_3,
        APIC_REG_ISR_4,
        APIC_REG_ISR_5,
        APIC_REG_ISR_6,
        APIC_REG_ISR_7,
};
u64 tmr_reg[8] = {
        APIC_REG_TMR_0,
        APIC_REG_TMR_1,
        APIC_REG_TMR_2,
        APIC_REG_TMR_3,
        APIC_REG_TMR_4,
        APIC_REG_TMR_5,
        APIC_REG_TMR_6,
        APIC_REG_TMR_7,
};
u64 irr_reg[8] = {
        APIC_REG_IRR_0,
        APIC_REG_IRR_1,
        APIC_REG_IRR_2,
        APIC_REG_IRR_3,
        APIC_REG_IRR_4,
        APIC_REG_IRR_5,
        APIC_REG_IRR_6,
        APIC_REG_IRR_7,
};
bool lapic_get_vec(int bit, enum lapic_vec_type t)
{
        if (!(bit >= 0 && bit <= 127))
                return 0;
        int index = bit / APIC_REG_SIZE;
        int offset = bit % APIC_REG_SIZE;
        u32 reg_val = 0;

        if (arch_irq_type == xAPIC_IRQ) {
                if (t == lapic_isr_type) {
                        reg_val = (*((u32 *)(xAPIC_REG_PHY_ADDR(isr_reg[index])
                                             + KERNEL_VIRT_OFFSET)));
                } else if (t == lapic_tmr_type) {
                        reg_val = (*((u32 *)(xAPIC_REG_PHY_ADDR(tmr_reg[index])
                                             + KERNEL_VIRT_OFFSET)));
                } else if (t == lapic_irr_type) {
                        reg_val = (*((u32 *)(xAPIC_REG_PHY_ADDR(irr_reg[index])
                                             + KERNEL_VIRT_OFFSET)));
                } else {
                        return 0;
                }
        } else if (arch_irq_type == x2APIC_IRQ) {
                if (t == lapic_isr_type) {
                        reg_val = rdmsr(x2APIC_REG_ADDR(isr_reg[index]));
                } else if (t == lapic_tmr_type) {
                        reg_val = rdmsr(x2APIC_REG_ADDR(tmr_reg[index]));
                } else if (t == lapic_irr_type) {
                        reg_val = rdmsr(x2APIC_REG_ADDR(irr_reg[index]));
                } else {
                        return 0;
                }
        }
        return (reg_val & (1 << offset)) != 0;
}
void lapic_set_vec(int bit, enum lapic_vec_type t)
{
        if (!(bit >= 0 && bit <= 127))
                return;
        int index = bit / APIC_REG_SIZE;
        int offset = bit % APIC_REG_SIZE;
        u32 reg_val = 0;
        if (arch_irq_type == xAPIC_IRQ) {
                if (t == lapic_isr_type) {
                        reg_val = (*((u32 *)(xAPIC_REG_PHY_ADDR(isr_reg[index])
                                             + KERNEL_VIRT_OFFSET)));
                        reg_val = set_bit(reg_val, offset);
                        (*((u32 *)(xAPIC_REG_PHY_ADDR(isr_reg[index])
                                   + KERNEL_VIRT_OFFSET))) = reg_val;
                } else if (t == lapic_tmr_type) {
                        reg_val = (*((u32 *)(xAPIC_REG_PHY_ADDR(tmr_reg[index])
                                             + KERNEL_VIRT_OFFSET)));
                        reg_val = set_bit(reg_val, offset);
                        (*((u32 *)(xAPIC_REG_PHY_ADDR(tmr_reg[index])
                                   + KERNEL_VIRT_OFFSET))) = reg_val;
                } else if (t == lapic_irr_type) {
                        reg_val = (*((u32 *)(xAPIC_REG_PHY_ADDR(irr_reg[index])
                                             + KERNEL_VIRT_OFFSET)));
                        reg_val = set_bit(reg_val, offset);
                        (*((u32 *)(xAPIC_REG_PHY_ADDR(irr_reg[index])
                                   + KERNEL_VIRT_OFFSET))) = reg_val;
                } else {
                        return;
                }
        } else if (arch_irq_type == x2APIC_IRQ) {
                if (t == lapic_isr_type) {
                        reg_val = rdmsr(x2APIC_REG_ADDR(isr_reg[index]));
                        reg_val = set_bit(reg_val, offset);
                        wrmsr(x2APIC_REG_ADDR(isr_reg[index]), reg_val);
                } else if (t == lapic_tmr_type) {
                        reg_val = rdmsr(x2APIC_REG_ADDR(tmr_reg[index]));
                        reg_val = set_bit(reg_val, offset);
                        wrmsr(x2APIC_REG_ADDR(tmr_reg[index]), reg_val);
                } else if (t == lapic_irr_type) {
                        reg_val = rdmsr(x2APIC_REG_ADDR(irr_reg[index]));
                        reg_val = set_bit(reg_val, offset);
                        wrmsr(x2APIC_REG_ADDR(irr_reg[index]), reg_val);
                } else {
                        return;
                }
        }
}
void lapci_clear_vec(int bit, enum lapic_vec_type t)
{
        if (!(bit >= 0 && bit <= 127))
                return;
        int index = bit / APIC_REG_SIZE;
        int offset = bit % APIC_REG_SIZE;
        u32 reg_val = 0;
        if (arch_irq_type == xAPIC_IRQ) {
                if (t == lapic_isr_type) {
                        reg_val = (*((u32 *)(xAPIC_REG_PHY_ADDR(isr_reg[index])
                                             + KERNEL_VIRT_OFFSET)));
                        reg_val = clear_bit(reg_val, offset);
                        (*((u32 *)(xAPIC_REG_PHY_ADDR(isr_reg[index])
                                   + KERNEL_VIRT_OFFSET))) = reg_val;
                } else if (t == lapic_tmr_type) {
                        reg_val = (*((u32 *)(xAPIC_REG_PHY_ADDR(tmr_reg[index])
                                             + KERNEL_VIRT_OFFSET)));
                        reg_val = clear_bit(reg_val, offset);
                        (*((u32 *)(xAPIC_REG_PHY_ADDR(tmr_reg[index])
                                   + KERNEL_VIRT_OFFSET))) = reg_val;
                } else if (t == lapic_irr_type) {
                        reg_val = (*((u32 *)(xAPIC_REG_PHY_ADDR(irr_reg[index])
                                             + KERNEL_VIRT_OFFSET)));
                        reg_val = clear_bit(reg_val, offset);
                        (*((u32 *)(xAPIC_REG_PHY_ADDR(irr_reg[index])
                                   + KERNEL_VIRT_OFFSET))) = reg_val;
                } else {
                        return;
                }
        } else if (arch_irq_type == x2APIC_IRQ) {
                if (t == lapic_isr_type) {
                        reg_val = rdmsr(x2APIC_REG_ADDR(isr_reg[index]));
                        reg_val = clear_bit(reg_val, offset);
                        wrmsr(x2APIC_REG_ADDR(isr_reg[index]), reg_val);
                } else if (t == lapic_tmr_type) {
                        reg_val = rdmsr(x2APIC_REG_ADDR(tmr_reg[index]));
                        reg_val = clear_bit(reg_val, offset);
                        wrmsr(x2APIC_REG_ADDR(tmr_reg[index]), reg_val);
                } else if (t == lapic_irr_type) {
                        reg_val = rdmsr(x2APIC_REG_ADDR(irr_reg[index]));
                        reg_val = clear_bit(reg_val, offset);
                        wrmsr(x2APIC_REG_ADDR(irr_reg[index]), reg_val);
                } else {
                        return;
                }
        }
}
void APIC_EOI()
{
        APIC_WR_REG(EOI, KERNEL_VIRT_OFFSET, 0);
}