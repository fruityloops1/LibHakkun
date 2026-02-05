#pragma once

#include "hk/ValueOrResult.h"

namespace hk::hook {

    ptr findAslr(size searchSize);
    ptr findStack(size searchSize);
    ValueOrResult<ptr> mapRoToRw(ptr addr, size mapSize);

} // namespace hk::hook
