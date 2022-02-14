#include <pit.hpp>

#include <8259.hpp>
#include <io_port.h>
#include <kernel_defines.h>
#include <ld_syms.h>
#include <protected_mode.hpp>
#include <resche.hpp>

#include <objects/Process.hpp>

#define CLOCKBASE 0x40          /* I/O base port of clock chip	*/
#define CLOCK0 CLOCKBASE        //
#define CLKCNTL (CLOCKBASE + 3) /* chip CSW I/O port		*/

static int64_t pic_ticks = 0;

int64_t get_tick()
{
    return pic_ticks;
}

void __attribute__((interrupt)) timer_handler(interrupt_frame *frame)
{
    SegmentRegsSetter setter;
    pic_ticks++;
    PIC_sendEOI(0);
    resche_tick(frame);
}

void pit_init()
{
    idt_set_descriptor(IRQ_BASE0, SEGMENT_SELECTOR(SEG_KERNEL_CODE, 0, 0), (void *)&timer_handler, 1, 0, IDT_INTERRUPT_32);
    outb(CLKCNTL, 0x34);

    /* Set the clock rate to 1.190 Mhz; this is 1 ms interrupt rate */

    int intv = 1193; /* Using 1193 instead of 1190 to fix clock skew	*/

    /* Must write LSB first, then MSB */

    outb(CLOCK0, (char)(0xff & intv));
    outb(CLOCK0, (char)(0xff & (intv >> 8)));

    IRQ_clear_mask(0);
}
