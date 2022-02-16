#include <exec.hpp>

#include <objects/Directory.hpp>
#include <objects/File.hpp>
#include <objects/FileSystem.hpp>
#include <objects/Process.hpp>

#include <elf/elf-loader.hpp>

#include <errno.h>
#include <string.h>

int execv(Process *proc, char *exec, char **args, uint32_t object_keep_low, uint32_t object_keep_high)
{
    shared_ptr<File> pfile = Fat16FileSystem::openFile(nullptr, exec);

    if (!pfile)
        return -ENOENT;

    pfile->seek(0, File::SEEK_END);
    int size = pfile->tell();
    pfile->seek(0, File::SEEK_SET);

    auto buffer = create_shared<uint8_t>(new uint8_t[size]);
    pfile->read(buffer.get(), size);

    int section_num;

    try
    {
        section_num = parseElfSections(buffer.get(), nullptr, nullptr, 0);
    }
    catch (InvalidElfException)
    {
        return -ENOEXEC;
    }

    uint32_t entry;
    auto sections = create_shared<Section>(new Section[section_num]);
    parseElfSections(buffer.get(), sections, &entry, section_num);

    try
    {
        proc->exec(args, sections, section_num, (void *)entry, object_keep_low, object_keep_high);
    }
    catch (InvalidSectionException)
    {
        return -ENOEXEC;
    }

    return 0;
}
