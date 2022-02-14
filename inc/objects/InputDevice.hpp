#ifndef _inputdevice_hpp

#define _inputdevice_hpp

#include "Object.hpp"

class InputDevice : virtual public Object
{
public:
    // return -EAGAIN if there's no data
    // if size == 0, check and return -EAGAIN if there's no data
    virtual int64_t read(void *buffer, size_t size) = 0;
};

InputDevice* setDefaultInputDevice(InputDevice* device);
InputDevice* getDefaultInputDevice();

#endif
