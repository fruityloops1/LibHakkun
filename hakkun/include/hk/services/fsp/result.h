#pragma once

#include "hk/Result.h"

namespace hk::fsp {

    HK_RESULT_MODULE(412)
    HK_DEFINE_RESULT_RANGE(FileSystemProxy, 100, 110)

    HK_DEFINE_RESULT(PathTooLong, 100)

} // namespace hk::fsp
