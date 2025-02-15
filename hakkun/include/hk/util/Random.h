#pragma once

#include "hk/svc/api.h"
#include "hk/types.h"
#include <random>

namespace hk::util {

    inline u64 getRandomU64() {
        std::mt19937_64 engine { svc::getSystemTick() };
        return engine();
    }

    inline u32 getRandomU32() {
        std::mt19937 engine { u32(svc::getSystemTick() & 0xFFFFFFFF) };
        return engine();
    }

} // namespace hk::util
