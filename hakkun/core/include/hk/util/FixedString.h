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
        constexpr FixedStringBase() {
            mData[0] = '\0';
        };

        constexpr FixedStringBase(const T* data)
            : mLength(std::min(std::char_traits<T>::length(data), Capacity - 1)) {
            std::copy(data, data + mLength, mData.begin());
        }

        template <size N>
        constexpr FixedStringBase(const T data[N])
            : mLength(std::min(N - 1, Capacity - 1)) {
            std::copy(data, data + mLength, mData.begin());
        }

        constexpr FixedStringBase(const StringViewBase<T> src) {
            append(src);
        }

        constexpr void truncate(size newSize) {
            HK_ABORT_UNLESS(newSize <= mLength, "hk::util::FixedStringBase<%s>::truncate(%zu): new size is too high (size: %zu)", Capacity, newSize, mLength);
            mLength = newSize;
            mData[mLength] = T('\0');
        }

        constexpr bool append(const StringViewBase<T> other) {
            auto desiredSize = mLength + other.size();
            auto newSize = std::min(desiredSize, Capacity - 1);
            std::copy(other.begin(), other.end(), end());
            mData[newSize] = T('\0');
            mLength = newSize;
            return desiredSize == newSize;
        }

        constexpr bool append(T value) {
            if (mLength == Capacity - 1)
                return false;

            mData[mLength++] = value;
            mData[mLength] = T('\0');
            return true;
        }

        constexpr FixedStringBase operator+(const StringViewBase<T> other) {
            FixedStringBase newString = *this;
            newString.append(other);
            return newString;
        }

        constexpr T* data() { return mData.data(); }
        constexpr const T* data() const { return mData; }
        constexpr size size_bytes() const { return mLength * sizeof(T); }
        constexpr size size() const { return mLength; }
        constexpr ::size length() const { return mLength; }
        constexpr static ::size capacity() { return Capacity; }

        constexpr T& operator[](::size index) {
            HK_ABORT_UNLESS(index < mLength, "hk::util::FixedStringBase<%s, %s>::operator[%zu]: out of range (size: %zu)", getTypeName<T>(), Capacity, index, mLength);
            return mData[index];
        }

        constexpr const T& operator[](::size index) const {
            HK_ABORT_UNLESS(index < mLength, "hk::util::FixedStringBase<%s, %s>::operator[%zu]: out of range (size: %zu)", getTypeName<T>(), Capacity, index, mLength);
            return mData[index];
        }

        constexpr T* begin() { return &mData[0]; }
        constexpr T* end() { return &mData[mLength]; }
        constexpr const T* begin() const { return &mData[0]; }
        constexpr const T* end() const { return &mData[mLength]; }

        constexpr operator StringViewBase<T>() const { return Span<const T> { data(), size() }; }
        constexpr operator Span<const T>() const { return Span<const T> { data(), size() }; }
    };

    template <size Capacity>
    using FixedString = FixedStringBase<char, Capacity>;
    template <size Capacity>
    using WideFixedString = FixedStringBase<wchar_t, Capacity>;
    template <size Capacity>
    using FixedString16 = FixedStringBase<char16_t, Capacity>;

} // namespace hk::util
