#ifndef _wait_process_hpp

#define _wait_process_hpp

#include "InputDevice.hpp"
#include "OutputDevice.hpp"
#include "Process.hpp"

class ProcessWait : public virtual InputDevice, public virtual OutputDevice
{
    struct wait_req
    {
        // 0 mean no blocking
        int pid;
        int status;
    } reqs[Process::MAX_PROCESS_NUM];

    int last_exit_code[Process::MAX_PROCESS_NUM];
protected:
    ProcessWait();

    inline static shared_ptr<ProcessWait> instance;
public:

    virtual int64_t read(void *buffer, size_t size) override;
    virtual int64_t write(const void *data, size_t size) override;

    void processExit(int pid, int status);

    static shared_ptr<ProcessWait> getInstance();
};

#endif
