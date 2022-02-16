#ifndef _stdio_h

#define _stdio_h

#ifndef NULL
#define NULL 0
#endif

int puts(const char *str);
int fputs(const char *str, int fd);
int fprintf(int fd, const char *fmt, ...);
int printf(const char *fmt, ...);

#endif
