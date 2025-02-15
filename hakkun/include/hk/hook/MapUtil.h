#pragma once

#include "hk/types.h"

namespace hk::hook {

    ptr findAslr(size searchSize);
    Result mapRoToRw(ptr addr, size mapSize, ptr* outRw);

} // namespace hk::hook
