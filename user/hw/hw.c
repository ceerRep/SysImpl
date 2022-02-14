#include <stdio.h>
#include <syscall.h>

char str[] = "Hello world\n";

int main(int argc, char **argv)
{
    sched_yield();

    for (int i = 1; /*i <= 5*/; i++)
    {
        // read(14, NULL, 1);
        fprintf(1, "child %d\n", i);
        // write(15, NULL, 1);
    }

    return 0;
}
