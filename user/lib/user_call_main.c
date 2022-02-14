#include <stdint.h>

#include <syscall.h>

uint32_t crt_argc;
uint32_t crt_argv;

int main(int argc, char **argv);

void crt_call_main()
{
    exit(main(crt_argc, (char **)crt_argv));
}
