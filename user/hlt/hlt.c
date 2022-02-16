#include <stdio.h>
#include <syscall.h>
#include <syscall.hpp>

int main(int argc, char **argv)
{
    if (fork())
    {
        for (;;)
            syscall(SYSCALL_HIDDEN_HLT, 0, 0, 0, 0, 0);
    }
    else
    {
        exec(argv[0], argv);
        exit(0);
    }

    return 0;
}
