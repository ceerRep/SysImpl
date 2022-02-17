#include <objects/BlockInputDevice.hpp>

#include <errno.h>
#include <objects/Process.hpp>
#include <stdio.h>
#include <syscall/syscall_handler.hpp>

BlockInputDeviceWrapper::BlockInputDeviceWrapper(shared_ptr<InputDevice> input)
    : pending_request_end(0), pending_request(create_shared(new PendingRead[Process::MAX_PROCESS_NUM])), input(input)
{
    for (int i = 0; i < MAX_BLOCK_INPUT_DEVICE_NUM; i++)
        if (devices[i] == nullptr)
        {
            devices[i] = this;
            return;
        }

    throw BlockInputDeviceLimitExceededException();
}

BlockInputDeviceWrapper::~BlockInputDeviceWrapper()
{
    for (int i = 0; i < MAX_BLOCK_INPUT_DEVICE_NUM; i++)
        if (devices[i] == this)
            devices[i] = nullptr;
}

int64_t BlockInputDeviceWrapper::do_read(Process *proc, void *buffer, size_t size)
{
    int ret = input->read(buffer, size);

    // block
    if (ret == -EAGAIN)
    {
        pending_request[pending_request_end++] = {proc, buffer, size};
        proc->setProcessState(Process::PROCESS_STATE_SLEEPING);
    }

    return ret;
}

int64_t BlockInputDeviceWrapper::read(void *buffer, size_t size)
{
    return do_read(Process::getProcess(Process::getCurrentProcess()), buffer, size);
}

void BlockInputDeviceWrapper::onRemovedByOwner(Object *owner)
{
    int owner_pos;

    for (owner_pos = 0; owner_pos < pending_request_end; owner_pos++)
        if (pending_request[owner_pos].proc == owner)
        {
            while (owner_pos + 1 < pending_request_end)
            {
                pending_request[owner_pos] = pending_request[owner_pos + 1];
                owner_pos++;
            }

            pending_request_end--;

            break;
        }

    input->onRemovedByOwner(owner);
}

void BlockInputDeviceWrapper::check()
{
    auto now_running_process = Process::getCurrentProcess();

    int ind = 0;
    for (ind = 0; ind < pending_request_end && input->read(nullptr, 0) != -EAGAIN; ind++)
    {
        auto now = pending_request[ind];

        Process::setCurrentProcess(now.proc->getPid());

        now.proc->setProcessState(Process::PROCESS_STATE_RUNNABLE);
        int ret = do_read(now.proc, now.buffer, now.size);
        syscall_set_retval(now.proc, ret);
    }

    pending_request_end -= ind;

    for (int i = 0; i < pending_request_end; i++)
        pending_request[i] = pending_request[i + ind];

    Process::setCurrentProcess(now_running_process);
}

void BlockInputDeviceWrapper::checkAll()
{
    for (int i = 0; i < MAX_BLOCK_INPUT_DEVICE_NUM; i++)
        if (devices[i])
            devices[i]->check();
}
