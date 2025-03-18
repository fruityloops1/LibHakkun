#include "hk/mem/ExpHeap.h"
#include "ams/lmem/lmem_common.hpp"
#include "ams/lmem/lmem_exp_heap.hpp"

namespace hk::mem {

#define getHeapHandle() reinterpret_cast<::ams::lmem::HeapHandle>(mHeapHandle)

    ExpHeap::~ExpHeap() {
        destroy();
    }

    void ExpHeap::initialize(void* arena, size size) {
        destroy();
        mHeapSize = size;
        mHeapHandle = ams::lmem::CreateExpHeap(arena, size, 0);
    }

    void ExpHeap::destroy() {
        if (mHeapHandle) {
            ams::lmem::DestroyExpHeap(getHeapHandle());
            mHeapSize = 0;
            mHeapHandle = nullptr;
        }
    }

    void ExpHeap::adjust() {
        ams::lmem::AdjustExpHeap(getHeapHandle());
    }

    void* ExpHeap::allocate(size size, s32 alignment) {
        return ams::lmem::AllocateFromExpHeap(getHeapHandle(), size, alignment);
    }

    void* ExpHeap::reallocate(void* oldPtr, size size) {
        auto resizedSize = ams::lmem::ResizeExpHeapMemoryBlock(getHeapHandle(), oldPtr, size);
        if (resizedSize > 0)
            return oldPtr;

        void* newPtr = allocate(size);
        if (newPtr) {
            __builtin_memcpy(newPtr, oldPtr, size);
            free(oldPtr);
        }
        return newPtr;
    }

    void ExpHeap::free(void* ptr) {
        ams::lmem::FreeToExpHeap(getHeapHandle(), ptr);
    }

    size ExpHeap::getTotalSize() const {
        return mHeapSize;
    }

    size ExpHeap::getFreeSize() const {
        return ams::lmem::GetExpHeapTotalFreeSize(getHeapHandle());
    }

    size ExpHeap::getAllocatableSize() const {
        return ams::lmem::GetExpHeapAllocatableSize(getHeapHandle(), 4);
    }

    void ExpHeap::setAllocationMode(AllocationMode mode) {
        ams::lmem::SetExpHeapAllocationMode(getHeapHandle(), static_cast<::ams::lmem::AllocationMode>(mode));
    }

    AllocationMode ExpHeap::getAllocationMode() const {
        return static_cast<AllocationMode>(ams::lmem::GetExpHeapAllocationMode(getHeapHandle()));
    }

    void ExpHeap::setGroupId(u16 groupId) {
        ams::lmem::SetExpHeapGroupId(getHeapHandle(), groupId);
    }

    u16 ExpHeap::getGroupId() const {
        return ams::lmem::GetExpHeapGroupId(getHeapHandle());
    }

    void ExpHeap::forEachAllocation(HeapVisitor callback, void* userData) {
        struct UserData {
            HeapVisitor callback;
            ExpHeap& heap;
            void* userData;
        } data = { callback, *this, userData };

        constexpr auto visit = [](void* block, ams::lmem::HeapHandle handle, uintptr_t user_data) -> void {
            const UserData* data = reinterpret_cast<const UserData*>(user_data);
            data->callback(block, data->heap, data->userData);
        };

        ams::lmem::VisitExpHeapAllocatedBlocks(getHeapHandle(), visit, uintptr_t(&data));
    }

    size ExpHeap::getAllocationSize(void* ptr) {
        return ams::lmem::GetExpHeapMemoryBlockSize(ptr);
    }

    u16 ExpHeap::getAllocationGroupId(void* ptr) {
        return ams::lmem::GetExpHeapMemoryBlockGroupId(ptr);
    }

    AllocationDirection ExpHeap::getAllocationDirection(void* ptr) {
        return static_cast<AllocationDirection>(ams::lmem::GetExpHeapMemoryBlockAllocationDirection(ptr));
    }

} // namespace hk::mem
