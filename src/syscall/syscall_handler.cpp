#include <syscall/syscall.hpp>
#include <syscall/syscall_handler.hpp>

#include <objects/BlockInputDevice.hpp>
#include <objects/OutputDevice.hpp>

#include <resche.hpp>
#include <stdio.h>

SyscallWrapperBase *syscall_wrappers[SYSCALL_END] = {0};

void syscall_set_retval(Process *proc, uint64_t ret)
{
    auto pstate = proc->getUsermodeState();
    pstate->eax = ret & 0xFFFFFFFF;
    pstate->edx = ret >> 32;
}

// TODO: add global hook manager

extern "C" void real_syscall_handler()
{
    // update tss
    Process::enterKernelMode();

    auto pid = Process::getCurrentProcess();
    auto now_proc = Process::getProcess(pid);
    auto pstate = now_proc->getUsermodeState();

    if (syscall_wrappers[pstate->*syscall_callsys])
    {
        auto ret = syscall_wrappers[pstate->*syscall_callsys]->apply();

        BlockInputDeviceWrapper::checkAll();

        if (Process::getProcess(pid)) // check whether it exits
        {
            syscall_set_retval(now_proc, ret);

            // sleeped
            if (now_proc->getProcessState() != Process::PROCESS_STATE_RUNNABLE)
                resche();
        }
        else
            resche();
    }

    resche_leave_kernel_mode_hook();
    Process::leaveKernelMode();
}

asm(".globl syscall_handler\n\t"
    "syscall_handler:\n\t"
    "call real_syscall_handler\n\t"
    "iret\n\t"
    "jmp syscall_handler\n\t");

void register_syscall(int callsys, SyscallWrapperBase *wrapper)
{
    syscall_wrappers[callsys] = wrapper;
}
