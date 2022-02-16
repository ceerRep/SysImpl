#include <stdio.h>
#include <syscall.h>
#include <syscall.hpp>

int main(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
        printf("%s%s", argv[i], i + 1 == argc ? "" : " ");
    
    puts("");

    return 0;
}
