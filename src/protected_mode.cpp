#include <protected_mode.hpp>
#include <kernel_defines.h>
#include <string.h>

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

    tss_array[SEG_GLOBAL_TSS] = new tss_entry_struct_t;
    memset(tss_array[SEG_GLOBAL_TSS], 0, sizeof(tss_entry_struct_t));
    tss_array[SEG_GLOBAL_TSS]->iomap_base = sizeof(tss_entry_struct_t);

    tss_array[SEG_V8086_TSS] = (tss_entry_struct_t *)new tss_entry_struct_with_io_t;
    memset(tss_array[SEG_V8086_TSS], 0, sizeof(tss_entry_struct_with_io_t));
    tss_array[SEG_V8086_TSS]->iomap_base = sizeof(tss_entry_struct_with_io_t);
    tss_array[SEG_V8086_TSS]->esp2 = (uint32_t)(new char[1024]) + 1024; // ring2 is never used, so we store the stack top there
    tss_array[SEG_V8086_TSS]->ss0 = SEG_KERNEL_DATA << 3;
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
    gdts[SEG_KERNEL_CODE].conforming_expand_down = 0; // does not matter for ring 3 as no lower privilege level exists
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

    gdts[SEG_USER_CODE] = gdts[SEG_KERNEL_CODE];
    gdts[SEG_USER_CODE].DPL = 3; // ring 3

    gdts[SEG_USER_DATA] = gdts[SEG_USER_CODE];
    gdts[SEG_USER_DATA].code = 0; // data

    gdts[SEG_GLOBAL_TSS].limit_low = sizeof(tss_entry_struct_t);
    gdts[SEG_GLOBAL_TSS].base_low = (uintptr_t)tss_array[SEG_GLOBAL_TSS];
    gdts[SEG_GLOBAL_TSS].accessed = 1;               // With a system entry (`code_data_segment` = 0), 1 indicates TSS and 0 indicates LDT
    gdts[SEG_GLOBAL_TSS].read_write = 0;             // For a TSS, indicates busy (1) or not busy (0).
    gdts[SEG_GLOBAL_TSS].conforming_expand_down = 0; // always 0 for TSS
    gdts[SEG_GLOBAL_TSS].code = 1;                   // For a TSS, 1 indicates 32-bit (1) or 16-bit (0).
    gdts[SEG_GLOBAL_TSS].code_data_segment = 0;      // indicates TSS/LDT (see also `accessed`)
    gdts[SEG_GLOBAL_TSS].DPL = 0;                    // ring 0, see the comments below
    gdts[SEG_GLOBAL_TSS].present = 1;
    gdts[SEG_GLOBAL_TSS].limit_high = (sizeof(tss_entry_struct_t) & (0xf << 16)) >> 16;           // isolate top nibble
    gdts[SEG_GLOBAL_TSS].available = 0;                                                           // 0 for a TSS
    gdts[SEG_GLOBAL_TSS].long_mode = 0;                                                           //
    gdts[SEG_GLOBAL_TSS].big = 0;                                                                 // should leave zero according to manuals.
    gdts[SEG_GLOBAL_TSS].gran = 0;                                                                // limit is in bytes, not pages
    gdts[SEG_GLOBAL_TSS].base_high = ((uintptr_t)tss_array[SEG_GLOBAL_TSS] & (0xff << 24)) >> 24; // isolate top byte

    gdts[SEG_V8086_TSS] = gdts[SEG_GLOBAL_TSS];
    gdts[SEG_V8086_TSS].limit_low = sizeof(tss_entry_struct_with_io_t);
    gdts[SEG_V8086_TSS].limit_high = (sizeof(tss_entry_struct_with_io_t) & (0xf << 16)) >> 16;
    gdts[SEG_V8086_TSS].base_low = (uintptr_t)tss_array[SEG_V8086_TSS];
    gdts[SEG_V8086_TSS].base_high = ((uintptr_t)tss_array[SEG_V8086_TSS] & (0xff << 24)) >> 24;

    // Initialize gdtr

    gdtr_t *gdtr = new gdtr_t;
    gdtr->limit = sizeof(gdt_entry_bits_t) * GDT_ENTRY_NUM;
    gdtr->base = (uint32_t)(uintptr_t)gdts;

    asm("lgdt %0"
        :
        : "m"(*gdtr)
        :);

    asm(
        "mov %0, %%ds\n\t"
        "mov %0, %%es\n\t"
        "mov %0, %%fs\n\t"
        "mov %0, %%gs\n\t"
        "mov %0, %%ss\n\t"
        :
        : "r"(SEG_KERNEL_DATA << 3));

    struct __attribute__((packed))
    {
        uint32_t addr;
        uint32_t seg;
    } ljmp_target;

    ljmp_target.seg = SEG_KERNEL_CODE << 3;
    ljmp_target.addr = (uint32_t)protected_mode_ljmp_target;

    asm("ljmp *%0"
        :
        : "m"(ljmp_target)
        :);

    asm(".globl protected_mode_ljmp_target\n\t"
        "protected_mode_ljmp_target: nop");

    asm("ltr %%ax\n\t"
        :
        : "a"(SEG_GLOBAL_TSS << 3)
        :);
}
