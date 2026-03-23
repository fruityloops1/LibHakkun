#pragma once

#include "hk/util/Span.h"
#include <string>

namespace hk::util {

    template <typename T>
    class StringViewBase : public Span<const T> {
    public:
        StringViewBase(const T* data)
            : Span<const T>(data, std::char_traits<T>::length(data)) {
            size index = 0;
        }

        template <size N>
        StringViewBase(const T data[N])
            : Span<const T>(data, N) { }

        StringViewBase(const Span<const T>& span)
            : Span<const T>(span.data(), std::char_traits<T>::length(span.data())) { }
    };

    using StringView = StringViewBase<char>;
    using WideStringView = StringViewBase<wchar_t>;
    using StringView16 = StringViewBase<wchar_t>;

}
