#ifndef _elf_loader_hpp

#define _elf_loader_hpp

#include <stdexcept.h>
#include <string.h>

struct Section;

class InvalidElfException : public std::exception
{
    char _reason[32];

public:
    InvalidElfException(const char* reason)
    {
        for (int i = 0; i < 32 && reason[i]; i++)
            _reason[i] = reason[i];
        
        _reason[31] = 0;
    }

    virtual const char* what() const throw() override
    {
        return _reason;
    }
};

int parseElfSections(const uint8_t *buffer, Section *sections, uint32_t *entry, int n);

#endif
