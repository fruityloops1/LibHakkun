#pragma once

#include "hk/Result.h"
#include "hk/diag/diag.h"
#include "hk/types.h"
#include "hk/util/Allocator.h"
#include "hk/util/Span.h"

namespace hk::util {

    template <typename T, typename Allocator = DefaultAllocator>
    struct Buffer : public Span<T> {
        Buffer() = default;

        Buffer(const Buffer& other) {
            setCopy(other);
        }

        Buffer(Buffer&& old) {
            Span<T>::set(old.data(), old.size());
            old.Span<T>::set(nullptr, 0);
        }

        Buffer(const Span<T>& buffer) {
            setCopy(buffer);
        }

        ~Buffer() {
            freeBuffer();
        }

        Result allocBuffer(size size, ::size align = alignof(T)) {
            T* ptr = cast<T*>(Allocator::allocate(size * sizeof(T), align));
            HK_UNLESS(ptr != nullptr, ResultOutOfResource());

            Span<T>::set(ptr, size);
            return ResultSuccess();
        }

        Result freeBuffer() {
            if (static_cast<Span<T>*>(this)->mPtr != nullptr) {
                Allocator::free(static_cast<Span<T>*>(this)->mPtr);
                Span<T>::set(nullptr, 0);
                return ResultSuccess();
            }
            return ResultNoValue();
        }

        Result setCopy(const Span<T>& buffer) {
            return setCopy(buffer.data(), buffer.size());
        }

        Result setCopy(T* ptr, size size) {
            T* newPtr = cast<T*>(Allocator::allocate(size * sizeof(T), alignof(T)));
            HK_UNLESS(newPtr != nullptr, ResultOutOfResource());
            for (::size i = 0; i < size; i++)
                newPtr[i] = ptr[i];

            set(ptr, size);
            return ResultSuccess();
        }

    private:
        using Span<T>::set;
    };

} // namespace hk::util
