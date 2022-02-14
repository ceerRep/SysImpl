#ifndef _vgabaseoutputdevice_hpp

#define _vgabaseoutputdevice_hpp

#include "8086.h"
#include <protected_mode.hpp>

#include "OutputDevice.hpp"
#include "EarlyStageOutputDevice.hpp"

class VGABaseOutputDevice : public virtual EarlyStageOutputDevice
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

    virtual int64_t write(const void *data, size_t size) override
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
};

#endif
