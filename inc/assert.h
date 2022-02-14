#ifndef _assert_h

#define _assert_h

#include <panic.h>

#define assert(c) do { if (!(c)) panic("Assertion failed: " #c); } while(0)

#endif
