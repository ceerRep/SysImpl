#ifndef _PANIC_H

#define _PANIC_H

#ifdef __cplusplus
extern "C"
{
#endif

void panic(const char *) __attribute__((noreturn));

#ifdef __cplusplus
}
#endif

#endif
