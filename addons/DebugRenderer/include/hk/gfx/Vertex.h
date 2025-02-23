#pragma once

#include "hk/util/Math.h"

namespace hk::gfx {

    struct Vertex {
        util::Vector2f pos;
        util::Vector2f uv;
        u32 color;
    };

} // namespace hk::gfx
