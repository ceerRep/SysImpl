#include <assert.h>
#include <cxxabi.h>
#include <heap.h>
#include <ld_syms.h>
#include <multiboot.h>
#include <new>
#include <stdexcept.h>
#include <stdio.h>
#include <stdlib.h>
#include <typeinfo.h>

#include <objects/EarlyStageOutputDevice.hpp>

extern "C"
{
    void _init();
    int main();

    multiboot_info_t mbi_info;
    char kernel_cmdline[512];
    EarlyStageOutputDevice early_output;

    void crt_init(uint32_t magic, uint32_t info_addr)
    {
        KERNEL_BOOT_STACK_PROTECTOR = 0x1919810;

        new (&early_output) EarlyStageOutputDevice;
        setDefaultOutputDevice(&early_output);
        setErrorOutputDevice(&early_output);

        assert(magic == 0x2BADB002);

        mbi_info = *(multiboot_info *)info_addr;
        strlcpy(kernel_cmdline, (char *)mbi_info.cmdline, 512);

        setKernelHeap(HeapInitialize(KERNEL_BOOT_HEAP_END - KERNEL_BOOT_HEAP_START, KERNEL_BOOT_HEAP_START));
        printf("CRT initialized...\n");
    }

    int enter_main()
    {
        int status;
        try
        {
            for (auto p = __init_array_start; p != __init_array_end; p += sizeof(uintptr_t))
                (**(void (**)())p)();

            return main();
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
