#include <heap.h>
#include <panic.h>
#include <stddef.h>
#include <align_util.h>

#define MIN_NODE_SIZE (sizeof(HeapNode_t) + 4)

HANDLE hKernelHeap = 0;

#define AVAIL_MAGIC 0xAA55AA55

typedef struct HeapNode
{
    uintptr_t size;
    uintptr_t free;
    char payload[0];
    struct HeapNode *next;
} HeapNode_t;

typedef struct HeapInfo
{
    uintptr_t heapSize;
    HeapNode_t *fistAvail;
} HeapInfo_t;

HANDLE HeapInitialize(uintptr_t size, void *heapMem)
{
    // TODO: assert mininum size

    HeapInfo_t *info = heapMem;
    HeapNode_t *first = (HeapNode_t *)(info + 1);

    first = (void *)ALIGN_CEIL(first, 8);

    info->heapSize = size;
    info->fistAvail = first;

    first->size = heapMem + size - (void *)first;
    first->free = AVAIL_MAGIC;
    first->next = 0;
    return heapMem;
}

void *HeapAlloc(HANDLE hHeap, uintptr_t size)
{
    if (hHeap == NULL)
        hHeap = hKernelHeap;

    HeapInfo_t *heap = hHeap;
    size = ALIGN_CEIL(size, 8) + offsetof(HeapNode_t, payload);

    for (HeapNode_t *now = heap->fistAvail, *prev = NULL; now; prev = now, now = now->next)
    {
        if (now->free != AVAIL_MAGIC)
            panic("Unexpected magic at heapalloc");

        if (now->size >= size)
        {
            HeapNode_t *next;
            if (now->size - size >= MIN_NODE_SIZE) // Split
            {
                HeapNode_t *newnode = (void *)now + size;
                newnode->size = now->size - size;
                newnode->next = now->next;
                newnode->free = AVAIL_MAGIC;

                now->size = size;

                next = newnode;
            }
            else
            {
                next = now->next;
            }
            if (prev)
            {
                prev->next = next;
            }
            else
            {
                heap->fistAvail = next;
            }
            now->free = (uintptr_t)hHeap;
            return now->payload;
        }
    }

    if (hHeap == hKernelHeap)
        panic("Kernel ran out of heap memory");
    
    return NULL;
}

void HeapFree(void *mem)
{
    HeapNode_t *node = mem - offsetof(HeapNode_t, payload);

    if (node->free == AVAIL_MAGIC)
        panic("Double free");

    HeapInfo_t *heap = (HeapInfo_t *)(node->free);
    node->free = AVAIL_MAGIC;

    for (HeapNode_t *now = heap->fistAvail, *prev = NULL, **ppnow = &heap->fistAvail;
         now || ppnow;
         ppnow = now ? &now->next : NULL, prev = now, now = now ? now->next : NULL)
    {
        if (!now || node < now)
        {
            node->next = now;

            if (prev)
            {
                prev->next = node;

                if ((void *)prev + prev->size == node)
                {
                    // Merge prev
                    prev->size += node->size;
                    prev->next = node->next;
                    node = prev;
                }
            }
            else
            {
                heap->fistAvail = node;
            }

            // Normally it's true -> now > node -> now is not NULL
            if ((void *)node + node->size == now)
            {
                node->next = now->next;
                node->size += now->size;
            }

            return;
        }
    }
}

HANDLE getKernelHeap()
{
    return hKernelHeap;
}

HANDLE setKernelHeap(HANDLE heap)
{
    HANDLE ret = hKernelHeap;
    hKernelHeap = heap;
    return ret;
}
