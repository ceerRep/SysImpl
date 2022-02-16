#ifndef _sleep_hpp

#define _sleep_hpp

#include "InputDevice.hpp"
#include "OutputDevice.hpp"
#include "Process.hpp"

class SleepDevice : public virtual InputDevice, public virtual OutputDevice
{
    uint64_t process_sleep_until[Process::MAX_PROCESS_NUM];
protected:
    SleepDevice();

    inline static shared_ptr<SleepDevice> instance;
public:

    virtual int64_t read(void *buffer, size_t size) override;
    virtual int64_t write(const void *data, size_t size) override;

    static shared_ptr<SleepDevice> getInstance();
};

#endif
