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
    if (pending_request_end)
    {
        int ret = input->read(nullptr, 0);

        if (ret != -EAGAIN)
        {
            if (pending_request_end)
            {
                auto first = pending_request[0];
                auto now_running_process = Process::getCurrentProcess();

                Process::setCurrentProcess(first.proc->getPid());

                for (int i = 1; i < pending_request_end; i++)
                    pending_request[i - 1] = pending_request[i];
                pending_request_end--;

                first.proc->setProcessState(Process::PROCESS_STATE_RUNNABLE);
                ret = do_read(first.proc, first.buffer, first.size);
                syscall_set_retval(first.proc, ret);

                Process::setCurrentProcess(now_running_process);
            }
        }
    }
}

void BlockInputDeviceWrapper::checkAll()
{
    for (int i = 0; i < MAX_BLOCK_INPUT_DEVICE_NUM; i++)
        if (devices[i])
            devices[i]->check();
}
