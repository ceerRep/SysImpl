#include <objects/Semaphore.hpp>

#include <errno.h>

int64_t Semaphore::read(void *buffer, size_t size)
{
    if (cnt <= 0)
    {
        return -EAGAIN;
    }

    if (size)
        cnt--;
    
    return 0;
}

int64_t Semaphore::write(const void *data, size_t size)
{
    cnt++;
    return 0;
}
