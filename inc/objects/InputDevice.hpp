#ifndef _inputdevice_hpp

#define _inputdevice_hpp

#include "Object.hpp"

class InputDevice : virtual public Object
{
public:
    virtual int getc() = 0;
    virtual int empty() = 0;
};

#endif
