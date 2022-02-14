#ifndef _outputdevice_hpp

#define _outputdevice_hpp

#include <stddef.h>

#include "Object.hpp"

class OutputDevice : virtual public Object
{
public:
    virtual int64_t write(const void *data, size_t size) = 0;
};

OutputDevice *setDefaultOutputDevice(OutputDevice *output);
OutputDevice *setErrorOutputDevice(OutputDevice *output);

OutputDevice *getDefaultOutputDevice();
OutputDevice *getErrorOutputDevice();

#endif
