#pragma once

#include "hk/util/Span.h"
#include <string>

namespace hk::util {

    template <typename T>
    class StringViewBase : public Span<const T> {
        static constexpr const T cNullChar = 0;

        static StringViewBase getEmpty() { return { &cNullChar }; }

        using Span<const T>::empty;

    public:
        using Span<const T>::data;

        StringViewBase()
            : StringViewBase(getEmpty()) { }

        StringViewBase(const T* data, size length)
            : Span<const T>(data, length) {
        }

        StringViewBase(const T* data)
            : StringViewBase(data, std::char_traits<T>::length(data)) {
        }

        template <size N>
        StringViewBase(const T data[N])
            : Span<const T>(data, N) { }

        StringViewBase(const Span<const T>& data)
            : Span<const T>(data.data(), std::char_traits<T>::length(data.data())) { }

        ::size length() const { return Span<const T>::size(); }
        bool empty() const { return length() == 0; }

        bool operator==(StringViewBase other) const {
            return this->length() == other.length() && __builtin_memcmp(this->data(), other.data(), length()) == 0;
        }

        bool startsWith(StringViewBase start) const {
            return this->length() >= start.length() && __builtin_memcmp(this->data(), start.data(), start.length()) == 0;
        }

        StringViewBase operator+(size offs) const {
            if (offs >= length())
                return StringViewBase();

            return { data() + offs, length() - offs };
        }

        ::size findIndex(StringViewBase needle) const {
            if (needle.length() > this->length())
                return -1;

            for (size curIdx = 0; curIdx < this->length() - needle.length(); curIdx++) {
                if ((*this + curIdx).startsWith(needle))
                    return curIdx;
            }

            return -1;
        }
    };

    using StringView = StringViewBase<char>;
    using WideStringView = StringViewBase<wchar_t>;
    using StringView16 = StringViewBase<char16_t>;

} // namespace hk::util
