#ifndef _block_input_device

#define _block_input_device

#include "InputDevice.hpp"

#include <move.hpp>
#include <shared_ptr.hpp>
#include <stdexcept.h>

class Process;

struct BlockInputDeviceLimitExceededException : public std::exception
{
    virtual const char *what() const throw() override
    {
        return "block input device limit exceeded";
    }
};

class BlockInputDeviceWrapper : virtual public InputDevice
{
    enum
    {
        MAX_BLOCK_INPUT_DEVICE_NUM = 64
    };

    struct PendingRead
    {
        Process *proc;
        void *buffer;
        size_t size;
    };

    int pending_request_end;
    shared_ptr<PendingRead> pending_request;
    shared_ptr<InputDevice> input;

    inline static BlockInputDeviceWrapper *devices[MAX_BLOCK_INPUT_DEVICE_NUM] = {nullptr};

public:
    BlockInputDeviceWrapper(shared_ptr<InputDevice> input);
    ~BlockInputDeviceWrapper();

    int64_t do_read(Process *proc, void *buffer, size_t size);
    virtual int64_t read(void *buffer, size_t size) override;

    virtual void onRemovedByOwner(Object *owner) override;

    void check();

    static void checkAll();
};

template <typename Base>
class BlockInputDeviceMixin : virtual public Base, virtual public BlockInputDeviceWrapper
{
    struct BaseWrapper : virtual public InputDevice
    {
        BlockInputDeviceMixin<Base> *parent;

        BaseWrapper(BlockInputDeviceMixin<Base> *parent) : parent(parent) {}

        virtual int64_t read(void *buffer, size_t size) override
        {
            return parent->Base::read(buffer, size);
        }

        virtual void onRemovedByOwner(Object *owner) override
        {
            parent->Base::onRemovedByOwner(owner);
        }
    };

    BaseWrapper wrapper;

public:
    template <typename... Args>
    BlockInputDeviceMixin(Args &&...args)
        : Base(std::forward<Args>(args)...),
          BlockInputDeviceWrapper(shared_ptr<InputDevice>(&wrapper, nullptr)), // dummy shared ptr
          wrapper{this}
    {
    }

    virtual int64_t read(void *buffer, size_t size) override
    {
        return BlockInputDeviceWrapper::read(buffer, size);
    }

    virtual void onRemovedByOwner(Object *owner) override
    {
        // BlockInputDeviceWrapper::onRemovedByOwner will call Base::onRemovedByOwner
        BlockInputDeviceWrapper::onRemovedByOwner(owner);
    }
};

#endif
