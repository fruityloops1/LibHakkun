#pragma once

#include "hk/types.h"
#include <algorithm>

namespace hk::util {

    /**
     * @brief String to be used as a template argument
     *
     * @tparam N
     */
    template <size N>
    struct TemplateString {
        char value[N];

        constexpr TemplateString(const char (&str)[N]) {
            std::copy_n(str, N, value);
        }
    };

} // namespace hk::util
