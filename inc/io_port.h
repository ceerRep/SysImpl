#ifndef _IO_PORT_H

#define _IO_PORT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

    static inline void outb(uint16_t port, uint8_t val)
    {
        asm volatile("outb %0, %1"
                     :
                     : "a"(val), "Nd"(port));
        /* There's an outb %al, $imm8  encoding, for compile-time constant port numbers that fit in 8b.  (N constraint).
         * Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
         * The  outb  %al, %dx  encoding is the only option for all other cases.
         * %1 expands to %dx because  port  is a uint16_t.  %w1 could be used if we had the port number a wider C type */
    }

    static inline void io_wait(void)
    {
        outb(0x80, 0);
    }

    static inline uint8_t inb(uint16_t port)
    {
        uint8_t ret;
        asm volatile("inb %1, %0"
                     : "=a"(ret)
                     : "Nd"(port));
        return ret;
    }

    static inline void cpuid(int code, uint32_t *a, uint32_t *d)
    {
        asm volatile("cpuid"
                     : "=a"(*a), "=d"(*d)
                     : "0"(code)
                     : "ebx", "ecx");
    }

    static inline void wrmsr(uint32_t msr_id, uint64_t msr_value)
    {
        asm volatile("wrmsr"
                     :
                     : "c"(msr_id), "A"(msr_value));
    }

    static inline uint64_t rdmsr(uint32_t msr_id)
    {
        uint64_t msr_value;
        asm volatile("rdmsr"
                     : "=A"(msr_value)
                     : "c"(msr_id));
        return msr_value;
    }

#ifdef __cplusplus
}
#endif

#endif
