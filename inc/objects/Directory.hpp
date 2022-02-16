#ifndef _directory_hpp

#define _directory_hpp

#include "File.hpp"
#include "Object.hpp"

#include <stdexcept.h>

struct Fat16FileSystem;

struct InvalidFileModeException : public std::exception
{
    virtual const char *what() const throw() override
    {
        return "invalid file mode";
    }
};

struct FileInfo
{
    char filename[15];
    bool directory;
    size_t size;
};

class Directory : public virtual Object
{
    Fat16FileSystem *fs;
    shared_ptr<File> file;

    struct FileInfoEx : FileInfo
    {
        uint32_t cluster;
    };

    int file_num;
    shared_ptr<FileInfoEx> infos;

    void init();

public:
    Directory(Fat16FileSystem *fs, uint32_t first_cluster);
    Directory(Fat16FileSystem *fs, uint32_t begin, uint32_t end);
    Directory(Fat16FileSystem *fs, shared_ptr<File> file);

    shared_ptr<File> asFile() { return file; }

    int size() const { return file_num; }

    const FileInfo &operator[](int index) const { return infos[index]; }

    shared_ptr<File> openFile(const char *name);
    static shared_ptr<Directory> fromFile(shared_ptr<File> dir);
};

#endif
