#include <stdio.h>
#include <string.h>

#include <typeinfo.h>

#include <objects/OutputDevice.hpp>

extern "C"
{
    int fputs(const char *s, void *device)
    {
        Object *object = (Object *)device;

        size_t size = strlen(s);

        if (auto device = dynamic_cast<OutputDevice *>(object); device)
        {
            return device->write(s, size);
        }
        else
        {
            fprintf(stderr, "fputs: expected OutputDevice, got %s\n", typeid(object).name());
            return EOF;
        }
    }

    int fputc(int c, void *device)
    {
        Object *object = (Object *)device;
        char ch = c;

        if (auto device = dynamic_cast<OutputDevice *>(object); device)
        {
            return device->write(&c, 1);
        }
        else
        {
            fprintf(stderr, "fputc: expected OutputDevice, got %s\n", typeid(object).name());
            return EOF;
        }
    }

    int putchar(int c)
    {
        return fputc(c, stdout);
    }
}
