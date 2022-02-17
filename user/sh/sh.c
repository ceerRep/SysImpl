#include <common_def.h>
#include <stdio.h>
#include <string.h>
#include <syscall.h>

int main(int argc, char **argv)
{
    int waitfd = open(":WAITPID");

    char buffer[256];
    char prompt[] = "> ";

    int siz;
    for (; fputs(prompt, 1), siz = read(0, buffer, 255);) // until EOF
    {
        buffer[siz] = 0;

        char *cmd = strtok(buffer, " \n");
        if (!cmd)
            continue;

        if (strcmp(cmd, "dir") == 0)
        {
            char *path = strtok(NULL, " \n");

            if (!path)
            {
                fprintf(2, "Syntax error: dir <path>\n");
                continue;
            }

            int fd = open(path), n = 0;

            if (fd < 0)
            {
                fprintf(2, "dir %s not found\n", path);
                continue;
            }

            int ret = readdir(fd, NULL, &n);

            if (ret < 0)
            {
                fprintf(2, "%s is not a directory\n", path);
                continue;
            }

            file_info_t infos[n];
            readdir(fd, infos, &n);

            fprintf(1, "* %15s %10s\n", "name", "size");
            for (int i = 0; i < n; i++)
            {
                fprintf(1, "%c %15s %10d\n", infos[i].directory ? 'd' : '-', infos[i].filename, infos[i].size);
            }
        }
        else // exec
        {
            int wait = 1;
            char *args[PROCESS_MAX_ARGUMENTS + 1];
            char *stdin_file = NULL, *stdout_file = NULL;
            args[0] = cmd;
            int i = 1;

            for (;;)
            {
                char *now = strtok(NULL, " \n");

                if (!now)
                    break;

                if (now[0] == '>')
                    stdout_file = now + 1;
                else if (now[0] == '<')
                    stdin_file = now + 1;
                else if (i < PROCESS_MAX_ARGUMENTS)
                    args[i++] = now;
            }

            if (strcmp(args[i - 1], "&") == 0)
            {
                i--;
                wait = 0;
            }

            args[i] = NULL;

            int pid = fork();
            int ret = 1919810;

            if (!pid)
            {
                if (stdin_file)
                {
                    int stdin_fd = open(stdin_file);
                    dup2(stdin_fd, 0);
                    close(stdin_fd);
                }

                if (stdout_file)
                {
                    int stdout_fd = open(stdout_file);
                    dup2(stdout_fd, 1);
                    dup2(stdout_fd, 2);
                    close(stdout_fd);
                }

                exec(cmd, args);
                fprintf(2, "exec failed\n");
                exit(0);
            }
            else if (wait)
            {
                write(waitfd, &pid, 4);
                read(waitfd, &ret, 4);
                fprintf(1, "process exited with code: %d\n", ret);
            }
        }
    }

    return 0;
}
