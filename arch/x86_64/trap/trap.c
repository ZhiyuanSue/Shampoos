#include <arch/x86_64/PIC/PIC.h>
#include <arch/x86_64/PIC/APIC.h>
#include <arch/x86_64/sys_ctrl.h>
#include <arch/x86_64/sys_ctrl_def.h>
#include <arch/x86_64/trap.h>
#include <modules/log/log.h>
#include <arch/x86_64/PIC/IRQ.h>

extern u64 trap_vec;
extern enum IRQ_type arch_irq_type;
extern union idt_gate_desc trap_vec_table[IDT_LIMIT];
const char *trap_name_string[TRAP_ARCH_USED + 2] = {
        "Fault:Divide Error\n\0",
        "Fault/Trap:Debug Exception\n\0",
        "Interrupt:NMI Interrupt\n\0",
        "Trap:Breakpoint\n\0",
        "Trap:Overflow\n\0",
        "Fault:BOUND Range exceeded\n\0",
        "Fault:Invalid Opcode\n\0",
        "Fault:Math Coprocessor Device Not Available\n\0",
        "Abort:Double Fault\n\0",
        "Fault:Coprocessor Segment Overrun\n\0",
        "Fault:Invalid TSS\n\0",
        "Fault:Segment Not Present\n\0",
        "Fault:Stack-Segment Fault\n\0",
        "Fault:General Protection\n\0",
        "Fault:Page Fault\n\0",
        "Intel reserved\n\0",
        "Fault:x87 FPU Floating-Point Error\n\0",
        "Fault:Alignment Check\n\0",
        "Abort:Machine Check\n\0",
        "Fault:SIMD Floating-Point Exception\n\0",
        "Fault:Virtualization Exception\n\0",
        /*others*/
        "Intel reserved\n\0",
        /*for trap number between 21-31*/
        "User Defined interrupts\n\0"};
void init_interrupt(void)
{
        struct pseudo_descriptor idtr_desc;
        union desc_selector sel;

        idtr_desc.limit = IDT_LIMIT * sizeof(union idt_gate_desc) - 1;
        idtr_desc.base_addr = (u64)(&trap_vec_table);
        for (int i = 0; i < IDT_LIMIT; ++i) {
                /*generate the IDT table*/
                sel.rpl = KERNEL_PL;
                sel.table_indicator = 0;
                sel.index = 1;
                SET_IDT_GATE(trap_vec_table[i],
                             (vaddr)((u64 *)&trap_vec)[i],
                             sel,
                             KERNEL_PL,
                             IA32E_IDT_GATE_TYPE_INT);
        }
        lidt(&idtr_desc);
}

void trap_handler(void)
{
        pr_info("go into trap handler\n");
}

void time_irq(void)
{
        pr_info("timer interrupt\n");
        if (arch_irq_type == PIC_IRQ) {
                PIC_EOI(0x20);
        } else if (arch_irq_type == xAPIC_IRQ) {
                APIC_EOI();
        }
}