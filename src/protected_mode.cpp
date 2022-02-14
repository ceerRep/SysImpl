#include <kernel_defines.h>
#include <protected_mode.hpp>
#include <string.h>
#include <syscall/syscall_handler.hpp>

extern char protected_mode_ljmp_target[0];

struct __attribute__((packed)) gdtr_t
{
    uint16_t limit;
    uint32_t base;
};

tss_entry_struct_t **tss_array;
gdt_entry_bits_t *gdts;

void initialize_tss()
{
    tss_array = new tss_entry_struct_t *[GDT_ENTRY_NUM];

    tss_array[SEG_INIT_TSS] = new tss_entry_struct_t;
    memset(tss_array[SEG_INIT_TSS], 0, sizeof(tss_entry_struct_t));
    tss_array[SEG_INIT_TSS]->iomap_base = sizeof(tss_entry_struct_t);

    tss_array[SEG_USER_TSS] = new tss_entry_struct_t;
    memset(tss_array[SEG_USER_TSS], 0, sizeof(tss_entry_struct_t));
    tss_array[SEG_USER_TSS]->iomap_base = sizeof(tss_entry_struct_t);

    tss_array[SEG_SYSCALL_TSS] = new tss_entry_struct_t;
    memset(tss_array[SEG_SYSCALL_TSS], 0, sizeof(tss_entry_struct_t));
    tss_array[SEG_SYSCALL_TSS]->iomap_base = sizeof(tss_entry_struct_t);
    tss_array[SEG_SYSCALL_TSS]->cs = SEGMENT_SELECTOR(SEG_KERNEL_CODE, 0, 0);
    tss_array[SEG_SYSCALL_TSS]->ds =
        tss_array[SEG_SYSCALL_TSS]->es =
            tss_array[SEG_SYSCALL_TSS]->fs =
                tss_array[SEG_SYSCALL_TSS]->gs =
                    tss_array[SEG_SYSCALL_TSS]->ss = SEGMENT_SELECTOR(SEG_KERNEL_DATA, 0, 0);
    tss_array[SEG_SYSCALL_TSS]->eip = (uintptr_t)&syscall_handler;
    tss_array[SEG_SYSCALL_TSS]->esp = (uint32_t)(new char[8192]) + 8192;
    tss_array[SEG_SYSCALL_TSS]->eflags |= 1 << 9; // enable interrupt

    tss_array[SEG_V8086_TSS] = (tss_entry_struct_t *)new tss_entry_struct_with_io_t;
    memset(tss_array[SEG_V8086_TSS], 0, sizeof(tss_entry_struct_with_io_t));
    tss_array[SEG_V8086_TSS]->iomap_base = sizeof(tss_entry_struct_with_io_t);
    tss_array[SEG_V8086_TSS]->ss0 = SEGMENT_SELECTOR(SEG_KERNEL_DATA, 0, 0);
    tss_array[SEG_V8086_TSS]->esp0 = (uint32_t)(new char[1024]) + 1024; // unlike ss and esp, ss0 and esp0 are static
}

