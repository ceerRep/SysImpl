#ifndef _vgatextiodevice_hpp

#define _vgatextiodevice_hpp

#include "EarlyStageOutputDevice.hpp"
#include "InputDevice.hpp"
#include "OutputDevice.hpp"

class VGATextIODevice : public EarlyStageOutputDevice, public virtual InputDevice
{
    char *cursor;

protected:
    VGATextIODevice();

public:
    int putc0(char ch);

    int getc0();

    int empty();

    void backspace();

    virtual int64_t write(const void *data, size_t size) override;
    virtual int64_t read(void *buffer, size_t size) override;

    static shared_ptr<VGATextIODevice> getInstance();
};

#endif
