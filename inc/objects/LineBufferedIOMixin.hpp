#ifndef _line_buffered_io_mixin_hpp

#define _line_buffered_io_mixin_hpp

#include <errno.h>
#include <move.hpp>
#include <stdio.h>

// requires: Base --> InputDevice OutputDevice void backspace()
template <typename Base>
class LineBufferedIOMixin : public Base
{
    enum
    {
        BUFFER_SIZE = 256
    };

    bool eof = 0;
    char buffer[256];
    int buffer_start = 0, buffer_end = 0;

    char buffer_committed[256];
    int buffer_committed_start = 0, buffer_committed_end = 0;

public:
    template <typename... Args>
    LineBufferedIOMixin(Args &&...args) : Base(std::forward<Args>(args)...) {}

    void commit()
    {
        while (buffer_start != buffer_end &&
               (buffer_committed_end + 1) % BUFFER_SIZE != buffer_committed_start)
        {
            buffer_committed[buffer_committed_end] = buffer[buffer_start];
            buffer_committed_end = (buffer_committed_end + 1) % BUFFER_SIZE;
            buffer_start = (buffer_start + 1) % BUFFER_SIZE;
        }

        // clear buffer

        buffer_start = buffer_end = 0;
    }

    virtual int64_t read(void *_buf, size_t size) override
    {
        if (eof)
        {
            eof = size == 0;
            return 0;
        }

        char *buf = (char *)_buf;
        // fill buffer
        uint8_t ch;

        while (Base::read(&ch, 1) > 0)
        {
            if (ch >= 128)
                continue;

            if (ch == '\x04') // C-d commit now
            {
                if (buffer_start == buffer_end && buffer_committed_start == buffer_committed_end) // empty, send EOF
                {
                    eof = size == 0;
                    return 0;
                }
                else
                {
                    commit();
                    break;
                }
            }
            else if (ch == '\x08') // backspace
            {
                if (buffer_start != buffer_end)
                {
                    this->backspace();
                    buffer_end = (buffer_end - 1 + BUFFER_SIZE) % BUFFER_SIZE;
                }
            }
            else if ((buffer_end + 1) % BUFFER_SIZE != buffer_start)
            {
                buffer[buffer_end] = ch;
                buffer_end = (buffer_end + 1) % BUFFER_SIZE;

                if (ch == '\n')
                    commit();

                this->write(&ch, 1);
            }
        }

        if (buffer_committed_start == buffer_committed_end)
        {
            return -EAGAIN;
        }

        int i = 0;
        for (; i < size && buffer_committed_start != buffer_committed_end;)
        {
            buf[i++] = buffer_committed[buffer_committed_start++];
            buffer_committed_start %= BUFFER_SIZE;
        }

        return i;
    }
};

#endif
