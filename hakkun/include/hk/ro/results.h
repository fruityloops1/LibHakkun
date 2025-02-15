#pragma once

#include "hk/Result.h"

namespace hk::ro {

    HK_RESULT_MODULE(412)
    HK_DEFINE_RESULT_RANGE(Ro, 0, 9)

    HK_DEFINE_RESULT(UnusualSectionLayout, 0)
    HK_DEFINE_RESULT(GnuHashMissing, 1)
    HK_DEFINE_RESULT(AlreadyMapped, 2)
    HK_DEFINE_RESULT(NotMapped, 3)
    HK_DEFINE_RESULT(OutOfRange, 4)
    HK_DEFINE_RESULT(RtldModuleInvalid, 5)

} // namespace hk::ro
