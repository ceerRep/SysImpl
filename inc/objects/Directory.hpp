#ifndef _directory_hpp

#define _directory_hpp

#include "Object.hpp"
#include "File.hpp"
#include "FileSystem.hpp"

struct Fat16FileSystem;

struct FileStat
{
    char filename[15];
    bool directory;
    size_t size;
};

class Directory : virtual public Object
{
    Fat16FileSystem *fs;
    shared_ptr<File> file;

    struct FileStatEx : FileStat
    {
        uint32_t cluster;
    };

    int file_num;
    shared_ptr<FileStatEx> stats;

    void init();

public:
    Directory(Fat16FileSystem *fs, uint32_t first_cluster);
    Directory(Fat16FileSystem *fs, uint32_t begin, uint32_t end);

    int size() const { return file_num; }

    const FileStat &operator[](int index) const { return stats[index]; }

    File *openFile(const char *name);
    Directory *openDirectory(const char *name);
};

#endif
