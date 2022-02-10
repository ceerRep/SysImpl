#ifndef _assert_h

#define _assert_h

#include <panic.h>

#define assert(c) do { if (!(c)) panic(#c); } while(0)

#endif
