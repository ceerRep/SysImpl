#ifndef _earlystageoutputdevice_hpp

#define _earlystageoutputdevice_hpp

#include <ld_syms.h>
#include <string.h>

#include "OutputDevice.hpp"

class EarlyStageOutputDevice : public virtual OutputDevice
{
    char *cursor;

public:
    EarlyStageOutputDevice() : cursor(VGA_TEXT_BASE) {}

    virtual int putc(char ch) override
    {
        while (cursor >= VGA_TEXT_BASE + 80 * 25 * 2)
        {
            memcpy(VGA_TEXT_BASE, VGA_TEXT_BASE + 2 * 80, 80 * 24 * 2);
            memset(VGA_TEXT_BASE + 80 * 24 * 2, 0, 80 * 2);
            cursor -= 80 * 2;
        }

        *(cursor++) = ch;
        *(cursor++) = 0x04;

        return ch;
    }
};

#endif
