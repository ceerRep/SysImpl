#include <stdio.h>
#include <shared_ptr.hpp>
#include <objects/InputDevice.hpp>
extern "C"
{
    InputDevice *defaultInputDevice = nullptr;
    shared_ptr<InputDevice> pdefaultInputDevice;
    extern void *stdin __attribute__((alias("defaultInputDevice")));
}

shared_ptr<InputDevice> setDefaultInputDevice(shared_ptr<InputDevice> device)
{
    auto ret = pdefaultInputDevice;
    pdefaultInputDevice = device;
    defaultInputDevice = device;
    return ret;
}

shared_ptr<InputDevice> getDefaultInputDevice()
{
    return pdefaultInputDevice;
}
