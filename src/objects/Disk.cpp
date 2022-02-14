#include <objects/Disk.hpp>

#include <8086.h>
#include <assert.h>
#include <string.h>
#include <ld_syms.h>
#include <shared_ptr.hpp>
#include <protected_mode.hpp>

struct __attribute__((packed)) PartitionTableEntry
{
    uint8_t attr;
    unsigned chs_start : 24;
    uint8_t type;
    unsigned chs_end : 24;
    uint32_t lba_start;
    uint32_t sector_count;
};

struct __attribute__((packed)) MasterBootRecord
{
    char bootstrap[440];
    uint32_t disk_id;
    uint16_t reserved;
    PartitionTableEntry parts[4];
};

struct __attribute__((packed)) DiskAddressPacket
{
    uint8_t size;
    uint8_t zero;
    uint16_t sector_count;
    uint16_t buffer_offset;
    uint16_t buffer_base;
    uint64_t lba;
};

class PhysicalDisk : public Disk
{
    uint8_t drive_number;

public:
    PhysicalDisk(uint8_t drive_number) : drive_number(drive_number) {}

    virtual int readSector(uintptr_t lba, void *buffer) override
    {
        DiskAddressPacket *packet = (DiskAddressPacket *)(real_mode_buffer - code16_source_start + CODE16);
        char *buffer_realmode = (char *)(packet + 1);
        packet->size = sizeof(DiskAddressPacket);
        packet->zero = 0;
        packet->buffer_base = 0;
        packet->buffer_offset = (uintptr_t)buffer_realmode;
        packet->sector_count = 1;
        packet->lba = lba;
        int ret = v8086_call(
            code16_source_int13,
            0x4200,
            0,
            0,
            drive_number,
            (uintptr_t)packet,
            0,
            0,
            0);

        memcpy(buffer, buffer_realmode, 512);

        return (ret >> 8) & 0xff == 0;
    }
};

class LogicalDisk : public Disk
{
    shared_ptr<Disk> parent;
    uint32_t begin_lba;

public:
    LogicalDisk(shared_ptr<Disk> parent, uint32_t begin_lba) : parent(parent), begin_lba(begin_lba) {}

    virtual int readSector(uintptr_t lba, void *buffer) override
    {
        return parent->readSector(lba + begin_lba, buffer);
    }
};

Disk *Disk::getBootDisk(uint8_t *seq)
{
    if (!boot_disk)
    {
        assert("Only support top level partition" && seq[2] == 0xFF);

        Disk *now = new PhysicalDisk(seq[0]);
        MasterBootRecord buffer;
        now->readSector(0, &buffer);

        if (seq[1] != 0xFF)
        {
            now = new LogicalDisk(create_shared(now), buffer.parts[seq[1]].lba_start);
        }

        boot_disk = now;
    }

    return boot_disk;
}
