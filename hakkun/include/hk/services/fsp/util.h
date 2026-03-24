#pragma once

#include "hk/services/fsp/result.h" // IWYU pragma: keep
#include "hk/types.h"
#include <string_view> // IWYU pragma: keep

namespace hk::fsp {
    constexpr size maxPathSize = 0x301;
#define ASSERT_PATH_LEN(PATH) HK_UNLESS((PATH).size() < maxPathSize, ResultPathTooLong())
} // namespace hk::fsp
