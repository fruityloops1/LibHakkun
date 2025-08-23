#pragma once

#include "hk/Result.h"

namespace hk::hook {

    HK_RESULT_MODULE(412)
    HK_DEFINE_RESULT_RANGE(Hook, 20, 29)

    HK_DEFINE_RESULT(AlreadyInstalled, 20)
    HK_DEFINE_RESULT(NotInstalled, 21)
    HK_DEFINE_RESULT(OutOfBounds, 22)
    HK_DEFINE_RESULT(MismatchedInstruction, 23)
    HK_DEFINE_RESULT(InvalidRead, 24)
    HK_DEFINE_RESULT(NotUninstallable, 25)
    HK_DEFINE_RESULT(MapAddressNotViable, 26)
    HK_DEFINE_RESULT(AddressNotAligned, 27)

} // namespace hk::hook
