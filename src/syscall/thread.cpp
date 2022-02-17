#include <syscall/syscall.hpp>
#include <syscall/syscall_handler.hpp>

#include <objects/Process.hpp>

uint64_t thread_handler(uint32_t func, uint32_t data)
{
    auto now_proc = Process::getProcess(Process::getCurrentProcess());
    auto child = now_proc->clone(Process::PROCESS_CLONE_NEW_THREAD, (void *)func, (void *)data);
    return child->getPid();
}

static void __attribute__((constructor, used)) register_thread_handler()
{
    register_syscall(SYSCALL_THREAD, new SyscallWrapper(&thread_handler));
}
