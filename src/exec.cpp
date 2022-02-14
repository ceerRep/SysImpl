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
    char *cursor = exec;
    char filename[Fat16FileSystem::MAX_FILENAME_LENGTH + 1];
    filename[0] = 0;

    auto nowdir = Fat16FileSystem::getRootFileSystem()->getRootDirectory();
    shared_ptr<File> pfile;

    while (*cursor && *cursor == '/')
        cursor++;

    for (;;)
    {
        char *cursor_start = cursor;

        while (*cursor && *cursor != '/')
            cursor++;

        memcpy(filename, cursor_start, cursor - cursor_start);
        filename[cursor - cursor_start] = 0;

        if (*cursor == '/') // dir
        {
            while (*cursor && *cursor == '/')
                cursor++;

            auto nextdir = nowdir->openDirectory(filename);

            if (!nextdir)
                return -ENOENT;

            nowdir = create_shared(nextdir);
        }
        else // target file
        {
            pfile = create_shared(nowdir->openFile(filename));

            if (!pfile)
                return -ENOENT;

            break;
        }
    }

    pfile->seek(0, File::SEEK_END);
    int size = pfile->tell();
    pfile->seek(0, File::SEEK_SET);

    auto buffer = create_shared<uint8_t>(new uint8_t[size]);
    pfile->read(buffer.get(), size);

    auto section_num = parseElfSections(buffer.get(), nullptr, nullptr, 0);

    uint32_t entry;
    auto sections = create_shared<Section>(new Section[section_num]);
    parseElfSections(buffer.get(), sections, &entry, section_num);

    try
    {
        proc->exec(args, sections, section_num, (void *)entry, object_keep_low, object_keep_high);
    }
    catch (InvalidSectionException)
    {
        return -ENOENT;
    }

    return 0;
}
