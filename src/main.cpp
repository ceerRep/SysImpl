#include <io_port.h>
#include <multiboot.h>
#include <protected_mode.hpp>
#include <heap.h>
#include <8086.h>
#include <stddef.h>
#include <stdio.h>
#include <kernel_defines.h>

#include <objects/SerialIODevice.hpp>
#include <objects/VGATextOutputDevice.hpp>

extern char __EH_FRAME_BEGIN__[0];

extern "C" void __register_frame_info(__attribute__((unused)) const void *p,
                                      __attribute__((unused)) struct object *o);

struct object
{
    void *pc_begin;
    void *tbase;
    void *dbase;
    union
    {
        const struct dwarf_fde *single;
        struct dwarf_fde **array;
        struct fde_vector *sort;
    } u;

    union
    {
        struct
        {
            unsigned long sorted : 1;
            unsigned long from_array : 1;
            unsigned long mixed_encoding : 1;
            unsigned long encoding : 8;
            /* ??? Wish there was an easy way to detect a 64-bit host here;
           we've got 32 bits left to play with...  */
            unsigned long count : 21;
        } b;
        size_t i;
    } s;

    char *fde_end;

    struct object *next;
} obj;

struct A
{
    virtual ~A()
    {
        printf("~A\n");
    }
};

struct B : public A
{
    virtual ~B()
    {
        printf("~B\n");
    }
};
struct C : public A
{
    virtual ~C()
    {
        printf("~C\n");
    }
};

struct D
{
    D()
    {
        *(char *)(0xB8000) = 'A';
    }
} d;

extern "C"
{
    void _init();
    extern char KERNEL_BOOT_HEAP_START[0], KERNEL_BOOT_HEAP_END[0];

    int main(uint32_t magic, uint32_t info_addr)
    {
        initialize_gdt();
        initialize_intr();

        printf("Enter main...\n");

        {
            uint32_t eax, ebx, ecx, edx;
            asm("cpuid"
                : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                : "a"(1));
            printf("CPUID: %08x %08x %08x %08x\n", eax, ebx, ecx, edx);
        }

        // Initialize serial
        try
        {
            SerialIODevice *serial = new SerialIODevice(SERIAL_PORT);
            setDefaultOutputDevice(serial);
            printf("Serial IO initialized...\n");
        }
        catch (std::exception e)
        {
            printf("Error initializing serial io: %s\n", e.what());
        }
        printf("%08x %08x\n", magic, info_addr);

        v8086_init();

        setDefaultOutputDevice(new VGABaseOutputDevice);

        // bochs magic breakpoint
        asm volatile("xchg %bx, %bx");
        printf("rtti test:\n");
        A *a0 = new B;
        A *a1 = new C;
        printf("a0: ");
        if (dynamic_cast<B *>(a0))
        {
            printf("B\n");
        }
        if (dynamic_cast<C *>(a0))
        {
            printf("C\n");
        }
        printf("a1: ");
        if (dynamic_cast<B *>(a1))
        {
            printf("B\n");
        }
        if (dynamic_cast<C *>(a1))
        {
            printf("C\n");
        }

        delete a0;
        delete a1;

#define CHECK_FLAG(flags, bit) ((flags) & (1 << (bit)))

        multiboot_info *mbi = (multiboot_info *)info_addr;
        printf("%08x\n", mbi->flags);

        if (CHECK_FLAG(mbi->flags, 0))
            printf("mem_lower = %uKB, mem_upper = %uKB\n",
                   (unsigned)mbi->mem_lower, (unsigned)mbi->mem_upper);

        /* Is boot_device valid? */
        if (CHECK_FLAG(mbi->flags, 1))
            printf("boot_device = 0x%08x\n", (unsigned)mbi->boot_device);

        /* Is the command line passed? */
        if (CHECK_FLAG(mbi->flags, 2))
            printf("cmdline = %s\n", (char *)mbi->cmdline);

        /* Are mods_* valid? */
        if (CHECK_FLAG(mbi->flags, 3))
        {
            multiboot_module_t *mod;
            int i;

            printf("mods_count = %d, mods_addr = 0x%08x\n",
                   (int)mbi->mods_count, (int)mbi->mods_addr);
            for (i = 0, mod = (multiboot_module_t *)mbi->mods_addr;
                 i < mbi->mods_count;
                 i++, mod++)
                printf(" mod_start = 0x%08x, mod_end = 0x%08x, cmdline = %s\n",
                       (unsigned)mod->mod_start,
                       (unsigned)mod->mod_end,
                       (char *)mod->cmdline);
        }

        /* Bits 4 and 5 are mutually exclusive! */
        if (CHECK_FLAG(mbi->flags, 4) && CHECK_FLAG(mbi->flags, 5))
        {
            printf("Both bits 4 and 5 are set.\n");
            return 1;
        }

        /* Is the symbol table of a.out valid? */
        if (CHECK_FLAG(mbi->flags, 4))
        {
            multiboot_aout_symbol_table_t *multiboot_aout_sym = &(mbi->u.aout_sym);

            printf("multiboot_aout_symbol_table: tabsize = 0x%08x, "
                   "strsize = 0x%08x, addr = 0x%08x\n",
                   (unsigned)multiboot_aout_sym->tabsize,
                   (unsigned)multiboot_aout_sym->strsize,
                   (unsigned)multiboot_aout_sym->addr);
        }

        /* Is the section header table of ELF valid? */
        if (CHECK_FLAG(mbi->flags, 5))
        {
            multiboot_elf_section_header_table_t *multiboot_elf_sec = &(mbi->u.elf_sec);

            printf("multiboot_elf_sec: num = %u, size = 0x%08x,"
                   " addr = 0x%08x, shndx = 0x%08x\n",
                   (unsigned)multiboot_elf_sec->num, (unsigned)multiboot_elf_sec->size,
                   (unsigned)multiboot_elf_sec->addr, (unsigned)multiboot_elf_sec->shndx);
        }

        /* Are mmap_* valid? */
        if (CHECK_FLAG(mbi->flags, 6))
        {
            multiboot_memory_map_t *mmap;

            printf("mmap_addr = 0x%08x, mmap_length = 0x%08x\n",
                   (unsigned)mbi->mmap_addr, (unsigned)mbi->mmap_length);
            for (mmap = (multiboot_memory_map_t *)mbi->mmap_addr;
                 (unsigned long)mmap < mbi->mmap_addr + mbi->mmap_length;
                 mmap = (multiboot_memory_map_t *)((unsigned long)mmap + mmap->size + sizeof(mmap->size)))
                printf(" size = 0x%08x, base_addr = 0x%08x%08x,"
                       " length = 0x%08x%08x, type = 0x%08x\n",
                       (unsigned)mmap->size,
                       (unsigned)(mmap->addr >> 32),
                       (unsigned)(mmap->addr & 0xffffffff),
                       (unsigned)(mmap->len >> 32),
                       (unsigned)(mmap->len & 0xffffffff),
                       (unsigned)mmap->type);
        }

        return 0;
    }
}
