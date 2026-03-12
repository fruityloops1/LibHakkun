#pragma once

#include "hk/util/Span.h"
#include <string>

namespace hk::util {

    // light
    template <typename T>
    class StringViewBase : public Span<const T> {
    public:
        StringViewBase(const T* data)
            : Span<T>(data, std::char_traits<T>::length(data)) {
            size index = 0;
        }

        template <size N>
        StringViewBase(const T data[N])
            : Span<T>(data, N) { }

        const T* cstr() const { return Span<T>::data(); }
    };

    using StringView = StringViewBase<char>;

}
