#ifndef _SHAMPOOS_ARCH_SMP_H_
#define _SHAMPOOS_ARCH_SMP_H_
#include <arch/x86_64/PIC/LocalAPIC.h>
#include <shampoos/limits.h>
#include <shampoos/percpu.h>
#define _SHAMPOOS_X86_64_AP_PHY_ADDR_ 0x1000
/*
    we put the ap entry code at 0x1000 when we try to mp set up
*/

void arch_start_smp(struct setup_info *arch_setup_info);
#endif