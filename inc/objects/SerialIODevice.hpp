#ifndef _serialiodevice_hpp

#define _srialiodevice_hpp

#include <stdexcept.h>

#include "InputDevice.hpp"
#include "OutputDevice.hpp"

class SerialIOInitializeError : public std::exception
{
};

class SerialIODevice : public virtual OutputDevice, public virtual InputDevice
{
    int port;

protected:

    void backspace();
    SerialIODevice(int port);

public:

    int empty();
    int getc();
    virtual int64_t read(void *buffer, size_t size);

    int is_transmit_empty();

    int putc0(char ch);
    int putc(char ch);
    virtual int64_t write(const void *data, size_t size) override;

    static shared_ptr<SerialIODevice> createSerialIODevice(int port);
};

#endif
