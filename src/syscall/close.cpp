#include <objects/Process.hpp>
#include <syscall/syscall.hpp>
#include <syscall/syscall_handler.hpp>

uint64_t close_handler(uint32_t fd)
{
    auto now_proc = Process::getProcess(Process::getCurrentProcess());
    now_proc->removeObject(fd);

    return 0;
}

static void __attribute__((constructor, used)) register_close_handler()
{
    register_syscall(SYSCALL_CLOSE, new SyscallWrapper(&close_handler));
}
