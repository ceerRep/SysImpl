#include <stdint.h>

#include <common_def.h>
#include <stdio.h>
#include <syscall.h>

int main(int argc, char **argv);

void start(process_runtime_info_t *runtime_info)
{
    int argc;
    char *argv[PROCESS_MAX_ARGUMENTS + 1];

    for (argc = 0; argc < PROCESS_MAX_ARGUMENTS; argc++)
    {
        if (runtime_info->args[argc] != 0xFFFF)
        {
            argv[argc] = runtime_info->buffer + runtime_info->args[argc];
        }
        else
            break;
    }

    argv[argc] = NULL;

    exit(main(argc, (char **)argv));
}
