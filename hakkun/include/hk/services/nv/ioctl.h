#pragma once

#include "hk/types.h"

namespace hk::nvdrv {

    struct IoctlEncoding {
        u32 function : 8;
        u32 driverId : 8;
        u32 argumentSize : 14;
        bool read : 1;
        bool write : 1;
    };

    static_assert(sizeof(IoctlEncoding) == sizeof(u32));

    union Ioctl {
        IoctlEncoding encoding;
        u32 value;
    };

} // namespace hk::nvdrv
