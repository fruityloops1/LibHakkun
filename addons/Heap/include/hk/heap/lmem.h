#pragma once

#include <hk/types.h>

namespace nn::lmem {
    namespace detail {
        struct HeapHead;
    }

    enum AllocationMode {
        AllocationMode_FirstFit,
        AllocationMode_BestFit,
    };

    using HeapHandle = detail::HeapHead*;

    HeapHandle CreateExpHeap(void* address, size_t size, int option);
    void DestroyExpHeap(HeapHandle handle);
    void* AllocateFromExpHeap(HeapHandle handle, size_t size, int alignment);
    void FreeToExpHeap(HeapHandle handle, void* ptr);
    size_t GetExpHeapAllocatableSize(HeapHandle handle, int alignment);
    size_t GetExpHeapTotalFreeSize(HeapHandle handle);
    size_t GetExpHeapBlockSize(const void* ptr);
    AllocationMode GetExpHeapAllocationMode(HeapHandle handle);
    AllocationMode SetExpHeapAllocationMode(HeapHandle handle, AllocationMode new_mode);
}
