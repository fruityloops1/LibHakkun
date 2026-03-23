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
        size mLength = 0; // always less than Capacity, excludes null terminator
        Array<T, Capacity> mData;

    public:
        FixedStringBase() {
            mData[0] = '\0';
        };

        FixedStringBase(const T* data)
            : mLength(std::min(std::char_traits<T>::length(data), Capacity - 1)) {
            std::copy(data, data + mLength, mData.begin());
        }

        template <size N>
        FixedStringBase(const T data[N])
            : mLength(std::min(N - 1, Capacity - 1)) {
            std::copy(data, data + mLength, mData.begin());
        }

        FixedStringBase(const StringViewBase<T> src) {
            append(src);
        }

        template <typename... Args>
        FixedStringBase(std::format_string<Args...> fmt, Args&&... args) {
            auto result = std::format_to_n(mData.begin(), Capacity, fmt, forward<Args>(args)...);
            mLength = std::min<::size>(result.size, Capacity - 1);
            mData[mLength] = '\0';
        }

        void truncate(size newSize) {
            HK_ABORT_UNLESS(newSize <= mLength, "hk::util::FixedStringBase<%s>::truncate(%zu): new size is too high (size: %zu)", Capacity, newSize, mLength);
            mLength = newSize;
            mData[mLength] = '\0';
        }

        void append(const StringViewBase<T> other) {
            auto newSize = std::min(mLength + other.size(), Capacity - 1);
            std::copy(other.begin(), other.end(), end());
            mData[newSize] = '\0';
            mLength = newSize;
        }

        FixedStringBase operator+(const StringViewBase<T> other) {
            FixedStringBase newString = *this;
            newString.append(other);
            return newString;
        }

        T* data() { return mData.data(); }
        const T* data() const { return mData; }
        size size_bytes() const { return mLength * sizeof(T); }
        size size() const { return mLength; }
        ::size length() const { return mLength; }
        static ::size capacity() { return Capacity; }

        T& operator[](::size index) {
            HK_ABORT_UNLESS(index < mLength, "hk::util::FixedStringBase<%s, %s>::operator[%zu]: out of range (size: %zu)", getTypeName<T>(), Capacity, index, mLength);
            return mData[index];
        }

        const T& operator[](::size index) const {
            HK_ABORT_UNLESS(index < mLength, "hk::util::FixedStringBase<%s, %s>::operator[%zu]: out of range (size: %zu)", getTypeName<T>(), Capacity, index, mLength);
            return mData[index];
        }

        T* begin() { return &mData[0]; }
        T* end() { return &mData[mLength]; }
        const T* begin() const { return &mData[0]; }
        const T* end() const { return &mData[mLength]; }

        operator StringViewBase<T>() const { return Span<const T> { data(), size() }; }
        operator Span<const T>() const { return Span<const T> { data(), size() }; }
    };

    template <size Capacity>
    using FixedString = FixedStringBase<char, Capacity>;
    template <size Capacity>
    using WideFixedString = FixedStringBase<wchar_t, Capacity>;
    template <size Capacity>
    using FixedString16 = FixedStringBase<char16_t, Capacity>;

} // namespace hk::util
