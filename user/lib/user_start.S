#include <common_def.h>

.global start

.data
5:
.fill 4 * (PROCESS_MAX_ARGUMENTS + 1), 1, 0
.text

start:

movl $PROCESS_MAX_ARGUMENTS, %ecx

movl $0, %eax

2:
movzwl (%ebx, %eax, 0x2), %edx
cmpl $0xFFFF, %edx
jz 3f
lea 2*PROCESS_MAX_ARGUMENTS(%ebx, %edx), %esi
mov %esi, 5b(, %eax, 0x4)
inc %eax
loop 2b

xor %esi, %esi
mov %esi, 5b(, %eax, 0x4)

3:
mov $5b, %ecx
mov %ecx, crt_argv
mov %eax, crt_argc
call crt_call_main
