#pragma once

#include "hk/util/Span.h"
#include <string>

namespace hk::util {

    template <typename T>
    class StringViewBase : public Span<const T> {
        static StringViewBase getEmpty() { return { &cNullChar }; }

        using Span<const T>::empty;

    public:
        using Span<const T>::data;

        constexpr StringViewBase()
            : StringViewBase(getEmpty()) { }

        constexpr StringViewBase(const T* data, size length)
            : Span<const T>(data, length) {
        }

        constexpr StringViewBase(const T* str)
            : StringViewBase(str, str != nullptr ? std::char_traits<T>::length(str) : 0) {
        }

        template <size N>
        constexpr StringViewBase(const T data[N])
            : Span<const T>(data, N) { }

        constexpr StringViewBase(const Span<const T>& data)
            : Span<const T>(data.data(), data.size()) { }

        StringViewBase(const Span<const u8>& data)
            : StringViewBase(cast<const T>(data)) { }

        constexpr ::size length() const { return Span<const T>::size(); }
        constexpr bool empty() const { return length() == 0; }

        constexpr bool operator==(StringViewBase other) const {
            return this->length() == other.length() && __builtin_memcmp(this->data(), other.data(), length()) == 0;
        }

        constexpr bool startsWith(StringViewBase start) const {
            return this->length() >= start.length() && __builtin_memcmp(this->data(), start.data(), start.length()) == 0;
        }

        constexpr StringViewBase operator+(size offs) const {
            if (offs >= length())
                return StringViewBase();

            return { data() + offs, length() - offs };
        }

        constexpr ::size findIndex(StringViewBase needle) const {
            if (needle.length() > this->length())
                return -1;

            for (size curIdx = 0; curIdx < this->length() - needle.length(); curIdx++) {
                if ((*this + curIdx).startsWith(needle))
                    return curIdx;
            }

            return -1;
        }

        static constexpr const T cNullChar = 0;
    };

    using StringView = StringViewBase<char>;
    using WideStringView = StringViewBase<wchar_t>;
    using StringView16 = StringViewBase<char16_t>;

} // namespace hk::util
