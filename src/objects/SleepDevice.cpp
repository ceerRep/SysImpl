#include <objects/SleepDevice.hpp>

#include <objects/BlockInputDevice.hpp>

#include <errno.h>
#include <pit.hpp>
#include <string.h>

SleepDevice::SleepDevice()
{
    memset(process_sleep_until, 0, sizeof(process_sleep_until));
}

int64_t SleepDevice::write(const void *data, size_t size)
{
    if (size != 4)
    {
        return -EINVAL;
    }

    auto pid = Process::getCurrentProcess();

    process_sleep_until[pid] = get_tick() + *(uint32_t *)data;

    return size;
}

int64_t SleepDevice::read(void *buffer, size_t size)
{
    auto pid = Process::getCurrentProcess();
    if (process_sleep_until[pid] > get_tick())
        return -EAGAIN;

    return 0;
}

shared_ptr<SleepDevice> SleepDevice::getInstance()
{
    if (!instance)
        instance = make_shared<BlockInputDeviceMixin<SleepDevice>>().cast<SleepDevice>();

    return instance;
}
