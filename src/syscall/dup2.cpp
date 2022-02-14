#include <objects/Process.hpp>
#include <syscall/syscall.hpp>
#include <syscall/syscall_handler.hpp>

uint64_t dup2_handler(uint32_t oldfd, uint32_t newfd)
{
    if (oldfd == newfd)
        return 0;

    auto now_proc = Process::getProcess(Process::getCurrentProcess());
    auto sobj = now_proc->getObject(oldfd);

    if (!sobj)
        return -1;

    now_proc->setObject(newfd, sobj);

    return 0;
}

static void __attribute__((constructor, used)) register_dup2_handler()
{
    register_syscall(SYSCALL_DUP2, new SyscallWrapper(&dup2_handler));
}
