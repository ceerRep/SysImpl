#include <stdio.h>
#include <string.h>
#include <syscall.h>

char str[] = "Parent process\n";

int main(int argc, char **argv)
{
    int sem_sync_parent = sem_create(1);
    int sem_sync_child = sem_create(0);

    fprintf(1, "SemSyncParent fd: %d\n", sem_sync_parent);
    fprintf(1, "SemSyncChild fd: %d\n", sem_sync_child);

    dup2(sem_sync_parent, 14);
    dup2(sem_sync_child, 15);

    close(sem_sync_parent);
    close(sem_sync_child);

    puts("args:");
    for (int i = 0; i < argc; i++)
    {
        fprintf(1, "%s\n", argv[i]);
    }

    if (fork())
    {
        for (int i = 1; /*i <= 5*/; i++)
        {
            // read(15, NULL, 1);
            fprintf(1, "parent %d\n", i);
            // write(14, NULL, 1);
        }
    }
    else
    {
        char *target_file = argv[1];
        char *args[] = {target_file, NULL};

        exec_exl(target_file, args, 0, 1, 2, 15, 14, -1);
        exit(1);
    }
    return 0;
}
