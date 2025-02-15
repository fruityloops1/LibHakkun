#pragma once

#include "hk/types.h"
#include <algorithm>

namespace hk::util {

    template <size N>
    struct TemplateString {
        char value[N];

        constexpr TemplateString(const char (&str)[N]) {
            std::copy_n(str, N, value);
        }
    };

} // namespace hk::util
