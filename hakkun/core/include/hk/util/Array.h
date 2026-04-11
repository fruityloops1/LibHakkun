#pragma once

#include "hk/types.h"
#include "hk/util/Span.h"

namespace hk::util {

    template <typename T, size Length>
    class Array {
        T mItems[Length] = { };

    public:
        static constexpr size cLength = Length;

        constexpr Array() = default;

        constexpr Array(const T values[Length])
            : mItems(values) { }

        constexpr Array(std::initializer_list<T> list)
            : mItems(list) { }

        constexpr T* data() { return mItems; }
        constexpr const T* data() const { return mItems; }
        constexpr size size_bytes() const { return Length * sizeof(T); }
        constexpr size size() const { return Length; }

        constexpr T& operator[](::size index) {
            HK_ABORT_UNLESS(index < Length, "hk::util::Array<%s>::operator[%zu]: out of range (size: %zu)", getTypeName<T>(), index, Length);
            return mItems[index];
        }

        constexpr const T& operator[](::size index) const {
            HK_ABORT_UNLESS(index < Length, "hk::util::Array<%s>::operator[%zu]: out of range (size: %zu)", getTypeName<T>(), index, Length);
            return mItems[index];
        }

        constexpr T* begin() { return &mItems[0]; }
        constexpr T* end() { return &mItems[Length]; }
        constexpr const T* begin() const { return &mItems[0]; }
        constexpr const T* end() const { return &mItems[Length]; }

        constexpr operator Span<T>() { return { data(), size() }; }
        constexpr operator Span<const T>() const { return { data(), size() }; }
    };

} // namespace hk::util
