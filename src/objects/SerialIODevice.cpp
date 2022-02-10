#include <objects/SerialIODevice.hpp>

#include <io_port.h>

SerialIODevice::SerialIODevice(int port) : port(port)
{
    outb(port + 1, 0x00); // Disable all interrupts
    outb(port + 3, 0x80); // Enable DLAB (set baud rate divisor)
    outb(port + 0, 0x01); // Set divisor to 1 (lo byte) 115200 baud
    outb(port + 1, 0x00); //                  (hi byte)
    outb(port + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(port + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
    outb(port + 4, 0x0B); // IRQs enabled, RTS/DSR set
    outb(port + 4, 0x1E); // Set in loopback mode, test the serial chip
    outb(port + 0, 0x5A); // Test serial chip (send byte 0xAE and check if serial returns same byte)

    // Check if serial is faulty (i.e: not same byte as sent)
    if (inb(port + 0) != 0x5A)
    {
        throw SerialIOInitializeError();
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(port + 4, 0x0F);
}

int SerialIODevice::empty()
{
    return inb(port + 5) & 1;
}

int SerialIODevice::getc()
{
    while (empty() == 0)
        ;

    return inb(port);
}

int SerialIODevice::is_transmit_empty()
{
    return inb(port + 5) & 0x20;
}

int SerialIODevice::putc0(char ch)
{
    while (is_transmit_empty() == 0)
        ;

    outb(port, ch);

    return ch;
}

int SerialIODevice::putc(char ch)
{
    if (ch == '\n')
        putc0('\r');

    return putc0(ch);
}
