#pragma once

#include "hk/Result.h"
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

        Buffer(const Span<const T>& buffer) {
            setCopy(buffer);
        }

        ~Buffer() {
            freeBuffer();
        }

        Result allocBuffer(size size, const T& value = T(), ::size align = alignof(T)) {
            T* ptr = cast<T*>(Allocator::allocate(size * sizeof(T), align));
            HK_UNLESS(ptr != nullptr, ResultOutOfResource());

            Span<T>::set(ptr, size);
            for (::size i = 0; i < mSize; i++)
                new (&mData[i]) T(value);

            return ResultSuccess();
        }

        Result freeBuffer() {
            if (mData != nullptr) {
                for (size i = 0; i < mSize; i++)
                    mData[i].~T();

                Allocator::free(mData);
                Span<T>::set(nullptr, 0);
                return ResultSuccess();
            }
            return ResultNoValue();
        }

        Result setCopy(const Span<const T>& buffer) {
            return setCopy(buffer.data(), buffer.size());
        }

        Result setCopy(const T* ptr, size size) {
            T* newPtr = cast<T*>(Allocator::allocate(size * sizeof(T), alignof(T)));
            HK_UNLESS(newPtr != nullptr, ResultOutOfResource());
            for (::size i = 0; i < size; i++)
                newPtr[i] = ptr[i];

            set(newPtr, size);
            return ResultSuccess();
        }

    private:
        using Span<T>::set;
        using Span<T>::mData;
        using Span<T>::mSize;
    };

} // namespace hk::util
