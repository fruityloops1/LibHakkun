#pragma once

#include "hk/types.h"
#include "hk/util/Array.h"
#include "hk/util/Span.h"
#include "hk/util/StringView.h"
#include <algorithm>
#include <cstdarg>
#include <format>

namespace hk::util {

    template <typename T, size Capacity>
    class FixedStringBase {
        size mSize = 0; // always less than Capacity, excludes null terminator
        Array<T, Capacity> mArray;

    public:
        FixedStringBase() {
            mArray[0] = '\0';
        };

        FixedStringBase(const T* data)
            : mSize(std::min(std::char_traits<T>::length(data), Capacity - 1)) {
            std::copy(data, data + mSize, mArray.begin());
        }

        template <size N>
        FixedStringBase(const T data[N])
            : mSize(std::min(N - 1, Capacity - 1)) {
            std::copy(data, data + mSize, mArray.begin());
        }

        FixedStringBase(const StringViewBase<T> src) {
            append(src);
        }

        template <typename... Args>
        FixedStringBase(std::format_string<Args...> fmt, Args&&... args) {
            auto result = std::format_to_n(mArray.begin(), Capacity, fmt, forward<Args>(args)...);
            mSize = std::min<::size>(result.size, Capacity - 1);
            mArray[mSize] = '\0';
        }

        void truncate(size newSize) {
            HK_ABORT_UNLESS(newSize <= mSize, "hk::util::FixedStringBase<%s>::truncate(%zu): new size is too high (size: %zu)", Capacity, newSize, mSize);
            mSize = newSize;
            mArray[mSize] = '\0';
        }

        void append(const StringViewBase<T> other) {
            auto newSize = std::min(mSize + other.size(), Capacity - 1);
            std::copy(other.begin(), other.end(), end());
            mArray[newSize] = '\0';
            mSize = newSize;
        }

        FixedStringBase operator+(const StringViewBase<T> other) {
            FixedStringBase newString = *this;
            newString.append(other);
            return newString;
        }

        T* data() { return mArray.data(); }
        const T* data() const { return mArray; }
        size size_bytes() const { return mSize * sizeof(T); }
        size size() const { return mSize; }

        T& operator[](::size index) {
            HK_ABORT_UNLESS(index < mSize, "hk::util::FixedStringBase<%s>::operator[%zu]: out of range (size: %zu)", Capacity, index, mSize);
            return mArray[index];
        }

        const T& operator[](::size index) const {
            HK_ABORT_UNLESS(index < mSize, "hk::util::FixedStringBase<%s>::operator[%zu]: out of range (size: %zu)", Capacity, index, mSize);
            return mArray[index];
        }

        T* begin() { return &mArray[0]; }
        T* end() { return &mArray[mSize]; }
        const T* begin() const { return &mArray[0]; }
        const T* end() const { return &mArray[mSize]; }

        operator StringViewBase<T>() const { return Span<const T> { data(), size() }; }
    };

    template <size Capacity>
    using FixedString = FixedStringBase<char, Capacity>;
    template <size Capacity>
    using WideFixedString = FixedStringBase<wchar_t, Capacity>;
    template <size Capacity>
    using FixedString16 = FixedStringBase<char16_t, Capacity>;
}
