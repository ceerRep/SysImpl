#ifndef _outputdevice_hpp

#define _outputdevice_hpp

#include <stddef.h>

#include "Object.hpp"

class OutputDevice : virtual public Object
{
public:
    virtual int putc(char ch) = 0;

    int write(void *data, size_t size);

    int puts(const char *str);

    int printf(const char *fmt, ...);
};

OutputDevice *setDefaultOutputDevice(OutputDevice *output);
OutputDevice *setErrorOutputDevice(OutputDevice *output);

#endif
