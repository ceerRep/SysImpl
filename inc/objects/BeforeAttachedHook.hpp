#ifndef _before_attached_hook_hpp

#define _before_attached_hook_hpp

#include "Object.hpp"

class BeforeAttachedHook : public virtual Object
{
public:
    virtual shared_ptr<Object> beforeAttachedToObjectHook(Object *obj, shared_ptr<Object> self) = 0;
};

#endif
