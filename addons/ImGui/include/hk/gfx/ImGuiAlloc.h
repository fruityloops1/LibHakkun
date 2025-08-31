#pragma once

#include "hk/types.h"

namespace hk::gfx {
    struct Allocator {
        using Alloc = void* (*)(size, size);
        using Free = void (*)(void*);

        Alloc alloc = nullptr;
        Free free = nullptr;
    };
}