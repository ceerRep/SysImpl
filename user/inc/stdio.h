#ifndef _stdio_h

#define _stdio_h

int puts(const char *str);
int fputs(const char *str, int fd);
int fprintf(int fd, const char *fmt, ...);

#endif