void initialize_gdt()
{
    initialize_tss();

    gdts = new gdt_entry_bits_t[GDT_ENTRY_NUM];

    memset(&gdts[0], 0, sizeof(gdt_entry_bits_t));

    gdts[SEG_KERNEL_CODE].limit_low = 0xFFFF;
    gdts[SEG_KERNEL_CODE].base_low = 0;
    gdts[SEG_KERNEL_CODE].accessed = 0;
    gdts[SEG_KERNEL_CODE].read_write = 1;             // since this is a code segment, specifies that the segment is readable
    gdts[SEG_KERNEL_CODE].conforming_expand_down = 0; // cannot far jump from lower priv.
    gdts[SEG_KERNEL_CODE].code = 1;
    gdts[SEG_KERNEL_CODE].code_data_segment = 1;
    gdts[SEG_KERNEL_CODE].DPL = 0; // ring 0
    gdts[SEG_KERNEL_CODE].present = 1;
    gdts[SEG_KERNEL_CODE].limit_high = 0xF;
    gdts[SEG_KERNEL_CODE].available = 1;
    gdts[SEG_KERNEL_CODE].long_mode = 0;
    gdts[SEG_KERNEL_CODE].big = 1;  // it's 32 bits
    gdts[SEG_KERNEL_CODE].gran = 1; // 4KB page addressing
    gdts[SEG_KERNEL_CODE].base_high = 0;

    gdts[SEG_KERNEL_DATA] = gdts[SEG_KERNEL_CODE];
    gdts[SEG_KERNEL_DATA].code = 0; // data
    gdts[SEG_KERNEL_DATA].conforming_expand_down = 0;

    gdts[SEG_USER_CODE] = gdts[SEG_KERNEL_CODE];
    gdts[SEG_USER_CODE].DPL = 3; // ring 3

    gdts[SEG_USER_DATA] = gdts[SEG_USER_CODE];
    gdts[SEG_USER_DATA].code = 0; // data
    // Lower 32MiB reserved for kernel
    gdts[SEG_USER_DATA].conforming_expand_down = 1; // expand down
    gdts[SEG_USER_DATA].limit_low = 0x1fff;
    gdts[SEG_USER_DATA].limit_high = 0;

    gdts[SEG_INIT_TSS].limit_low = sizeof(tss_entry_struct_t);
    gdts[SEG_INIT_TSS].base_low = (uintptr_t)tss_array[SEG_INIT_TSS];
    gdts[SEG_INIT_TSS].accessed = 1;               // With a system entry (`code_data_segment` = 0), 1 indicates TSS and 0 indicates LDT
    gdts[SEG_INIT_TSS].read_write = 0;             // For a TSS, indicates busy (1) or not busy (0).
    gdts[SEG_INIT_TSS].conforming_expand_down = 0; // always 0 for TSS
    gdts[SEG_INIT_TSS].code = 1;                   // For a TSS, 1 indicates 32-bit (1) or 16-bit (0).
    gdts[SEG_INIT_TSS].code_data_segment = 0;      // indicates TSS/LDT (see also `accessed`)
    gdts[SEG_INIT_TSS].DPL = 0;                    // ring 0, see the comments below
    gdts[SEG_INIT_TSS].present = 1;
    gdts[SEG_INIT_TSS].limit_high = (sizeof(tss_entry_struct_t) & (0xf << 16)) >> 16;         // isolate top nibble
    gdts[SEG_INIT_TSS].available = 0;                                                         // 0 for a TSS
    gdts[SEG_INIT_TSS].long_mode = 0;                                                         //
    gdts[SEG_INIT_TSS].big = 0;                                                               // should leave zero according to manuals.
    gdts[SEG_INIT_TSS].gran = 0;                                                              // limit is in bytes, not pages
    gdts[SEG_INIT_TSS].base_high = ((uintptr_t)tss_array[SEG_INIT_TSS] & (0xff << 24)) >> 24; // isolate top byte

    gdts[SEG_USER_TSS] = gdts[SEG_INIT_TSS];
    gdts[SEG_USER_TSS].DPL = 3;
    gdts[SEG_USER_TSS].base_low = (uintptr_t)tss_array[SEG_USER_TSS];
    gdts[SEG_USER_TSS].base_high = ((uintptr_t)tss_array[SEG_USER_TSS] & (0xff << 24)) >> 24;

    gdts[SEG_SYSCALL_TSS] = gdts[SEG_INIT_TSS];
    gdts[SEG_SYSCALL_TSS].base_low = (uintptr_t)tss_array[SEG_SYSCALL_TSS];
    gdts[SEG_SYSCALL_TSS].base_high = ((uintptr_t)tss_array[SEG_SYSCALL_TSS] & (0xff << 24)) >> 24;
    gdts[SEG_SYSCALL_TSS].DPL = 0; // can not be called directly

    gdts[SEG_V8086_TSS] = gdts[SEG_INIT_TSS];
    gdts[SEG_V8086_TSS].limit_low = sizeof(tss_entry_struct_with_io_t);
    gdts[SEG_V8086_TSS].limit_high = (sizeof(tss_entry_struct_with_io_t) & (0xf << 16)) >> 16;
    gdts[SEG_V8086_TSS].base_low = (uintptr_t)tss_array[SEG_V8086_TSS];
    gdts[SEG_V8086_TSS].base_high = ((uintptr_t)tss_array[SEG_V8086_TSS] & (0xff << 24)) >> 24;

    // Initialize gdtr

    gdtr_t *gdtr = new gdtr_t;
    gdtr->limit = sizeof(gdt_entry_bits_t) * GDT_ENTRY_NUM;
    gdtr->base = (uint32_t)(uintptr_t)gdts;

    asm volatile("lgdt %0"
                 :
                 : "m"(*gdtr)
                 : "memory", "cc");

    asm volatile(
        "mov %0, %%ds\n\t"
        "mov %0, %%es\n\t"
        "mov %0, %%fs\n\t"
        "mov %0, %%gs\n\t"
        "mov %0, %%ss\n\t"
        :
        : "r"(SEGMENT_SELECTOR(SEG_KERNEL_DATA, 0, 0))
        : "memory", "cc");

    struct __attribute__((packed))
    {
        uint32_t addr;
        uint32_t seg;
    } ljmp_target;

    ljmp_target.seg = SEGMENT_SELECTOR(SEG_KERNEL_CODE, 0, 0);
    ljmp_target.addr = (uint32_t)protected_mode_ljmp_target;

    asm volatile("ljmp *%0"
                 :
                 : "m"(ljmp_target)
                 : "memory", "cc");

    asm volatile(".globl protected_mode_ljmp_target\n\t"
                 "protected_mode_ljmp_target: nop" ::
                     : "memory", "cc");

    asm volatile("ltr %%ax\n\t"
                 :
                 : "a"(SEGMENT_SELECTOR(SEG_INIT_TSS, 0, 0))
                 : "memory", "cc");
}
