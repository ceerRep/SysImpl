#ifndef _serialiodevice_hpp

#define _srialiodevice_hpp

#include <stdexcept.h>

#include "InputDevice.hpp"
#include "OutputDevice.hpp"

class SerialIOInitializeError : public std::exception {};

class SerialIODevice : virtual public OutputDevice, virtual public InputDevice
{
    int port;

public:
    SerialIODevice(int port);

    virtual int empty() override;
    virtual int getc() override;

    int is_transmit_empty();

    int putc0(char ch);
    virtual int putc(char ch) override;
};

#endif
