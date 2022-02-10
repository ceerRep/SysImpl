#include <stdio.h>

#include <typeinfo.h>

#include <objects/OutputDevice.hpp>

extern "C"
{
    void *defaultOutputDevice;
    void *errorOutputDevice;
    extern void *stdout __attribute__((alias("defaultOutputDevice")));
    extern void *stderr __attribute__((alias("errorOutputDevice")));

    int fputc(int c, void *device)
    {
        Object *object = (Object *)device;

        if (auto device = dynamic_cast<OutputDevice *>(object); device)
        {
            return device->putc(c);
        }
        else
        {
            fprintf(stderr, "fputc: expected OutputDevice, got %s\n", typeid(object).name());
            return -c;
        }
    }

    int putchar(int c)
    {
        return fputc(c, stdout);
    }
}

OutputDevice *setDefaultOutputDevice(OutputDevice *output)
{
    OutputDevice *ret = ((OutputDevice *)defaultOutputDevice);
    defaultOutputDevice = output;
    return ret;
}

OutputDevice *setErrorOutputDevice(OutputDevice *output)
{
    OutputDevice *ret = ((OutputDevice *)errorOutputDevice);
    errorOutputDevice = output;
    return ret;
}
