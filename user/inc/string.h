#ifndef _STDLIB_H

#define _STDLIB_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>
#include <stdint.h>

    void *memset(void *b, int c, uintptr_t len);
    int memcmp(const void *s1, const void *s2, size_t n);
    void *memcpy(void *dst, const void *src, size_t n);
    size_t strlen(const char *s);
    int strcmp(const char *s1, const char *s2);
    int strncmp(const char *s1, const char *s2, size_t n);
    int strcasecmp(const char *s1, const char *s2);
    int strncasecmp(const char *s1, const char *s2, size_t n);
    char *strcpy(char *to, const char *from);
    size_t strlcpy(char *dst, const char *src, size_t dsize);
    char *strcat(char *s, const char *append);
    char *strdup(const char *s1);
    char *strstr(const char *s, const char *find);
    long strtol(const char *nptr, char **endptr, int base);

#ifdef __cplusplus
}
#endif

#endif
