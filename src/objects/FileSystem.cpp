#include <objects/FileSystem.hpp>

#include <assert.h>
#include <objects/Directory.hpp>
#include <objects/File.hpp>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Fat16FileSystem::Fat16FileSystem(Disk *disk) : disk(disk)
{
    char buffer[512];
    disk->readSector(0, buffer);

    info.pbr = *(PartitionBootRecord *)buffer;
    info.feb = *(Fat16ExtBS *)((PartitionBootRecord *)buffer + 1);

    assert(info.pbr.bytes_per_sector == 512);
    assert(info.feb.boot_signature == 0x28 || info.feb.boot_signature == 0x29);
    assert(info.pbr.sectors_per_fat);

    info.total_sectors = (info.pbr.total_sectors == 0) ? info.pbr.large_total_sectors : info.pbr.total_sectors;
    info.fat_size = info.pbr.sectors_per_fat;
    assert(info.fat_size);
    info.root_dir_sectors = ((info.pbr.root_directory_entry_num * 32) + (info.pbr.bytes_per_sector - 1)) / info.pbr.bytes_per_sector;
    info.first_data_sector = info.pbr.hidden_sectors + (info.pbr.fat_num * info.fat_size) + info.root_dir_sectors;
    info.first_fat_sector = info.pbr.hidden_sectors;
    info.data_sectors = info.total_sectors - (info.pbr.hidden_sectors + (info.pbr.sectors_per_fat * info.fat_size) + info.root_dir_sectors);
    info.total_clusters = info.data_sectors / info.pbr.sectors_per_cluster;

    assert(info.total_clusters >= 4085 && info.total_clusters < 65525);

    fat_file = File::createSequentialFile(this, info.first_fat_sector, info.first_fat_sector + info.fat_size);
}

shared_ptr<Directory> Fat16FileSystem::getRootDirectory()
{
    return make_shared<Directory>(this, info.first_data_sector - info.root_dir_sectors, info.first_data_sector);
}

Fat16FileSystem *Fat16FileSystem::getRootFileSystem()
{
    if (!rootfs)
    {
        rootfs = new Fat16FileSystem(Disk::getBootDisk(nullptr));
    }

    return rootfs;
}
shared_ptr<File> Fat16FileSystem::openFile(shared_ptr<Directory> cwd, const char *path)
{
    shared_ptr<Directory> scwd;
    shared_ptr<File> file;

    if (cwd == nullptr)
        scwd = getRootFileSystem()->getRootDirectory();
    else if (path[0] == '/')
        scwd = getRootFileSystem()->getRootDirectory();
    else
        scwd = shared_ptr<Directory>(cwd, nullptr); // do not free

    file = scwd->asFile();

    const char *cursor = path;
    char filename[MAX_FILENAME_LENGTH + 1];
    filename[0] = 0;

    while (*cursor && *cursor == '/')
        cursor++;

    for (; *cursor;)
    {
        const char *cursor_start = cursor;

        while (*cursor && *cursor != '/')
            cursor++;

        memcpy(filename, cursor_start, cursor - cursor_start);
        filename[cursor - cursor_start] = 0;

        file = scwd->openFile(filename);

        if (!file)
            return shared_ptr<File>();

        if (*cursor == '/') // dir
        {
            while (*cursor && *cursor == '/')
                cursor++;

            scwd = Directory::fromFile(file);

            if (!scwd)
                return shared_ptr<File>();
        }
        else // target file
        {
            break;
        }
    }

    return file;
}
