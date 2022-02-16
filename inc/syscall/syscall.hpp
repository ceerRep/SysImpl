#ifndef _syscall_hpp

#define _syscall_hpp

#define SYSCALL_EXIT 1
#define SYSCALL_FORK 2
#define SYSCALL_READ 3
#define SYSCALL_WRITE 4
#define SYSCALL_OPEN 5
#define SYSCALL_CLOSE 6
#define SYSCALL_EXEC 11
#define SYSCALL_DUP2 63
#define SYSCALL_SCHED_YIELD 158


#define SYSCALL_SEM_CREATE 200
#define SYSCALL_READDIR 201

#define SYSCALL_HIDDEN_HLT 254
#define SYSCALL_END 255

#endif
