#include <objects/VGATextIODevice.hpp>

#include <objects/BlockInputDevice.hpp>
#include <objects/LineBufferedIOMixin.hpp>

#include <8086.h>
#include <8259.hpp>
#include <errno.h>
#include <kernel_defines.h>
#include <protected_mode.hpp>
#include <stdio.h>

static shared_ptr<VGATextIODevice> vga_device;

extern "C" int kbd_raw_getc();

#define KBD_BUFFER_SIZE 64
static uint8_t kbd_buffer[KBD_BUFFER_SIZE];
static int kbd_buffer_start = 0, kbd_buffer_end = 0;

__attribute__((interrupt)) void irq1_handler(interrupt_frame *frame)
{
    SegmentRegsSetter setter;

    int ch;

    while ((ch = kbd_raw_getc()) > 0)
    {
        if ((kbd_buffer_end + 1) % KBD_BUFFER_SIZE != kbd_buffer_start)
        {
            kbd_buffer[kbd_buffer_end] = ch;
            kbd_buffer_end = (kbd_buffer_end + 1) % KBD_BUFFER_SIZE;
        }
    }

    PIC_sendEOI(1);
}

VGATextIODevice::VGATextIODevice()
{
    idt_set_descriptor(IRQ_BASE0 + 1, SEGMENT_SELECTOR(SEG_KERNEL_CODE, 0, 0), (void *)irq1_handler, 1, 0, IDT_INTERRUPT_32);
    IRQ_clear_mask(1);
}

int VGATextIODevice::putc0(char ch)
{
    v8086_call(code16_source_int10,
               (0x0e << 8) + ch,
               0,
               0,
               0,
               0,
               0,
               0,
               0);

    return ch;
}

void VGATextIODevice::backspace()
{
    write("\x08 \x08", 3);
}

int VGATextIODevice::empty()
{
    return kbd_buffer_start == kbd_buffer_end;
}

int VGATextIODevice::getc0()
{
    if (empty())
        return EOF;

    int ch = kbd_buffer[kbd_buffer_start];

    kbd_buffer_start = (kbd_buffer_start + 1) % KBD_BUFFER_SIZE;

    if (ch == '\r')
        ch = '\n';

    return ch;
}

int64_t VGATextIODevice::write(const void *data, size_t size)
{
    if (gdts[SEG_V8086_TSS].read_write) // v8086 tss is busy, fallback
    {
        return EarlyStageOutputDevice::write(data, size);
    }
    else
    {
        const char *cdata = (const char *)data;

        for (size_t i = 0; i < size; i++)
        {
            if (cdata[i] == '\n')
                putc0('\r');
            putc0(cdata[i]);
        }

        return size;
    }
}

int64_t VGATextIODevice::read(void *buffer, size_t size)
{
    if (empty())
        return -EAGAIN;

    size_t i = 0;
    for (; i < size; i++)
    {
        int ch = getc0();
        if (ch != EOF)
            ((char *)buffer)[i] = ch;
        else
            break;
    }

    return i;
}

shared_ptr<VGATextIODevice> VGATextIODevice::getInstance()
{
    if (!vga_device)
        vga_device = make_shared<BlockInputDeviceMixin<LineBufferedIOMixin<VGATextIODevice>>>().cast<VGATextIODevice>();
    return vga_device;
}
