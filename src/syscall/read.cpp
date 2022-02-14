#include <objects/InputDevice.hpp>
#include <objects/Process.hpp>
#include <syscall/syscall.hpp>
#include <syscall/syscall_handler.hpp>

uint64_t read_handler(uint32_t device, uint32_t buffer0, uint32_t size)
{
    auto now_proc = Process::getProcess(Process::getCurrentProcess());
    auto sobj = now_proc->getObject(device);
    auto buffer = (char *)buffer0;
    auto count = size;

    if (!sobj)
        return -1;

    if (auto pread = dynamic_cast<InputDevice *>(sobj.get()))
    {
        return pread->read(buffer, count);
    }
    else
    {
        return -1;
    }
}

static void __attribute__((constructor, used)) register_read_handler()
{
    register_syscall(SYSCALL_READ, new SyscallWrapper(&read_handler));
}
