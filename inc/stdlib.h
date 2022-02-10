#ifndef _stdlib_h

#define _stdlib_h

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>

    void *calloc(size_t count, size_t size);

    void free(void *ptr);

    void *malloc(size_t size);

    void *realloc(void *ptr, size_t size);

    void abort(void) __attribute__((noreturn));

#ifdef __cplusplus
}
#endif

#endif
