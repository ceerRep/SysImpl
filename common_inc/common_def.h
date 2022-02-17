#ifndef _common_def_h

#define _common_def_h

#define PROCESS_MAX_ARGUMENTS 16
#define PROCESS_RUNTIME_INFO_SIZE 512

#ifndef __ASSEMBLER__

#include <stdint.h>

typedef struct
{
    uint32_t process_runtime_info_addr_arg0;
    uint16_t args[PROCESS_MAX_ARGUMENTS];
    char buffer[PROCESS_RUNTIME_INFO_SIZE - PROCESS_MAX_ARGUMENTS - 4];
} process_runtime_info_t;

typedef struct
{
    char filename[16];
    uint32_t size;
    uint32_t directory;
} file_info_t;

#endif

#endif
