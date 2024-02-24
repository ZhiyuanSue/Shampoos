#ifndef _SHAMPOOS_IO_H_
#define _SHAMPOOS_IO_H_
#include <shampoos/types.h>

#if _I386_ || _X86_64_
#define _X86_16550A_COM1_BASE_  0x3F8   /*actually using early serial ouput we just need one serial port*/
#define _X86_16550A_COM2_BASE_  0x2F8
static inline u_int8_t  inb(u_int16_t port) __attribute__((always_inline));
static inline u_int16_t inw(u_int16_t port) __attribute__((always_inline));
static inline u_int32_t inl(u_int16_t port) __attribute__((always_inline));

static inline void      outb(u_int16_t port,u_int8_t data) __attribute__((always_inline));
static inline void      outw(u_int16_t port,u_int16_t data) __attribute__((always_inline));
static inline void      outl(u_int16_t port,u_int32_t data) __attribute__((always_inline));


inline u_int8_t  inb(u_int16_t port) 
{
    u_int8_t data;
    __asm__ __volatile__
    (
        "inb %1, %0"
        :"=a"(data)
        :"d"(port)
    );
    return data;
}
inline u_int16_t  inw(u_int16_t port)
{
    u_int16_t data;
    __asm__ __volatile__
    (
        "inw %1, %0"
        :"=a"(data)
        :"d"(port)
    );
    return data;
}
inline u_int32_t  inl(u_int16_t port)
{
    u_int32_t data;
    __asm__ __volatile__
    (
        "inl %1, %0"
        :"=a"(data)
        :"d"(port)
    );
    return data;
}

inline void      outb(u_int16_t port,u_int8_t data)
{
    __asm__ __volatile__
    (
        "outb %0, %1"
        ::"a"(data),"d"(port)
    );
}
inline void      outw(u_int16_t port,u_int16_t data)
{
    __asm__ __volatile__
    (
        "outw %0, %1"
        ::"a"(data),"d"(port)
    );
}
inline void      outl(u_int16_t port,u_int32_t data)
{
    __asm__ __volatile__
    (
        "outl %0, %1"
        ::"a"(data),"d"(port)
    );
}
#endif

#endif