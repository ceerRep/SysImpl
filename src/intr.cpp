#include <panic.h>
#include <protected_mode.hpp>
#include <stdint.h>
#include <stdio.h>

typedef struct
{
    uint16_t isr_low;   // The lower 16 bits of the ISR's address
    uint16_t kernel_cs; // The GDT segment selector that the CPU will load into CS before calling the ISR
    uint8_t reserved;   // Set to zero
    union __attribute__((packed))
    {
        uint8_t attributes; // Type and attributes; see the IDT page
        struct
        {
            unsigned type : 4;
            unsigned zero : 1;
            unsigned dpl : 2;
            unsigned present : 1;
        } __attribute__((packed)) attributes_fields;
    };

    uint16_t isr_high; // The higher 16 bits of the ISR's address
} __attribute__((packed)) idt_entry_t;

static_assert(sizeof(idt_entry_t) == 8);

typedef struct
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idtr_t;

static idt_entry_t idt[256];
static idtr_t idtr;

struct exception_info
{
    const char *name;
    int id;
    bool error_code;
};

exception_info exception_infos[] = {
    {"Divide-by-zero Error", 0, false},
    {"Debug", 1, false},
    {"falsen-maskable Interrupt", 2, false},
    {"Breakpoint", 3, false},
    {"Overflow", 4, false},
    {"Bound Range Exceeded", 5, false},
    {"Invalid Opcode", 6, false},
    {"Device falset Available", 7, false},
    {"Double Fault", 8, true},
    {"Coprocessor Segment Overrun", 9, false},
    {"Invalid TSS", 10, true},
    {"Segment falset Present", 11, true},
    {"Stack-Segment Fault", 12, true},
    {"General Protection Fault", 13, true},
    {"Page Fault", 14, true},
    {"Reserved", 15, false},
    {"x87 Floating-Point Exception", 16, false},
    {"Alignment Check", 17, true},
    {"Machine Check", 18, false},
    {"SIMD Floating-Point Exception", 19, false},
    {"Virtualization Exception", 20, false},
    {"Control Protection Exception", 21, true},
    {"Reserved", 22, false},
    {"Reserved", 23, false},
    {"Reserved", 24, false},
    {"Reserved", 25, false},
    {"Reserved", 26, false},
    {"Reserved", 27, false},
    {"Hypervisor Injection Exception", 28, false},
    {"VMM Communication Exception", 29, true},
    {"Security Exception", 30, true},
    {"Reserved", 31, false}};

template <int N>
__attribute__((interrupt)) void trap_handler(interrupt_frame *frame)
{
    SegmentRegsSetter setter;
    fprintf(stderr, "Exception %s(%d) occured\n", N < 32 ? exception_infos[N].name : "Unknown", N);
    fprintf(stderr, "CS: %08x\nIP: %08x\nEFLAGS: %08x\nESP: %08x\nSS: %08x\n",
            frame->cs,
            frame->eip,
            frame->eflags,
            frame->esp,
            frame->ss);
    panic("Exception occured");
}

template <int N>
__attribute__((interrupt)) void trap_handler_with_error_code(interrupt_frame *frame, uint32_t error_code)
{
    SegmentRegsSetter setter;
    fprintf(stderr, "Exception %s(%d) with error code %x occured\n", N < 32 ? exception_infos[N].name : "Unknown", N, error_code);
    fprintf(stderr, "CS: %08x\nIP: %08x\nEFLAGS: %08x\nESP: %08x\nSS: %08x\n",
            frame->cs,
            frame->eip,
            frame->eflags,
            frame->esp,
            frame->ss);
    panic("Exception occured");
}

__attribute__((interrupt)) void gp_handler(interrupt_frame *frame, uint32_t error_code)
{
    SegmentRegsSetter setter;
    void v8086_gp_handler(interrupt_frame * frame);

    if (frame->eflags & 0x20000) // VM flag
    {
        v8086_gp_handler(frame);
        return;
    }

    fprintf(stderr, "Exception %s(%d) with error code %x occured\n", 13 < 32 ? exception_infos[13].name : "Unknown", 13, error_code);
    fprintf(stderr, "CS: %08x\nIP: %08x\nEFLAGS: %08x\nESP: %08x\nSS: %08x\n",
            frame->cs,
            frame->eip,
            frame->eflags,
            frame->esp,
            frame->ss);
    panic("Exception occured");
}

void idt_set_descriptor(uint8_t vector, unsigned int segsel, void *isr, unsigned int present, unsigned int dpl, unsigned int type)
{
    idt_entry_t *descriptor = &idt[vector];

    descriptor->isr_low = (uint32_t)isr & 0xFFFF;
    descriptor->kernel_cs = segsel; // this value can be whatever offset your kernel code selector is in your GDT
    descriptor->attributes_fields.dpl = dpl;
    descriptor->attributes_fields.present = present;
    descriptor->attributes_fields.type = type;
    descriptor->attributes_fields.zero = 0;
    descriptor->isr_high = (uint32_t)isr >> 16;
    descriptor->reserved = 0;
}

template <int N>
struct SetDefaultISR
{
    static void set()
    {
        if (N < 32 && exception_infos[N].error_code)
            idt_set_descriptor(N, SEGMENT_SELECTOR(SEG_KERNEL_CODE, 0, 0), (void *)(void (*)(interrupt_frame * frame, uint32_t error_code)) trap_handler_with_error_code<N>, 1, 0, IDT_INTERRUPT_32);
        else
            idt_set_descriptor(N, SEGMENT_SELECTOR(SEG_KERNEL_CODE, 0, 0), (void *)(void (*)(interrupt_frame * frame)) trap_handler<N>, 1, 0, IDT_INTERRUPT_32);
        SetDefaultISR<N - 1>::set();
    }
};

template <>
struct SetDefaultISR<0>
{
    static void set()
    {
        idt_set_descriptor(0, SEGMENT_SELECTOR(SEG_KERNEL_CODE, 0, 0), (void *)(void (*)(interrupt_frame * frame)) trap_handler<0>, 1, 0, IDT_INTERRUPT_32);
    }
};

void initialize_intr()
{
    SetDefaultISR<255>::set();

    idt_set_descriptor(13, SEGMENT_SELECTOR(SEG_KERNEL_CODE, 0, 0), (void *)gp_handler, 1, 0, IDT_INTERRUPT_32);
    idt_set_descriptor(0x80, SEGMENT_SELECTOR(SEG_SYSCALL_TSS, 0, 0), nullptr, 1, 3, IDT_TASK);

    idtr.base = (uintptr_t)&idt[0];
    idtr.limit = (uint16_t)sizeof(idt) - 1;
    __asm__ volatile("lidt %0"
                     :
                     : "m"(idtr)); // load the new IDT

    // asm volatile("sti");
}
