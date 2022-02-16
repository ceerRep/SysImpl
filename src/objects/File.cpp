#include <align_util.h>
#include <assert.h>
#include <objects/File.hpp>
#include <objects/FileSystem.hpp>
#include <stdio.h>

class SequentialFile : public File
{
    uint32_t cursor;
    uint32_t begin, end;
    uint32_t filesize;

    Fat16FileSystem *fs;

    int64_t buffer_pos;
    shared_ptr<uint8_t> _buffer;
    uint8_t *buffer;

public:
    SequentialFile(Fat16FileSystem *fs, uint32_t begin, uint32_t end)
        : cursor(0),
          begin(begin),
          end(end),
          filesize((end - begin) * 512),
          fs(fs),
          buffer_pos(-1),
          _buffer(create_shared(new uint8_t[512])),
          buffer(_buffer.get())
    {
    }

    void ensureCursorInBuffer()
    {
        uint32_t cursor_sector = cursor / 512;

        if (cursor_sector != buffer_pos)
        {
            fs->getDisk()->readSector(cursor_sector + begin, buffer);
            buffer_pos = cursor_sector;
        }
    }

    int getc()
    {
        if (cursor >= filesize)
            return EOF;

        ensureCursorInBuffer();

        return buffer[(cursor++) % 512];
    }

    virtual int64_t read(void *buffer, size_t size) override
    {
        size_t i;
        int ch;

        for (i = 0; i < size; i++)
        {
            ch = getc();

            if (ch != EOF)
                ((char *)buffer)[i] = ch;
            else
                break;
        }

        return i;
    }

    virtual int seek(int64_t pos, int whence) override
    {
        switch (whence)
        {
        case SEEK_CUR:
            cursor += pos;
            break;

        case SEEK_SET:
            cursor = pos;
            break;

        case SEEK_END:
            cursor = filesize + pos;
            break;

        default:
            return -1;
        }

        return 0;
    }

    virtual uint64_t tell() override
    {
        return cursor;
    }

    virtual FileStat stat() override
    {
        return FileStat{filesize, FileStat::F_DIR};
    }

    virtual Fat16FileSystem *getFS() override
    {
        return fs;
    }
};

class ClusterFile : public File
{
    enum
    {
        CLUSTER_END = 0xFFF8
    };

    Fat16FileSystem *fs;
    Disk *disk;
    uint32_t start_cluster_pos;

    uint32_t cursor;
    uint32_t cluster_num;

    FileStat _stat;

    shared_ptr<uint16_t> clusters;

    int64_t buffer_pos;
    shared_ptr<uint8_t> _buffer;
    uint8_t *buffer;

    shared_ptr<File> fat_file;

public:
    ClusterFile(Fat16FileSystem *fs, uint32_t start_cluster_pos, FileStat stat)
        : fs(fs),
          disk(fs->getDisk()),
          start_cluster_pos(start_cluster_pos),
          _stat(stat),
          _buffer(create_shared(new uint8_t[512])),
          buffer_pos(-1),
          buffer(_buffer.get()),
          fat_file(fs->getFatFile())
    {
        cursor = 0;

        int count = 0;
        auto info = fs->getInfo();
        uint16_t now_cluster = start_cluster_pos;

        if (_stat.mode == FileStat::F_DIR)
            _stat.size = 0;

        while (now_cluster < CLUSTER_END)
        {
            if (_stat.mode == FileStat::F_DIR)
                _stat.size += info->pbr.sectors_per_cluster * 512;

            fat_file->seek(now_cluster * sizeof(uint16_t), File::SEEK_SET);
            fat_file->read(&now_cluster, sizeof(now_cluster));
            count++;
        }

        clusters = create_shared(new uint16_t[count]);

        cluster_num = count;

        if (cluster_num * info->pbr.sectors_per_cluster * 512 > _stat.size)
            _stat.size = cluster_num * info->pbr.sectors_per_cluster * 512;

        count = 0;
        now_cluster = start_cluster_pos;

        while (now_cluster < CLUSTER_END && (int64_t)count * info->pbr.sectors_per_cluster * 512 < _stat.size)
        {
            assert(count < cluster_num);
            clusters[count] = now_cluster;
            fat_file->seek(now_cluster * sizeof(uint16_t), File::SEEK_SET);
            fat_file->read(&now_cluster, sizeof(now_cluster));
            count++;
        }
    }

    void ensureCursorInBuffer()
    {
        auto pinfo = fs->getInfo();
        uint32_t cursor_sector = cursor / 512;
        uint32_t cursor_cluster_ind = cursor_sector / pinfo->pbr.sectors_per_cluster;
        cursor_sector = cursor_sector % pinfo->pbr.sectors_per_cluster;

        uint32_t cursor_cluster = clusters[cursor_cluster_ind];
        cursor_sector += ((cursor_cluster - 2) * pinfo->pbr.sectors_per_cluster) + pinfo->first_data_sector;

        if (cursor_sector != buffer_pos)
        {
            disk->readSector(cursor_sector, buffer);
            buffer_pos = cursor_sector;
        }
    }

    int getc()
    {
        if (cursor >= _stat.size)
            return EOF;

        ensureCursorInBuffer();

        return buffer[(cursor++) % 512];
    }

    virtual int64_t read(void *buffer, size_t size) override
    {
        size_t i;
        int ch;

        for (i = 0; i < size; i++)
        {
            ch = getc();

            if (ch != EOF)
                ((char *)buffer)[i] = ch;
            else
                break;
        }

        return i;
    }

    virtual int seek(int64_t pos, int whence) override
    {
        switch (whence)
        {
        case SEEK_CUR:
            cursor += pos;
            break;

        case SEEK_SET:
            cursor = pos;
            break;

        case SEEK_END:
            cursor = _stat.size + pos;
            break;

        default:
            return -1;
        }

        return 0;
    }

    virtual uint64_t tell() override
    {
        return cursor;
    }

    virtual FileStat stat() override
    {
        return _stat;
    }

    virtual Fat16FileSystem *getFS() override
    {
        return fs;
    }
};

shared_ptr<File> File::createSequentialFile(Fat16FileSystem *fs, uint32_t begin, uint32_t end)
{
    return make_shared<SequentialFile>(fs, begin, end).cast<File>();
}

shared_ptr<File> File::createFileByCluster(Fat16FileSystem *fs, uint32_t start_cluster_pos, FileStat stat)
{
    return make_shared<ClusterFile>(fs, start_cluster_pos, stat).cast<File>();
}
