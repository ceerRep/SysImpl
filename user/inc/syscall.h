#ifndef _syscall_h

#define _syscall_h

#include <stddef.h>
#include <stdint.h>
#include <common_def.h>

#ifdef __cplusplus
extern "C"
{
#endif

    uint64_t syscall(uint32_t callsys, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5);

    void exit(int status);

    int32_t read(int32_t fd, void *buffer, size_t size);
    int32_t write(int32_t fd, void *buffer, size_t size);
    int32_t open(const char* filename);
    int32_t close(int32_t fd);

    int32_t fork();

    int32_t exec(char *exec, char **args);
    int32_t exec_ex(char *exec, char **args, uint32_t fd_keep_low, uint32_t fd_keep_high);

    int32_t exec_exv(char *exec, char **args, int *fd_keep); // -1 to terminate
    int32_t exec_exl(char *exec, char **args, ... /* -1 */);

    int32_t dup2(uint32_t oldfd, uint32_t newfd);
    int32_t readdir(uint32_t fd, file_info_t *info, int *n);

    void sched_yield();

    int sem_create(int cnt);

#ifdef __cplusplus
}
#endif

#endif
