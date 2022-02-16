#include <syscall/syscall.hpp>
#include <syscall/syscall_handler.hpp>

#include <objects/BlockInputDevice.hpp>
#include <objects/OutputDevice.hpp>

#include <cxxabi.h>
#include <resche.hpp>
#include <stdexcept.h>
#include <stdio.h>
#include <stdlib.h>
#include <typeinfo.h>

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

    int status;

    try
    {
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
    }
    catch (std::exception &e)
    {
        fprintf(stderr, "An exception of type: %s occured in syscall, what(): %s\n",
                abi::__cxa_demangle(typeid(e).name(),
                                    0,
                                    0,
                                    &status),
                e.what());

        abort();
    }
    catch (...)
    {
        fprintf(stderr, "An [object %s] occured in syscall\n",
                abi::__cxa_demangle(
                    abi::__cxa_current_exception_type()->name(),
                    0,
                    0,
                    &status));

        abort();
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
