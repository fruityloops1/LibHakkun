#pragma once

#include "hk/Result.h"
#include "hk/diag/diag.h"
#include "hk/types.h"
#include "hk/util/Allocator.h"

namespace hk::util {

    template <typename T>
    class Span {
    protected:
        T* mPtr = nullptr;
        size mSize = 0;

    public:
        Span() = default;
        Span(T* ptr, size size)
            : mPtr(ptr)
            , mSize(size) { }

        void set(T* ptr, size size) {
            mPtr = ptr;
            mSize = size;
        }

        T* data() { return mPtr; }
        const T* data() const { return mPtr; }
        size size_bytes() const { return mSize * sizeof(T); }
        size size() const { return mSize; }
        bool empty() const { return mSize == 0; }

        T& operator[](::size index) {
            HK_ABORT_UNLESS(index < mSize, "hk::util::Span<%s>::operator[%zu]: out of range (size: %zu)", index, mSize);
            return mPtr[index];
        }

        const T& operator[](::size index) const {
            HK_ABORT_UNLESS(index < mSize, "hk::util::Span<%s>::operator[%zu]: out of range (size: %zu)", index, mSize);
            return mPtr[index];
        }

        T* begin() { return &mPtr[0]; }
        T* end() { return &mPtr[mSize]; }
        const T* begin() const { return &mPtr[0]; }
        const T* end() const { return &mPtr[mSize]; }
    };

} // namespace hk::util
