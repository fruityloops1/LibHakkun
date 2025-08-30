#pragma once

#include "hk/types.h"

namespace hk::hook {

    ptr findAslr(size searchSize);
    ptr findStack(size searchSize);
    Result mapRoToRw(ptr addr, size mapSize, ptr* outRw);

} // namespace hk::hook
