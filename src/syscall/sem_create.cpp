#include <objects/Semaphore.hpp>
#include <syscall/syscall.hpp>
#include <syscall/syscall_handler.hpp>

#include <errno.h>

uint64_t sem_create_handler(uint32_t cnt)
{
    auto sem = Semaphore::createSemaphore((int)cnt);

    try
    {
        return Process::getProcess(Process::getCurrentProcess())->addObject(sem);
    }
    catch (BlockInputDeviceLimitExceededException)
    {
        return -ENOENT;
    }
}

static void __attribute__((constructor, used)) register_sem_create_handler()
{
    register_syscall(SYSCALL_SEM_CREATE, new SyscallWrapper(&sem_create_handler));
}
