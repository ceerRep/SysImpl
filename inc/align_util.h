#ifndef _ALIGN_UTIL

#define _ALIGN_UTIL

#define ALIGN_CEIL(p, a) (((uintptr_t)(p) + (a)-1) / (a) * (a))
#define ALIGN_FLOOR(p, a) (((uintptr_t)(p)) / (a) * (a))

#endif
