#include "syscall.h"
#include "syscall.hpp"

uint64_t syscall(uint32_t callsys, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    uint64_t ret;
    asm volatile("int $0x80"
        : "=A"(ret)
        : "a"(callsys), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4), "D"(arg5)
        : "memory", "cc");
    return ret;
}

void exit(int status)
{
    syscall(SYSCALL_EXIT, status, 0, 0, 0, 0);
}

int32_t read(int32_t fd, void *buffer, size_t size)
{
    return syscall(SYSCALL_READ, fd, (uintptr_t)buffer, size, 0, 0);
}

int32_t write(int32_t fd, void *buffer, size_t size)
{
    return syscall(SYSCALL_WRITE, fd, (uintptr_t)buffer, size, 0, 0);
}

int32_t close(int32_t fd)
{
    return syscall(SYSCALL_CLOSE, fd, 0, 0, 0, 0);
}

int32_t fork()
{
    return syscall(SYSCALL_FORK, 0, 0, 0, 0, 0);
}

int32_t exec(char *exec, char **args)
{
    return exec_ex(exec, args, 7, 0);
}

int32_t exec_ex(char *exec, char **args, uint32_t fd_keep_low, uint32_t fd_keep_high)
{
    return syscall(SYSCALL_EXEC, (uintptr_t)exec, (uintptr_t)args, fd_keep_low, fd_keep_high, 0);
}

int32_t exec_exv(char *exec, char **args, int fd_keep[])
{
    uint32_t fd_keep_low = 0, fd_keep_high = 0;

    for (int i = 0; fd_keep[i] != -1; i++)
    {
        if (fd_keep[i] < 32)
            fd_keep_low |= 1 << fd_keep[i];
        else
            fd_keep_high |= 1 << (fd_keep[i] - 32);
    }

    return exec_ex(exec, args, fd_keep_low, fd_keep_high);
}

int32_t exec_exl(char *exec, char **args, ... /* -1 */)
{
    return exec_exv(exec, args, (int *)&args + 1);
}

int32_t dup2(uint32_t oldfd, uint32_t newfd)
{
    return syscall(SYSCALL_DUP2, oldfd, newfd, 0, 0, 0);
}

void sched_yield()
{
    syscall(SYSCALL_SCHED_YIELD, 0, 0, 0, 0, 0);
}

int sem_create(int cnt)
{
    return syscall(SYSCALL_SEM_CREATE, cnt, 0, 0, 0, 0);
}
