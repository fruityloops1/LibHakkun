#pragma once

#include "hk/types.h"
#include <cstdlib>
#include <new>
#include <type_traits>

#ifdef HK_ADDON_Sead
#include "hk/ValueOrResult.h"
#include <sead/heap/seadHeapMgr.h>
#endif

namespace hk::util {

    template <typename Impl>
    struct Allocator {
        static void* operator new(std::size_t size, std::align_val_t align) {
            return Impl::allocate(size, align);
        }

        static void* operator new(std::size_t size, std::align_val_t align, const std::nothrow_t& t) noexcept {
            return Impl::allocate(size, align);
        }

        static void* operator new[](std::size_t size, std::align_val_t align) {
            return Impl::allocate(size, align);
        }

        static void* operator new[](std::size_t size, std::align_val_t align, const std::nothrow_t& t) noexcept {
            return Impl::allocate(size, align);
        }

        static void operator delete(void* ptr) {
            return Impl::free(ptr);
        }

        static void operator delete(void* ptr, const std::nothrow_t& t) {
            return Impl::free(ptr);
        }

        static void operator delete(void* ptr, std::size_t size) {
            return Impl::free(ptr);
        }

        static void operator delete[](void* ptr) {
            return Impl::free(ptr);
        }

        static void operator delete[](void* ptr, const std::nothrow_t& t) {
            return Impl::free(ptr);
        }

        static void operator delete[](void* ptr, std::size_t size) {
            return Impl::free(ptr);
        }
    };

    struct MallocAllocator : Allocator<MallocAllocator> {
        hk_alwaysinline inline static void* allocate(::size size, ::size alignment) {
            return ::aligned_alloc(alignment, size);
        }

        hk_alwaysinline inline static void* reallocate(void* ptr, ::size size) {
            return ::realloc(ptr, size);
        }

        hk_alwaysinline inline static void free(void* ptr) {
            ::free(ptr);
        }
    };

#ifdef HK_ADDON_Sead
    struct SeadAllocator : Allocator<SeadAllocator> {
        hk_noinline static void* allocate(::size size, ::size alignment) {
            sead::Heap* heap = HK_UNWRAP(HK_UNWRAP(sead::HeapMgr::instance())->getCurrentHeap());
            return heap->alloc(size, alignment);
        }

        hk_noinline static void* reallocate(void* ptr, ::size size) {
            sead::Heap* heap = HK_UNWRAP(HK_UNWRAP(sead::HeapMgr::instance())->getCurrentHeap());
            return heap->tryRealloc(ptr, size, sizeof(size));
        }

        hk_noinline static void free(void* ptr) {
            sead::Heap* heap = HK_UNWRAP(HK_UNWRAP(sead::HeapMgr::instance())->getCurrentHeap());
            heap->free(ptr);
        }
    };

    using DefaultAllocator = SeadAllocator;
#else
    using DefaultAllocator = MallocAllocator;
#endif

    template <class T>
    concept AllocatorType = std::is_base_of_v<Allocator<T>, T>;

} // namespace hk::util
