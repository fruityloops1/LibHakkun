#pragma once

#include "hk/types.h"

namespace hk::ro {

    struct ModuleHeader {
        constexpr static u32 cMod0Magic = 0x30444F4D;

        u32 magic;
        u32 dynamicOffset;
        u32 bssOffset;
        u32 bssEndOffset;
        u32 unwindStartOffset;
        u32 unwindEndOffset;
        u32 moduleOffset;

        bool isValidMagic() const { return magic == cMod0Magic; }
    };

} // namespace hk::ro
