#ifndef _KERNEL_DEFINES_H

#define _KERNEL_DEFINES_H

#define SERIAL_PORT 0x3f8 // COM1
#define IRQ_BASE0 0x20
#define IRQ_BASE1 0x28
#define PROCESS_TICKS 30

#ifdef __cplusplus

static constexpr inline unsigned long long operator""_k(unsigned long long x) { return x * 1024; }

#endif

#endif
