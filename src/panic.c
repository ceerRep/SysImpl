#include <panic.h>

#include <stdio.h>

void panic(const char *msg)
{
    fprintf(stderr, "Kernel panic: %s\n\nSystem halted\n", msg);

    while (1)
        asm("hlt");
}
