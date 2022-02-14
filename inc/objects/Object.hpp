#ifndef _object_hpp

#define _object_hpp

#include <shared_ptr.hpp>

class Object
{
public:
    Object() = default;
    Object(const Object &) = delete;
    virtual ~Object() = default;
    virtual void onRemovedByOwner(Object *owner) {}
};

#endif
