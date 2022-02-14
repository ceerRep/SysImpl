#ifndef _filesystem_hpp

#define _filesystem_hpp

#include "Disk.hpp"
#include "Object.hpp"

#include <shared_ptr.hpp>

class Directory;
class File;

struct __attribute__((packed)) PartitionBootRecord
{
    uint8_t jmp_short[3];
    uint8_t oem[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_num;
    uint16_t root_directory_entry_num;
    uint16_t total_sectors;
    uint8_t media_descriptor_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t heads;
    uint32_t hidden_sectors;
    uint32_t large_total_sectors;
};

struct __attribute__((packed)) Fat16ExtBS
{
    // extended fat12 and fat16 stuff
    unsigned char bios_drive_num;
    unsigned char reserved1;
    unsigned char boot_signature;
    unsigned int volume_id;
    unsigned char volume_label[11];
    unsigned char fat_type_label[8];
};

struct FSInfo
{
    PartitionBootRecord pbr;
    Fat16ExtBS feb;
    uintptr_t total_sectors;
    uintptr_t fat_size;
    uintptr_t root_dir_sectors;
    uintptr_t first_data_sector;
    uintptr_t first_fat_sector;
    uintptr_t data_sectors;
    uintptr_t total_clusters;
};

class Fat16FileSystem : virtual public Object
{
public:
    enum
    {
        MAX_FILENAME_LENGTH = 15
    };

private:
    inline static Fat16FileSystem *rootfs;

    Disk *disk;
    shared_ptr<File> fat_file;
    FSInfo info;

    Fat16FileSystem(Disk *disk);

public:
    shared_ptr<Directory> getRootDirectory();
    Disk *getDisk() { return disk; }
    shared_ptr<File> getFatFile() { return fat_file; }
    const FSInfo *getInfo() const { return &info; }

    static Fat16FileSystem *getRootFileSystem();
};

#endif
