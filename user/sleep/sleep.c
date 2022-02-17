#include <common_def.h>
#include <stdio.h>
#include <string.h>
#include <syscall.h>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(2, "Usage: %s <seconds>\n", argv[0]);
        exit(1);
    }

    int waitfd = open(":SLEEP");

    int sec = strtol(argv[1], NULL, 10) * 1000;
    write(waitfd, &sec, 4);
    read(waitfd, NULL, 0);

    printf("awaked\n");

    return 0;
}
