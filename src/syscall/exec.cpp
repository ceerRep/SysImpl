#include <syscall/syscall.hpp>
#include <syscall/syscall_handler.hpp>

#include <exec.hpp>

uint64_t exec_handler(uint32_t iexec, uint32_t iargs, uint32_t fd_keep_low, uint32_t fd_keep_high)
{

    char *exec = (char *)iexec;
    char **args = (char **)iargs;

    return execv(Process::getProcess(Process::getCurrentProcess()), exec, args, fd_keep_low, fd_keep_high);
}

static void __attribute__((constructor, used)) register_exec_handler()
{
    register_syscall(SYSCALL_EXEC, new SyscallWrapper(&exec_handler));
}
