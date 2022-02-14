#ifndef _disk_hpp

#define _disk_hpp

#include <objects/Object.hpp>

class Disk : virtual public Object
{
    inline static Disk *boot_disk = nullptr;

public:
    virtual int readSector(uintptr_t ind, void *buffer) = 0;

    static Disk* getBootDisk(uint8_t *seq);
};

#endif
