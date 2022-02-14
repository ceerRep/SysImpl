#include <elf/elf-loader.hpp>

#include <elf/elf.h>
#include <objects/Process.hpp>
#include <shared_ptr.hpp>

int parseElfSections(const uint8_t *buffer, Section *sections, uint32_t *entry, int n)
{
    int num = 0;

    const elf32_hdr *hdr = (const elf32_hdr *)buffer;

    if (memcmp(hdr->e_ident, "\x7f" "ELF", 4) != 0)
        throw InvalidElfException("Unexpected magic");

    if (hdr->e_ident[EI_CLASS] != 1)
        throw InvalidElfException("Unexpected arch");

    if (hdr->e_ident[EI_DATA] != 1)
        throw InvalidElfException("Unexpected endian");

    if (entry)
        *entry = hdr->e_entry;

    const elf32_phdr *phdr = (const elf32_phdr *)(buffer + hdr->e_phoff);

    for (int i = 0; i < hdr->e_phoff; i++)
    {
        if (phdr[i].p_type == PT_LOAD)
        {
            if (num < n)
            {
                sections[i].base = phdr[i].p_paddr;
                sections[i].limit = phdr[i].p_memsz;
                sections[i].buffer = (buffer + phdr[i].p_offset);
            }
            num++;
        }
    }

    return num;
}
