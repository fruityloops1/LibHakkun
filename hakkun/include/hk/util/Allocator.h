#pragma once

#include "hk/types.h"
#include <cstdlib>
#include <new>
#include <type_traits>

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
            return aligned_alloc(alignment, size);
        }

        hk_alwaysinline inline static void* reallocate(void* ptr, ::size size) {
            return realloc(ptr, size);
        }

        hk_alwaysinline inline static void free(void* ptr) {
            free(ptr);
        }
    };

    template <class T>
    concept AllocatorType = std::is_base_of_v<Allocator<T>, T>;

} // namespace hk::util
