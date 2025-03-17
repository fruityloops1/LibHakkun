#pragma once

#include "hk/mem/ExpHeap.h"

namespace hk::mem {

    extern ExpHeap sMainHeap;

    __attribute__((weak)) void initializeMainHeap();

} // namespace hk::mem
