#ifndef _file_hpp

#define _file_hpp

#include "Disk.hpp"
#include "FileSystem.hpp"
#include "InputDevice.hpp"

struct FileStat
{
    enum
    {
        F_REG = 0,
        F_DIR = 1
    };

    uint32_t size;
    uint32_t mode;

    FileStat(uint32_t size, uint32_t mode) : size(size), mode(mode) {}
};

class File : public virtual InputDevice
{
public:
    enum
    {
        SEEK_SET,
        SEEK_CUR,
        SEEK_END
    };
    virtual int seek(int64_t pos, int whence) = 0;
    virtual uint64_t tell() = 0;

    virtual FileStat stat() = 0;
    virtual Fat16FileSystem* getFS() = 0;

    static shared_ptr<File> createSequentialFile(Fat16FileSystem *fs, uint32_t begin, uint32_t end);
    static shared_ptr<File> createFileByCluster(Fat16FileSystem *fs, uint32_t start_cluster_pos, FileStat stat);
};

#endif
