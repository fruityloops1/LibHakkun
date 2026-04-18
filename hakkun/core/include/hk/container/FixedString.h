#pragma once

#include "hk/container/Array.h"
#include "hk/container/Span.h"
#include "hk/container/StringView.h"
#include "hk/prim/traits/Function.h"
#include "hk/types.h"
#include <algorithm>
#include <cstdarg>
#include <format>

namespace hk {

    // TODO: StringOperations
    template <typename T, size Capacity>
    class FixedStringBase {
        size mLength = 0; // always less than Capacity, excludes null terminator
        Array<T, Capacity> mData;

        using MutableStringView = MutableStringViewBase<T>;
        using StringView = StringViewBase<T>;

        constexpr hk_alwaysinline MutableStringView toMutableView() {
            return MutableStringView({ mData.data(), Capacity }, mLength);
        }

        template <typename L>
        constexpr hk_alwaysinline typename util::FunctionTraits<L>::ReturnType withMutableView(L func) {
            using Return = typename util::FunctionTraits<L>::ReturnType;

            MutableStringView view = toMutableView();
            defer { mLength = view.length(); };
            return func(view);
        }

    public:
        constexpr FixedStringBase() = default;

        constexpr FixedStringBase(const T* data)
            : mLength(std::min(std::char_traits<T>::length(data), Capacity - 1)) {
            std::copy(data, data + mLength, mData);
        }

        constexpr FixedStringBase(const FixedStringBase& other)
            : mLength(other.mLength) {
            mData.setCopy(other);
        }

        template <size N>
        constexpr FixedStringBase(const T data[N])
            : mLength(std::min(N - 1, Capacity - 1)) {
            std::copy(data, data + mLength, mData);
        }

        constexpr FixedStringBase(const StringView src) {
            append(src);
        }

        constexpr void truncate(size newSize) {
            withMutableView([&](MutableStringView& view) { view.truncate(newSize); });
        }
        constexpr bool append(const StringView other) {
            return withMutableView([&](MutableStringView& view) -> bool { return view.append(other); });
        }
        constexpr bool append(T value) {
            return withMutableView([&](MutableStringView& view) -> bool { return view.append(value); });
        }

        constexpr FixedStringBase operator+(const StringView other) const {
            FixedStringBase newString = *this;
            newString.append(other);
            return newString;
        }

        constexpr FixedStringBase& operator+=(const StringView other) {
            append(other);
            return *this;
        }

        constexpr T* data() { return mData.data(); }
        constexpr const T* data() const { return mData.data(); }
        constexpr size size_bytes() const { return mLength * sizeof(T); }
        constexpr size size() const { return mLength; }
        constexpr ::size length() const { return mLength; }
        constexpr static ::size capacity() { return Capacity; }

        constexpr T& operator[](::size index) { return StringView(*this)[index]; }
        constexpr const T& operator[](::size index) const { return StringView(*this)[index]; }

        using Iterator = MutableStringView::Iterator;
        using ConstIterator = StringView::ConstIterator;

        constexpr Iterator begin() { return toMutableView().begin(); }
        constexpr Iterator end() { return toMutableView().end(); }
        constexpr ConstIterator begin() const { return StringView(*this).begin(); }
        constexpr ConstIterator end() const { return StringView(*this).end(); }

        constexpr operator StringView() const { return Span<const T> { data(), size() }; }
        constexpr operator MutableStringView() { return toMutableView(); }
        constexpr operator Span<const T>() const { return Span<const T> { data(), size() }; }

        constexpr operator T*() { return data(); }
        constexpr operator const T*() const { return data(); }
        constexpr const T* cstr() const { return data(); }
    };

    template <size Capacity>
    using FixedString = FixedStringBase<char, Capacity>;
    template <size Capacity>
    using WideFixedString = FixedStringBase<wchar_t, Capacity>;
    template <size Capacity>
    using FixedString16 = FixedStringBase<char16_t, Capacity>;

} // namespace hk
