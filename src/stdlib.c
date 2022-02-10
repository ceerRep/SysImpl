#include <stdlib.h>
#include <heap.h>
#include <panic.h>
#include <string.h>

void *calloc(size_t count, size_t size)
{
    size_t s = count * size;
    void *ret = (char *)malloc(s);
    memset(ret, 0, s);
    return ret;
}

void free(void *ptr)
{
    HeapFree(ptr);
}

void *malloc(size_t size)
{
    return HeapAlloc(0, size);
}

void *realloc(void *ptr, size_t size)
{
    void *ret = malloc(size);

    if (ptr)
    {
        memcpy(ret, ptr, size);
        free(ptr);
    }

    return ret;
}

void abort(void)
{
    panic("abort() called");
}
