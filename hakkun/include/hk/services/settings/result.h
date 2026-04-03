#pragma once

#include "hk/Result.h"

namespace hk::settings {

    HK_RESULT_MODULE(412)
    HK_DEFINE_RESULT_RANGE(Settings, 110, 120)

    HK_DEFINE_RESULT(KeyTooLong, 110)
    HK_DEFINE_RESULT(ValueTooLarge, 111)
    HK_DEFINE_RESULT(ValueIsWrongSize, 112)

} // namespace hk::fsp
