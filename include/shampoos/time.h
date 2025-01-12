#ifndef _SHAMPOOS_TIME_
#define _SHAMPOOS_TIME_
#include <common/types.h>

#ifdef _AARCH64_
#include <arch/aarch64/time.h>
#elif defined _LOONGARCH_

#elif defined _RISCV64_

#elif defined _X86_64_
#include <arch/x86_64/time.h>
#else /*for default config is x86_64*/
#include <arch/x86_64/time.h>
#endif

extern volatile i64 jeffies;
/*in shampoos we only use 64 bit time cnt*/
#define time_after(a, b)     ((i64)b - (i64)a < 0)
#define time_after_eq(a, b)  ((i64)b - (i64)a <= 0)
#define time_before(a, b)    time_after(b, a)
#define time_before_eq(a, b) time_after_eq(b, a)

typedef u64 tick_t;
void shampoos_time_init();
void shampoos_do_time_irq();
#define SYS_TIME_MS_PER_INT 10
void ndelay(u64 ns);
void udelay(u64 us);
void mdelay(u64 ms);
#endif