#pragma once

#include "hk/types.h"

namespace hk::mem {

    enum AllocationMode
    {
        AllocationMode_FirstFit,
        AllocationMode_BestFit,
    };

    enum AllocationDirection
    {
        AllocationDirection_Front,
        AllocationDirection_Back,
    };

    class ExpHeap {
        void* mHeapHandle = nullptr;
        size mHeapSize = 0;

    public:
        using HeapVisitor = void (*)(void* ptr, ExpHeap& heap, void* userData);

        NON_COPYABLE(ExpHeap);
        NON_MOVABLE(ExpHeap);

        ExpHeap() = default;
        ~ExpHeap();

        void initialize(void* arena, size size);
        void destroy();
        void adjust();

        void* allocate(size size, s32 alignment = 4);
        void* reallocate(void* ptr, size size);
        void free(void* ptr);

        size getTotalSize() const { return mHeapSize; }
        size getFreeSize() const;
        size getAllocatableSize() const;

        void setAllocationMode(AllocationMode mode);
        AllocationMode getAllocationMode() const;

        void setGroupId(u16 groupId);
        u16 getGroupId() const;

        void forEachAllocation(HeapVisitor callback, void* userData = nullptr);

        static size getAllocationSize(void* ptr);
        static u16 getAllocationGroupId(void* ptr);
        static AllocationDirection getAllocationDirection(void* ptr);
    };

} // namespace hk::mem
