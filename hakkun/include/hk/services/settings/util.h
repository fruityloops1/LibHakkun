#pragma once

#include "hk/services/settings/result.h" // IWYU pragma: keep
#include "hk/types.h"
#include "hk/util/StringView.h" // IWYU pragma: keep

namespace hk::settings {
    constexpr size maxKeyLen = 0x48;
#define ASSERT_KEY_LEN(KEY) HK_UNLESS((KEY).size() < maxKeyLen, ResultKeyTooLong())
} // namespace hk::settings
