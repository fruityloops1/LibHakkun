#pragma once

#include "hk/types.h"

namespace hk::gfx {

    constexpr u32 rgba(u8 r, u8 g, u8 b, u8 a) {
        return r | g << 8 | b << 16 | a << 24;
    }

    constexpr u32 rgbaf(f32 r, f32 g, f32 b, f32 a) {
        return rgba(u8(r * 255), u8(g * 255), u8(b * 255), u8(a * 255));
    }

    constexpr u32 rgbaf32(f64 r, f64 g, f64 b, f64 a) {
        return rgba(u8(r * 255), u8(g * 255), u8(b * 255), u8(a * 255));
    }

} // namespace hk::gfx
