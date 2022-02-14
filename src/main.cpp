#include <8086.h>
#include <8259.hpp>
#include <align_util.h>
#include <assert.h>
#include <ctype.h>
#include <exec.hpp>
#include <heap.h>
#include <kernel_defines.h>
#include <ld_syms.h>
#include <multiboot.h>
#include <pit.hpp>
#include <protected_mode.hpp>
#include <stddef.h>
#include <stdio.h>

#include <common_def.h>

#include <elf/elf-loader.hpp>

#include <objects/Directory.hpp>
#include <objects/Disk.hpp>
#include <objects/FileSystem.hpp>
#include <objects/Process.hpp>
#include <objects/SerialIODevice.hpp>
#include <objects/VGATextOutputDevice.hpp>

extern "C" void check_disable_apic();

void enter_init(char *init_path, char **args)
{

    auto init_process = new Process(nullptr);

    if (getDefaultInputDevice())
        init_process->setObject(0, shared_ptr<Object>(getDefaultInputDevice(), nullptr));
    if (getDefaultOutputDevice())
        init_process->setObject(1, shared_ptr<Object>(getDefaultOutputDevice(), nullptr));
    if (getErrorOutputDevice())
        init_process->setObject(2, shared_ptr<Object>(getErrorOutputDevice(), nullptr));

    assert("Init process start failed" && !execv(init_process, init_path, args));

    init_process->resume();

    struct __attribute__((packed))
    {
        uint32_t addr;
        uint32_t seg;
    } ljmp_target;

    ljmp_target.seg = SEGMENT_SELECTOR(SEG_USER_TSS, 0, 3);
    ljmp_target.addr = 0; // UNUSED

    asm volatile("ljmp *%0"
                 :
                 : "m"(ljmp_target)
                 : "memory", "cc");
}

extern "C"
{
    void _init();
    extern char KERNEL_BOOT_HEAP_START[0], KERNEL_BOOT_HEAP_END[0];

    int main(uint32_t magic, uint32_t info_addr)
    {
        char cmdline[512];

        {
            multiboot_info *mbi = (multiboot_info *)info_addr;
            strlcpy(cmdline, (char *)mbi->cmdline, 512);
        }

        initialize_gdt();
        initialize_intr();
        PIC_remap(IRQ_BASE0, IRQ_BASE1);
        check_disable_apic();
        pit_init();

        printf("Enter main...\n");

        {
            uint32_t eax, ebx, ecx, edx;
            asm volatile("cpuid"
                         : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                         : "a"(1)
                         : "memory", "cc");
            printf("CPUID: %08x %08x %08x %08x\n", eax, ebx, ecx, edx);
        }

        // Initialize serial
        // try
        // {
        //     SerialIODevice *serial = new SerialIODevice(SERIAL_PORT);
        //     setDefaultInputDevice(serial);
        //     setDefaultOutputDevice(serial);
        //     setErrorOutputDevice(serial);
        //     printf("Serial IO initialized...\n");
        // }
        // catch (std::exception e)
        // {
        //     printf("Error initializing serial io: %s\n", e.what());
        // }
        printf("%08x %08x\n", magic, info_addr);

        v8086_init();
        setDefaultOutputDevice(new VGABaseOutputDevice);
        setErrorOutputDevice(new VGABaseOutputDevice);

        char *init_path = "/init.elf";
        shared_ptr<char *> args = create_shared(new char *[PROCESS_MAX_ARGUMENTS + 1]);
        for (int i = 0; i <= PROCESS_MAX_ARGUMENTS; i++)
            args[i] = nullptr;

        char *mem_upper;
        // drive num -> part id1 -> part id2 -> part id3, ended with 0xff
        uint8_t boot_device[4];

#define CHECK_FLAG(flags, bit) ((flags) & (1 << (bit)))
        {
            multiboot_info *mbi = (multiboot_info *)info_addr;

            assert("Multiboot info doesn't have memory info" && CHECK_FLAG(mbi->flags, 0));
            mem_upper = (char *)ALIGN_FLOOR(mbi->mem_upper * 1024, 8192);

            assert("Multiboot info doesn't have boot disk info" && CHECK_FLAG(mbi->flags, 1));
            boot_device[0] = mbi->boot_device >> 24;
            boot_device[1] = mbi->boot_device >> 16;
            boot_device[2] = mbi->boot_device >> 8;
            boot_device[3] = mbi->boot_device;

            printf("Booted from drive 0x%02x", boot_device[0]);

            for (int i = 1; i < 4; i++)
            {
                if (boot_device[i] != 0xff)
                    printf(", sub part 0x%02x", boot_device[i]);
                else
                    break;
            }
            puts("");

            char *now_start = cmdline;

            printf("cmdline: %s\n", now_start);

            bool begin_args = 0;
            int now_arg_ind = 0;
            char *now_end;

            // find the last param

            while (*now_start && *now_start == ' ')
                *(now_start++) = 0;

            for (;;)
            {
                now_end = now_start;
                while (*now_end && *now_end != ' ')
                    now_end++;

                if (*now_end == '\0')
                {
                    if (!begin_args && !strcmp(now_start, "--"))
                    {
                        begin_args = 1;
                        args[now_arg_ind++] = init_path;
                    }
                    else
                    {
                        if (begin_args)
                        {
                            if (now_arg_ind < PROCESS_MAX_ARGUMENTS)
                                args[now_arg_ind++] = now_start;
                        }
                        else
                        {
                            init_path = now_start;
                        }
                    }

                    break;
                }

                char *next = now_end;

                while (*next && *next == ' ')
                    *(next++) = 0;

                if (!begin_args && !strcmp(now_start, "--"))
                {
                    begin_args = 1;
                    args[now_arg_ind++] = init_path;
                }
                else
                {
                    if (begin_args)
                    {
                        if (now_arg_ind < PROCESS_MAX_ARGUMENTS)
                            args[now_arg_ind++] = now_start;
                    }
                    else
                    {
                        init_path = now_start;
                    }
                }

                if (*next)
                    now_start = next;
                else
                    break;
            }

            if (!begin_args)
            {
                args[now_arg_ind++] = init_path;
            }
        }
#undef CHECK_FLAG

        setKernelHeap(HeapInitialize(USER_SPACE_START - _end, _end));
        // Align to 4096
        void *kernelModeStack = HeapAlloc(nullptr, Process::MAX_PROCESS_NUM * Process::PROCESS_STACK_SIZE + Process::PROCESS_STACK_ALIGNMENT);
        kernelModeStack = (void *)ALIGN_CEIL((uintptr_t)kernelModeStack, Process::PROCESS_STACK_ALIGNMENT);
        Process::setKernelStack(kernelModeStack);
        Process::setUserStack(mem_upper - Process::MAX_PROCESS_NUM * Process::PROCESS_STACK_SIZE);

        Disk *disk = Disk::getBootDisk(boot_device);
        Fat16FileSystem *fs = Fat16FileSystem::getRootFileSystem();

        enter_init(init_path, args);

        return 0;
    }
}
