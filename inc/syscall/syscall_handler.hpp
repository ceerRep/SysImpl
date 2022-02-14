#ifndef _syscall_handler_hpp

#define _syscall_handler_hpp

#ifdef __cplusplus
extern "C"
#endif
    void
    syscall_handler();

#ifdef __cplusplus

#include <type_traits>
#include <protected_mode.hpp>
#include <objects/Process.hpp>

static auto constexpr syscall_callsys = &tss_entry_struct_t::eax;
static auto constexpr syscall_arg1 = &tss_entry_struct_t::ebx;
static auto constexpr syscall_arg2 = &tss_entry_struct_t::ecx;
static auto constexpr syscall_arg3 = &tss_entry_struct_t::edx;
static auto constexpr syscall_arg4 = &tss_entry_struct_t::esi;
static auto constexpr syscall_arg5 = &tss_entry_struct_t::edi;
static auto constexpr syscall_arg6 = &tss_entry_struct_t::ebp;
static auto constexpr syscall_ret1 = &tss_entry_struct_t::eax;
static auto constexpr syscall_ret2 = &tss_entry_struct_t::edx;

void syscall_set_retval(Process *proc, uint64_t ret);

struct SyscallWrapperBase
{
    uint64_t (*_apply)(SyscallWrapperBase *self);

    SyscallWrapperBase(uint64_t (*_apply)(SyscallWrapperBase *)) : _apply(_apply)
    {
    }

    uint64_t apply()
    {
        return (*_apply)(this);
    }
};

template <typename Func>
struct SyscallWrapper : public SyscallWrapperBase
{
    Func *func;

    static uint64_t apply(SyscallWrapperBase *self)
    {
        SyscallWrapper<Func> *self0 = (SyscallWrapper<Func> *)self;
        auto currentProcess = Process::getProcess(Process::getCurrentProcess());
        auto pstate = currentProcess->getUsermodeState();
        if constexpr (std::is_same_v<Func, uint64_t()>)
        {
            return self0->func();
        }
        else if constexpr (std::is_same_v<Func, uint64_t(uint32_t)>)
        {
            return self0->func(pstate->*syscall_arg1);
        }
        else if constexpr (std::is_same_v<Func, uint64_t(uint32_t, uint32_t)>)
        {
            return self0->func(pstate->*syscall_arg1, pstate->*syscall_arg2);
        }
        else if constexpr (std::is_same_v<Func, uint64_t(uint32_t, uint32_t, uint32_t)>)
        {
            return self0->func(pstate->*syscall_arg1, pstate->*syscall_arg2, pstate->*syscall_arg3);
        }
        else if constexpr (std::is_same_v<Func, uint64_t(uint32_t, uint32_t, uint32_t, uint32_t)>)
        {
            return self0->func(pstate->*syscall_arg1, pstate->*syscall_arg2, pstate->*syscall_arg3, pstate->*syscall_arg4);
        }
        else if constexpr (std::is_same_v<Func, uint64_t(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)>)
        {
            return self0->func(pstate->*syscall_arg1, pstate->*syscall_arg2, pstate->*syscall_arg3, pstate->*syscall_arg4, pstate->*syscall_arg5);
        }
        else if constexpr (std::is_same_v<Func, uint64_t(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)>)
        {
            return self0->func(pstate->*syscall_arg1, pstate->*syscall_arg2, pstate->*syscall_arg3, pstate->*syscall_arg4, pstate->*syscall_arg6);
        }
        else
        {
            static_assert(std::is_same_v<Func, uint64_t(uint32_t)>);
        }
    }

public:
    SyscallWrapper(Func *func) : SyscallWrapperBase(&SyscallWrapper<Func>::apply), func(func) {}
};

void register_syscall(int callsys, SyscallWrapperBase *wrapper);

#endif

#endif
