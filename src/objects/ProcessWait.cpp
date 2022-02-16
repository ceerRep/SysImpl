#include <objects/ProcessWait.hpp>

#include <errno.h>
#include <string.h>

#include <objects/BlockInputDevice.hpp>
#include <objects/Process.hpp>

ProcessWait::ProcessWait()
{
    memset(reqs, 0, sizeof(reqs));
}

int64_t ProcessWait::write(const void *data, size_t size)
{
    if (size != 4)
    {
        return -EINVAL;
    }

    auto pid = Process::getCurrentProcess();
    auto target_pid = *(uint32_t *)data;

    if (Process::getProcess(target_pid))
    {
        reqs[pid].pid = target_pid;
        reqs[pid].status = 0;
    }
    else
    {
        reqs[pid].pid = 0;
        reqs[pid].status = last_exit_code[target_pid];
    }

    return size;
}

int64_t ProcessWait::read(void *buffer, size_t size)
{
    auto pid = Process::getCurrentProcess();
    if (reqs[pid].pid)
        return -EAGAIN;

    if (size == 4)
    {
        *(int32_t *)buffer = reqs[pid].status;
        return 4;
    }

    return -EINVAL;
}

void ProcessWait::processExit(int pid, int status)
{
    last_exit_code[pid] = status;

    for (auto &req : reqs)
    {
        if (req.pid == pid)
        {
            req.status = status;
            req.pid = 0;
        }
    }
}

shared_ptr<ProcessWait> ProcessWait::getInstance()
{
    if (!instance)
        instance = make_shared<BlockInputDeviceMixin<ProcessWait>>().cast<ProcessWait>();

    return instance;
}
