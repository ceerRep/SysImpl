#ifndef _shared_ptr_hpp

#define _shared_ptr_hpp

#include <move.hpp>
#include <new>
#include <type_traits>

// TODO: impl shared_cast and remove all shared_ptr(ValueType*, nullptr)

template <typename ValueType>
class shared_ptr
{
    struct shared_buffer
    {
        ValueType *ptr;
        void (*destructor)(ValueType *);
        mutable int ref;

        // Not need

        int addRef() const
        {
            if (int ret = __sync_add_and_fetch(&ref, 1); ret < 0)
            {
                return 0;
            }
            else
                return ret;
        }

        int decRef()
        {
            int ret = __sync_sub_and_fetch(&ref, 1);
            if (__sync_bool_compare_and_swap(&ref, 0, 0x80000000))
            {
                return 0;
            }
            else
                return ret;
        }
    };
    shared_buffer *buffer;

    template <typename T>
    static void default_destruct(T *v)
    {
        if constexpr (std::is_void_v<T>)
        {
            delete (char *)v;
        }
        else
            delete v;
    }

    explicit shared_ptr(ValueType *pvalue) : shared_ptr(pvalue, &default_destruct<ValueType>)
    {
    }

    template <typename ValueType1>
    friend shared_ptr<ValueType1> create_shared(ValueType1 *ptr);

public:
    shared_ptr() : buffer(nullptr) {}
    shared_ptr(ValueType *pvalue, void (*destructor)(ValueType *))
        : buffer(new shared_buffer{pvalue, destructor, 1})
    {
    }
    shared_ptr(const shared_ptr &s)
        : buffer(s.buffer)
    {
        if (buffer && buffer->addRef() == 0)
            buffer = nullptr;
    }

    ~shared_ptr()
    {
        shared_buffer *pbuffer = (shared_buffer *)__atomic_exchange_n(&buffer, 0, __ATOMIC_SEQ_CST);

        if (pbuffer)
        {
            if (pbuffer->decRef() == 0)
            {
                if (pbuffer->destructor)
                    pbuffer->destructor(pbuffer->ptr);

                delete pbuffer;
            }
            pbuffer = nullptr;
        }
    }

    shared_ptr &operator=(const shared_ptr &r)
    {
        this->~shared_ptr();
        new (this) shared_ptr(r);

        return *this;
    }

    shared_ptr &operator=(const nullptr_t)
    {
        this->~shared_ptr();

        return *this;
    }

    ValueType *operator->()
    {
        return get();
    }

    operator ValueType *()
    {
        return get();
    }

    ValueType *get()
    {
        if (buffer)
            return buffer->ptr;
        return nullptr;
    }

    ValueType *operator->() const
    {
        return get();
    }

    operator ValueType *() const
    {
        return get();
    }

    ValueType *get() const
    {
        if (buffer)
            return buffer->ptr;
        return nullptr;
    }

    operator bool() const
    {
        return buffer;
    }
};

template <typename ValueType>
shared_ptr<ValueType> create_shared(ValueType *ptr)
{
    return shared_ptr<ValueType>(ptr);
}

template <typename ValueType, typename... Args>
shared_ptr<ValueType> make_shared(Args &&...args)
{
    return create_shared(new ValueType(std::forward<Args>(args)...));
}

#endif
