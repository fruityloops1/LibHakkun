#pragma once

#include "hk/types.h"
#include <algorithm>

namespace hk::util {

    /**
     * @brief String to be used as a template argument
     *
     * @tparam N
     */
    template <size N, typename Char = char>
    struct TemplateString {
        using CharType = Char;
        constexpr static size length = N;
        Char value[N];

        constexpr TemplateString(const Char (&str)[N]) {
            std::copy_n(str, N, value);
        }

        constexpr TemplateString(const std::array<char, N>& str) {
            std::copy_n(str.data(), N, value);
        }
    };

} // namespace hk::util
