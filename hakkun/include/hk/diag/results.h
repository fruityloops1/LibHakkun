#pragma once

#include "hk/Result.h"

namespace hk::diag {

    HK_RESULT_MODULE(412)
    HK_DEFINE_RESULT_RANGE(Diag, 10, 19)

    HK_DEFINE_RESULT(AssertionFailure, 10)
    HK_DEFINE_RESULT(Abort, 11)
    HK_DEFINE_RESULT(NotAnNnsdkThread, 19)

} // namespace hk::ro
