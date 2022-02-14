#include <objects/FileSystem.hpp>

#include <assert.h>
#include <objects/Directory.hpp>
#include <objects/File.hpp>
#include <stdint.h>
#include <stdio.h>

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

    fat_file = create_shared(File::createSequentialFile(disk, info.first_fat_sector, info.first_fat_sector + info.fat_size));
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
