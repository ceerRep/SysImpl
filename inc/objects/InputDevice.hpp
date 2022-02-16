#ifndef _inputdevice_hpp

#define _inputdevice_hpp

#include "Object.hpp"

#include <shared_ptr.hpp>

class InputDevice : public virtual Object
{
public:
    // return -EAGAIN if there's no data
    // if size == 0, check and return -EAGAIN if there's no data, and it shouldn't have side effect
    virtual int64_t read(void *buffer, size_t size) = 0;
};

shared_ptr<InputDevice> setDefaultInputDevice(shared_ptr<InputDevice> device);
shared_ptr<InputDevice> getDefaultInputDevice();

#endif
