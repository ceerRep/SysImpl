#ifndef _outputdevice_hpp

#define _outputdevice_hpp

#include <stddef.h>

#include "Object.hpp"

class OutputDevice : public virtual Object
{
public:
    virtual int64_t write(const void *data, size_t size) = 0;
};

shared_ptr<OutputDevice> setDefaultOutputDevice(shared_ptr<OutputDevice> output);
shared_ptr<OutputDevice> setErrorOutputDevice(shared_ptr<OutputDevice> output);

shared_ptr<OutputDevice> getDefaultOutputDevice();
shared_ptr<OutputDevice> getErrorOutputDevice();

#endif
