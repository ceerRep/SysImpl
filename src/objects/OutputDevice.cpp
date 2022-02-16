#include <objects/OutputDevice.hpp>

#include <stdint.h>
#include <stdio.h>

extern "C"
{
    OutputDevice *defaultOutputDevice = nullptr;
    OutputDevice *errorOutputDevice = nullptr;
    shared_ptr<OutputDevice> pdefaultOutputDevice, perrorOutputDevice;
    extern void *stdout __attribute__((alias("defaultOutputDevice")));
    extern void *stderr __attribute__((alias("errorOutputDevice")));
}

shared_ptr<OutputDevice> setDefaultOutputDevice(shared_ptr<OutputDevice> output)
{
    shared_ptr<OutputDevice> ret = pdefaultOutputDevice;
    defaultOutputDevice = output;
    pdefaultOutputDevice = output;
    return ret;
}

shared_ptr<OutputDevice> setErrorOutputDevice(shared_ptr<OutputDevice> output)
{
    shared_ptr<OutputDevice> ret = perrorOutputDevice;
    errorOutputDevice = output;
    perrorOutputDevice = output;
    return ret;
}

shared_ptr<OutputDevice> getDefaultOutputDevice()
{
    return pdefaultOutputDevice;
}

shared_ptr<OutputDevice> getErrorOutputDevice()
{
    return perrorOutputDevice;
}
