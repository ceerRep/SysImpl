#ifndef _STDLIB_H

#define _STDLIB_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>

    void *memset(void *b, int c, uintptr_t len);
    int memcmp(const void *s1, const void *s2, size_t n);
    void *memcpy(void *dst, const void *src, size_t n);
    size_t strlen(const char *s);
    int strcmp(const char *s1, const char *s2);
    int strncmp(const char *s1, const char *s2, size_t n);
    char *strdup(const char *s1);
    char *strstr(const char *s, const char *find);
    long strtol(const char *nptr, char **endptr, int base);

#ifdef __cplusplus
}
#endif

#endif
