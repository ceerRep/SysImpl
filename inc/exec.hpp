#ifndef _exec_hpp

#define _exec_hpp

#include <stdint.h>

class Process;

int execv(Process *proc, char *exec, char **args, uint32_t object_keep_low = 0x7, uint32_t object_keep_high = 0x0);

#endif
