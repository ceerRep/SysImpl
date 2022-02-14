#include <stdio.h>

int puts(const char *str)
{
    int ret;
    if ((ret = fputs(str, stdout)) >= 0)
    {
        putchar('\n');
        ret++;
    }
    return ret;
}

int printf(const char *format, ...)
{
    va_list ap;
    int rv;

    va_start(ap, format);
    rv = vfprintf(stdout, format, ap);
    va_end(ap);
    return rv;
}

int fprintf(void *device, const char *format, ...)
{
    va_list ap;
    int rv;

    va_start(ap, format);
    rv = vfprintf(device, format, ap);
    va_end(ap);
    return rv;
}

int vfprintf(void *device, const char *format, va_list ap)
{
    int rv;
    char buffer[128];

    rv = vsnprintf(buffer, 128, format, ap);

    if (rv < 0)
        return rv;

    if (rv > 128 - 1)
        rv = 128 - 1;

    rv = fputs(buffer, device);

    return rv;
}
