#include <panic.h>
#include <stdio.h>
#include <string.h>

#include <protected_mode.hpp>

extern char code16_source_start[0];
extern char code16_source_end[0];
extern char CODE16[0];
extern char CODE16END[0];
extern char CODE16STACKTOP[0];

extern tss_entry_struct_t **tss_array;
extern gdt_entry_bits_t *gdts;

void v8086_init()
{
    if (code16_source_end - code16_source_start > CODE16END - CODE16)
        panic("16bit code too large");

    memcpy(CODE16, code16_source_start, code16_source_end - code16_source_start);
}

void v8086_gp_handler(interrupt_frame *frame)
{
    uint8_t *addr = (uint8_t *)(frame->cs * 16 + frame->eip);
    uint16_t *stack = (uint16_t *)(frame->ss * 16 + frame->esp);

    switch (addr[0])
    {
    case 0xF4: // hlt
        // Exit v8086 mode
        asm volatile("pushfl\n\t"
                     "popl %%eax\n\t"
                     "orl $0x4000, %%eax\n\t" // NT flag, let "iret" return to caller task
                     "pushl %%eax\n\t"
                     "popfl\n\t"
                     "iret\n\t" ::
                         : "memory", "cc");

        return;

    case 0xCD: // int
    {
        uint32_t intno = addr[1];
        uint32_t int_cs = ((uint16_t *)0)[2 * intno + 1];
        uint32_t int_ip = ((uint16_t *)0)[2 * intno];

        // Save regs
        stack[-1] = frame->eflags;
        stack[-2] = frame->cs;
        stack[-3] = frame->eip + 2;

        frame->esp -= 6;

        frame->cs = int_cs;
        frame->eip = int_ip;

        return;
    }

    default:
        break;
    }

    fprintf(stderr,
            "CS: %08x\nIP: %08x\nEFLAGS: %08x\nESP: %08x\nSS: %08x\n",
            frame->cs,
            frame->eip,
            frame->eflags,
            frame->esp,
            frame->ss);
    panic("GP occured in v8086 mode");
}

uint32_t v8086_call(void *func,
                    uint16_t ax,
                    uint16_t bx,
                    uint16_t cx,
                    uint16_t dx,
                    uint16_t si,
                    uint16_t di,
                    uint16_t ds,
                    uint16_t es)
{
    tss_entry_struct_with_io_t &v8086_tss = *(tss_entry_struct_with_io_t *)tss_array[SEG_V8086_TSS];
    v8086_tss.io[sizeof(v8086_tss.io) - 1] = 0xFF;
    v8086_tss.tss.prev_tss = 0;
    asm volatile("movl %%cr3, %%eax\n\t"
                 : "=a"(v8086_tss.tss.cr3)
                 :
                 : "memory", "cc");

    // /* disable interrupt, */ enable v8086, set IOPL=3
    asm volatile("pushfl\n\t"
                 "pop %%eax\n\t"
                 // "andl $0xfffffdff, %%eax\n\t"
                 "orl  $0x23000, %%eax"
                 : "=a"(v8086_tss.tss.eflags)
                 :
                 : "memory", "cc");

    v8086_tss.tss.cs = 0;
    v8086_tss.tss.eip = (uintptr_t)((char *)func - code16_source_start + CODE16);

    v8086_tss.tss.ss = 0;
    v8086_tss.tss.esp = (uintptr_t)CODE16STACKTOP;

    v8086_tss.tss.ds = ds;
    v8086_tss.tss.es = es;

    v8086_tss.tss.eax = ax;
    v8086_tss.tss.ebx = bx;
    v8086_tss.tss.ecx = cx;
    v8086_tss.tss.edx = dx;
    v8086_tss.tss.esi = si;
    v8086_tss.tss.edi = di;

    v8086_tss.tss.prev_tss = 0;
    v8086_tss.tss.ldt = 0;
    v8086_tss.tss.iomap_base = sizeof(tss_entry_struct_t);

    for (int i = 0; i < 32; i++)
        v8086_tss.tss.redirect[i] = 0xFF;

    struct __attribute__((packed))
    {
        uint32_t addr;
        uint32_t seg;
    } lcall_target;

    lcall_target.seg = SEGMENT_SELECTOR(SEG_V8086_TSS, 0, 0);
    lcall_target.addr = 0; // UNUSED

    asm volatile("lcall *%0"
                 :
                 : "m"(lcall_target)
                 : "memory", "cc");

    return v8086_tss.tss.eax;
}
