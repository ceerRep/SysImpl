#ifndef _PROTECTED_MODE_HPP

#define _PROTECTED_MODE_HPP

#include <stdint.h>

enum
{
    SEG_NULL = 0,
    SEG_KERNEL_CODE,
    SEG_KERNEL_DATA,
    SEG_USER_CODE,
    SEG_USER_DATA,
    SEG_GLOBAL_TSS,
    SEG_V8086_TSS,
    GDT_ENTRY_NUM
};

enum
{
    IDT_TASK = 0x5,
    IDT_INTERRUPT_16 = 0x6,
    IDT_TRAP_16 = 0x7,
    IDT_INTERRUPT_32 = 0xe,
    IDT_TRAP_32 = 0xf
};

struct __attribute__((packed)) gdt_entry_bits_t
{
    unsigned limit_low : 16;
    unsigned base_low : 24;
    unsigned accessed : 1;
    unsigned read_write : 1;             // readable for code, writable for data
    unsigned conforming_expand_down : 1; // conforming for code, expand down for data
    unsigned code : 1;                   // 1 for code, 0 for data
    unsigned code_data_segment : 1;      // should be 1 for everything but TSS and LDT
    unsigned DPL : 2;                    // privilege level
    unsigned present : 1;                //
    unsigned limit_high : 4;             //
    unsigned available : 1;              // only used in software; has no effect on hardware
    unsigned long_mode : 1;              //
    unsigned big : 1;                    // 32-bit opcodes for code, uint32_t stack for data
    unsigned gran : 1;                   // 1 to use 4k page addressing, 0 for byte addressing
    unsigned base_high : 8;
};

struct __attribute__((packed)) tss_entry_struct_t
{
    uint32_t prev_tss; // The previous TSS - with hardware task switching these form a kind of backward linked list.
    uint32_t esp0;     // The stack pointer to load when changing to kernel mode.
    uint32_t ss0;      // The stack segment to load when changing to kernel mode.
                       //
    uint32_t esp1;     // esp and ss 1 and 2 would be used when switching to rings 1 or 2.
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
    char redirect[32];
};

struct __attribute__((packed)) tss_entry_struct_with_io_t
{
    tss_entry_struct_t tss;
    char io[65536 / 8 + 1];
};

struct __attribute__((packed)) interrupt_frame
{
    int32_t eip;
    int32_t cs;
    int32_t eflags;
    int32_t esp;
    int32_t ss;
};

void initialize_gdt();
void initialize_intr();
void initialize_paging();

void v8086_init();

uint32_t v8086_call(void *func,
                    uint16_t ax,
                    uint16_t bx,
                    uint16_t cx,
                    uint16_t dx,
                    uint16_t si,
                    uint16_t di,
                    uint16_t ds,
                    uint16_t es);

#endif
