#ifndef _HEAP_H

#define _HEAP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

    typedef void *HANDLE;

    HANDLE HeapInitialize(uintptr_t size, void *virtualMem);

    // If heap == NULL, use default kernel heap
    void *HeapAlloc(HANDLE heap, uintptr_t size);
    void HeapFree(void *mem);

    HANDLE getKernelHeap();
    HANDLE setKernelHeap(HANDLE);

#ifdef __cplusplus
}
#endif

#endif
