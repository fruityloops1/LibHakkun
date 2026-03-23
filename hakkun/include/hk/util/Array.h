#pragma once

#include "hk/types.h"
#include "hk/util/Span.h"

namespace hk::util {

    template <typename T, size Length>
    class Array {
        T mItems[Length] = {};

    public:
        constexpr Array() = default;

        constexpr Array(const T values[Length])
            : mItems(values) { }

        constexpr Array(std::initializer_list<T> list)
            : mItems(list) { }

        T* data() { return mItems; }
        const T* data() const { return mItems; }
        size size_bytes() const { return Length * sizeof(T); }
        size size() const { return Length; }

        T& operator[](::size index) {
            HK_ABORT_UNLESS(index < Length, "hk::util::Array<%s>::operator[%zu]: out of range (size: %zu)", index, Length);
            return mItems[index];
        }

        const T& operator[](::size index) const {
            HK_ABORT_UNLESS(index < Length, "hk::util::Array<%s>::operator[%zu]: out of range (size: %zu)", index, Length);
            return mItems[index];
        }

        T* begin() { return &mItems[0]; }
        T* end() { return &mItems[Length]; }
        const T* begin() const { return &mItems[0]; }
        const T* end() const { return &mItems[Length]; }

        operator Span<T>() const { return { data(), size() }; }
        operator Span<const T>() const { return { data(), size() }; }
    };

}
