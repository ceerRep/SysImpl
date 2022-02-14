#include <resche.hpp>
#include <objects/Object.hpp>
#include <syscall/syscall.hpp>
#include <syscall/syscall_handler.hpp>

uint64_t exit_handler(uint32_t code)
{
    Process::dropProcess(Process::getCurrentProcess());
    return 0;
}

static void __attribute__((constructor, used)) register_exit_handler()
{
    register_syscall(SYSCALL_EXIT, new SyscallWrapper(&exit_handler));
}
