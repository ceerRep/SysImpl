#include <syscall/syscall.hpp>
#include <syscall/syscall_handler.hpp>

#include <objects/Process.hpp>

uint64_t fork_handler()
{
    auto now_proc = Process::getProcess(Process::getCurrentProcess());
    auto child = now_proc->fork();
    return child->getPid();
}

static void __attribute__((constructor, used)) register_fork_handler()
{
    register_syscall(SYSCALL_FORK, new SyscallWrapper(&fork_handler));
}
