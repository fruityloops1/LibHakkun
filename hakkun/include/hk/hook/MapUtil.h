#pragma once

#include "hk/types.h"

namespace hk::hook {

    ptr findAslr(size searchSize);
    Result mapRoToRw(ptr addr, size mapSize, ptr* outRw);

    Result mapRwToRx(ptr rw, size mapSize, ptr* outRx);
    Result unmapRwToRx(ptr rw, size mapSize, ptr rx);

} // namespace hk::hook
