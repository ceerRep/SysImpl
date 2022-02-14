#include <heap.h>
#include <ld_syms.h>
#include <stdio.h>
#include <stdlib.h>
#include <cxxabi.h>
#include <stdexcept.h>
#include <typeinfo.h>

#include <objects/EarlyStageOutputDevice.hpp>

extern "C"
{
    void _init();
    int main(uint32_t magic, uint32_t info_addr);

    void crt_init()
    {
        KERNEL_BOOT_STACK_PROTECTOR = 0x1919810;
        setKernelHeap(HeapInitialize(KERNEL_BOOT_HEAP_END - KERNEL_BOOT_HEAP_START, KERNEL_BOOT_HEAP_START));

        auto *device = new EarlyStageOutputDevice;
        setDefaultOutputDevice(device);
        setErrorOutputDevice(device);
        printf("CRT initialized...\n");
    }

    int enter_main(uint32_t magic, uint32_t info_addr)
    {
        int status;
        try
        {
            for (auto p = __init_array_start; p != __init_array_end; p+=sizeof(uintptr_t))
                (**(void (**)())p)();

            return main(magic, info_addr);
        }
        catch (std::exception &e)
        {
            printf("Kernel exited with an exception of type: %s, what(): %s\n",
                   abi::__cxa_demangle(typeid(e).name(),
                                       0,
                                       0,
                                       &status),
                   e.what());
        }
        catch (...)
        {
            printf("Kernel exited with an [object %s]\n",
                   abi::__cxa_demangle(
                       abi::__cxa_current_exception_type()->name(),
                       0,
                       0,
                       &status));
        }

        abort();
    }
}
