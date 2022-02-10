#include <objects/OutputDevice.hpp>

#include <stdint.h>

int OutputDevice::write(void *data, size_t size)
{
    int i;
    for (i = 0; i < size; i++)
    {
        char ch = ((char *)data)[i];
        if (this->putc(ch) != ch)
            return i;
    }
    return i;
}

int OutputDevice::puts(const char *str)
{
    int i;
    for (i = 0; str[i]; i++)
    {
        char ch = str[i];
        if (this->putc(ch) != ch)
            return -i;
    }
    return i;
}

int OutputDevice::printf(const char *fmt, ...)
{
    enum
    {
        PRI_UNK = 0,
        PRI_CHAR = 1,
        PRI_I32 = 11,
        PRI_I64 = 21,
        PRI_U32 = 10,
        PRI_U64 = 20,
        PRI_X32 = 8,
        PRI_X64 = 16,
        PRI_STR = -1
    };

    enum
    {
        PRINTF_BUFFER_SIZE = 64
    };

    int cnt = 0;
    char buffer[PRINTF_BUFFER_SIZE];
    const static char hex[] = "0123456789ABCDEF";

    char *buffer_pos = buffer;

    char *now_arg = (char *)(&fmt) + sizeof(const char *);

    while (*fmt)
    {
        int type = PRI_UNK;
        int prefix_zero = 0;
        int output_length = 0;
        uint64_t data;

        if (*fmt == '%')
        {
            fmt++;

            int l_num = 0;
            uintptr_t data_size = 0;

            for (;;)
            {
                switch (*fmt)
                {
                case '0' ... '9':
                {
                    int num = *fmt - '0';

                    if (prefix_zero == 0 && num == 0)
                    {
                        prefix_zero = 1;
                    }
                    else
                    {
                        output_length = output_length * 10 + num;
                    }

                    break;
                }
                case 'd':
                    if (l_num <= 1)
                    {
                        data_size = 4;
                        type = PRI_I32;
                    }
                    else
                    {
                        data_size = 8;
                        type = PRI_I64;
                    }
                    break;
                case 'u':
                    if (l_num <= 1)
                    {
                        data_size = 4;
                        type = PRI_U32;
                    }
                    else
                    {
                        data_size = 8;
                        type = PRI_U64;
                    }
                    break;
                case 'x':
                    if (l_num <= 1)
                    {
                        data_size = 4;
                        type = PRI_X32;
                    }
                    else
                    {
                        data_size = 8;
                        type = PRI_X64;
                    }
                    break;
                case 'l':
                    l_num++;

                    if (l_num == 3)
                    {
                        data_size = 0;
                        type = PRI_STR;
                        data = (uint64_t)(uintptr_t) "<%lll is too long for printf>";
                    }

                    break;
                case 's':
                    data_size = sizeof(char *);
                    type = PRI_STR;
                    break;
                case 'c':
                    data_size = 4;
                    type = PRI_CHAR;
                    break;
                case '%':
                    data_size = 0;
                    type = PRI_CHAR;
                    data = '%';
                default:
                    data_size = 0;
                    type = PRI_CHAR;
                    data = *fmt;
                }

                if (type != PRI_UNK)
                    break;

                fmt++;
            }

            switch (data_size)
            {
            case 4:
                data = *(uint32_t *)now_arg;
                break;
            case 8:
                data = *(uint64_t *)now_arg;
                break;
            }

            now_arg = now_arg + data_size;
        }
        else
        {
            type = PRI_CHAR;
            data = *fmt;
        }

        if (*fmt)
            fmt++;

        if (output_length > type)
            output_length = type;

        if (type == PRI_STR || buffer_pos + type + 1 >= buffer + PRINTF_BUFFER_SIZE)
        {
            *buffer_pos = 0;
            if (int cnt1 = puts(buffer); cnt1 < 0)
            {
                cnt += -cnt1;
                return -cnt;
            }
            else
                cnt += cnt1;

            buffer_pos = buffer;
        }

        if (type == PRI_STR)
        {
            if (int cnt1 = puts((char *)(uintptr_t)data); cnt1 < 0)
            {
                cnt += -cnt1;
                return -cnt;
            }
            else
                cnt += cnt1;
        }
        else if (type == PRI_CHAR)
        {
            *(buffer_pos++) = (char)data;
        }
        else // INTS
        {
            if (type == PRI_I32 || type == PRI_U32 || type == PRI_I64 || type == PRI_U64 || type == PRI_X32 || type == PRI_X64)
            {
                int print_minus = 0;
                int printed = 0;
                int base = 10;

                if (type == PRI_I32 || type == PRI_U32 || type == PRI_I64 || type == PRI_U64)
                    base = 10;
                else if (type == PRI_X32 || type == PRI_X64)
                    base = 16;

                if (type == PRI_I32)
                {
                    if ((int32_t)data < 0)
                    {
                        data = -(int32_t)data;
                        print_minus = 1;

                        if (prefix_zero)
                            output_length--;
                    }
                }
                else if (type == PRI_I64)
                {
                    if ((int64_t)data < 0)
                    {
                        data = -(int64_t)data;
                        print_minus = 1;

                        if (prefix_zero)
                            output_length--;
                    }
                }

                char *start = buffer_pos, *end = buffer_pos;
                while (data)
                {
                    printed = 1;
                    *(end++) = hex[data % base];
                    data /= base;
                }

                if (!printed)
                {
                    *(end++) = '0';
                }

                if (print_minus && !prefix_zero)
                {
                    *(end++) = '-';
                }

                while (end - start < output_length)
                {
                    *(end++) = prefix_zero ? '0' : ' ';
                }

                if (print_minus && prefix_zero)
                {
                    *(end++) = '-';
                }

                buffer_pos = end;
                end--;

                while (start < end)
                {
                    char t = *start;
                    *start = *end;
                    *end = t;
                    start++;
                    end--;
                }
            }
        }
    }

    *buffer_pos = 0;
    if (int cnt1 = puts(buffer); cnt1 < 0)
    {
        cnt += -cnt1;
        return -cnt;
    }
    else
        cnt += cnt1;

    return cnt;
}
