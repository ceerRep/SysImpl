#ifndef _stdio_h

#define _stdio_h

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdarg.h>
#include <stddef.h>

    extern void *defaultOutputDevice;

    extern void *stdin;
    extern void *stdout;
    extern void *stderr;

#define stdin stdin
#define stdout stdout
#define stderr stderr

    int putchar(int c);

    int fputc(int c, void *device);

    int puts(const char *s);

    int printf(const char *format, ...);

    int fprintf(void *device, const char *format, ...);
    int vfprintf(void *device, const char *format, va_list ap);
    int vsnprintf(char *buffer, size_t n, const char *format, va_list ap);
    int snprintf(char *buffer, size_t n, const char *format, ...);
    int asprintf(char **bufp, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif
