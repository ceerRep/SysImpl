#ifndef _common_def_h

#define _common_def_h

#define PROCESS_MAX_ARGUMENTS 16
#define PROCESS_RUNTIME_INFO_SIZE 1024

#ifndef __ASSEMBLER__

#include <stdint.h>

typedef struct
{
    uint16_t args[PROCESS_MAX_ARGUMENTS];
    char buffer[PROCESS_RUNTIME_INFO_SIZE - PROCESS_MAX_ARGUMENTS];
} process_runtime_info_t;

#endif

#endif
