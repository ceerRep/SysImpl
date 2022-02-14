#ifndef _file_hpp

#define _file_hpp

#include "Disk.hpp"
#include "FileSystem.hpp"
#include "InputDevice.hpp"

class File : virtual public InputDevice
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

    static File *createSequentialFile(Disk *disk, uint32_t begin, uint32_t end);
    static File *createFileByCluster(Fat16FileSystem *fs, uint32_t start_cluster_pos, uint32_t size);
};

#endif
