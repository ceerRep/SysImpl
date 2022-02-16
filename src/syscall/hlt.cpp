#include <objects/Process.hpp>
#include <syscall/syscall.hpp>
#include <syscall/syscall_handler.hpp>

uint64_t hlt_handler()
{
    auto now_proc = Process::getProcess(Process::getCurrentProcess());

    if (now_proc->getPid() == 0)
        asm volatile("hlt" ::
                         : "memory", "cc");

    return 0;
}

static void __attribute__((constructor, used)) register_hlt_handler()
{
    register_syscall(SYSCALL_HIDDEN_HLT, new SyscallWrapper(&hlt_handler));
}
