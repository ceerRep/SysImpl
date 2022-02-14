#ifndef _semaphore_hpp

#define _semaphore_cpp

#include "BlockInputDevice.hpp"
#include "OutputDevice.hpp"

class Semaphore : public virtual InputDevice, public virtual OutputDevice
{
    int cnt;

protected:
    Semaphore(int cnt) : cnt(cnt) {}

public:
    // SEM_WAIT
    virtual int64_t read(void *buffer, size_t size) override;
    // SEM_SIGNAL
    virtual int64_t write(const void *data, size_t size) override;

    static shared_ptr<Object> createSemaphore(int cnt)
    {
        return create_shared<Object>(new BlockInputDeviceMixin<Semaphore>(cnt));
    }
};

#endif
