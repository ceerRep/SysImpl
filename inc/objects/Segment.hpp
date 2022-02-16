#ifndef _segment_hpp

#define _segment_hpp

#include "BeforeAttachedHook.hpp"

#include <stddef.h>
#include <heap.h>

#include <shared_ptr.hpp>

class Segment : public virtual BeforeAttachedHook
{
    uintptr_t base;
    uintptr_t size;

    Segment() : size(0) {}

public:
    Segment(uintptr_t base, uintptr_t size) : base(base), size(size) {}

    uintptr_t getBase()
    {
        return base;
    }

    uintptr_t getSize()
    {
        return size;
    }

    virtual shared_ptr<Object> beforeAttachedToObjectHook(Object *obj, shared_ptr<Object> self) override { return self; };
};

#endif
