#include <objects/Directory.hpp>

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <objects/FileSystem.hpp>

struct __attribute__((packed)) FileEntry
{
    enum
    {
        READ_ONLY = 0x01,
        HIDDEN = 0x02,
        SYSTEM = 0x04,
        VOLUME_ID = 0x08,
        DIRECTORY = 0x10,
        ARCHIVE = 0x20,
        LFN = READ_ONLY | HIDDEN | SYSTEM | VOLUME_ID
    };
    char filename[8];
    char extname[3];
    uint8_t attr;
    uint8_t reserved;
    uint8_t create_time_milli_seconds;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t access_date;
    uint16_t cluster_high;
    uint16_t modify_date;
    uint16_t modify_date1;
    uint16_t cluster_low;
    uint32_t size;
};

Directory::Directory(Fat16FileSystem *fs, uint32_t first_cluster)
    : fs(fs), file(create_shared(File::createFileByCluster(fs, first_cluster, 0x7fffffff)))
{
    init();
}

Directory::Directory(Fat16FileSystem *fs, uint32_t begin, uint32_t end)
    : fs(fs), file(create_shared(File::createSequentialFile(fs->getDisk(), begin, end)))
{
    init();
}

void Directory::init()
{
    FileEntry entry;

    int count = 0;
    while (file->read(&entry, sizeof(entry)) == sizeof(entry))
    {
        if (*(char *)&entry == '\0')
            break;

        if (!(entry.attr & FileEntry::VOLUME_ID))
        {
            count++;
        }
    }

    stats = create_shared(new FileStatEx[count]);

    file->seek(0, File::SEEK_SET);

    file_num = count;
    count = 0;
    while (file->read(&entry, sizeof(entry)) == sizeof(entry))
    {
        if (*(char *)&entry == '\0')
            break;

        if (!(entry.attr & FileEntry::VOLUME_ID))
        {
            assert(count < file_num);

            char filename[16];
            char *pos = filename;

            for (int i = 0; i < 8; i++)
            {
                if (entry.filename[i] && entry.filename[i] != ' ')
                    *(pos++) = entry.filename[i];
                else
                    break;
            }

            if (entry.extname[0] && entry.extname[0] != ' ')
            {
                *(pos++) = '.';
                for (int i = 0; i < 3; i++)
                {
                    if (entry.extname[i] && entry.extname[i] != ' ')
                        *(pos++) = entry.extname[i];
                    else
                        break;
                }
            }
            *(pos++) = 0;

            strcpy(stats[count].filename, filename);

            if (entry.attr & FileEntry::DIRECTORY)
                stats[count].directory = 1;

            stats[count].size = entry.size;
            stats[count].cluster = (entry.cluster_high << 16) + entry.cluster_low;

            count++;
        }
    }
}

File *Directory::openFile(const char *name)
{
    for (int i = 0, end = file_num; i < end; i++)
    {
        if (strcasecmp(stats[i].filename, name) == 0)
        {
            return File::createFileByCluster(fs, stats[i].cluster, stats[i].size);
        }
    }

    return nullptr;
}

Directory *Directory::openDirectory(const char *name)
{
    for (int i = 0, end = file_num; i < end; i++)
    {
        if (strcasecmp(stats[i].filename, name) == 0)
        {
            if (!stats[i].directory)
                return nullptr;
            
            return new Directory(fs, stats[i].cluster);
        }
    }

    return nullptr;
}
