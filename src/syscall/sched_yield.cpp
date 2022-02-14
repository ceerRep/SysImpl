#include <resche.hpp>
#include <syscall/syscall.hpp>
#include <syscall/syscall_handler.hpp>

uint64_t sched_yield_handler()
{
    resche();
    return 0;
}

static void __attribute__((constructor, used)) register_sched_yield_handler()
{
    register_syscall(SYSCALL_SCHED_YIELD, new SyscallWrapper(&sched_yield_handler));
}
