#include <stdio.h>
#include <objects/InputDevice.hpp>
extern "C"
{
    InputDevice *defaultInputDevice = nullptr;
    extern void *stdin __attribute__((alias("defaultInputDevice")));
}

InputDevice *setDefaultInputDevice(InputDevice *device)
{
    auto ret = defaultInputDevice;
    defaultInputDevice = device;
    return ret;
}

InputDevice *getDefaultInputDevice()
{
    return defaultInputDevice;
}
