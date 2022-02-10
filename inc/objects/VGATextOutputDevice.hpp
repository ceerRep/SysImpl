#ifndef _vgabaseoutputdevice_hpp

#define _vgabaseoutputdevice_hpp

#include <protected_mode.hpp>

#include "OutputDevice.hpp"

class VGABaseOutputDevice : public virtual OutputDevice
{
    char *cursor;

public:
    VGABaseOutputDevice() {}

    int putc0(char ch)
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

    virtual int putc(char ch) override
    {
        if (ch == '\n')
            putc0('\r');

        return putc0(ch);
    }
};

#endif
