#include <common_def.h>
#include <stdio.h>
#include <string.h>
#include <syscall.h>

volatile int running;
volatile int counter = 0;

typedef struct
{
    int sem1;
    int sem2;
    int id;
} info_t;

void A(info_t *info)
{
    for (; running;)
    {
        read(info->sem1, NULL, 1);
        counter++;

        if (counter >= 100)
            running = 0;

        printf("%d: %d\n", info->id, counter);
        write(info->sem2, NULL, 1);
    }

    exit(0);
}

int main(int argc, char **argv)
{
    int sem1 = sem_create(1);
    int sem2 = sem_create(0);
    int fd_wait = open(":WAITPID");

    info_t info1, info2;

    info1.sem1 = sem1;
    info1.sem2 = sem2;
    info1.id = 1;

    info2.sem1 = sem2;
    info2.sem2 = sem1;
    info2.id = 2;

    running = 1;

    int pid1 = thread(A, &info1);
    int pid2 = thread(A, &info2);
    int result;

    write(fd_wait, &pid1, 4);
    read(fd_wait, &result, 4);
    write(fd_wait, &pid2, 4);
    read(fd_wait, &result, 4);

    return 0;
}
