#pragma once

#include "hk/diag/diag.h"
#include "hk/gfx/Texture.h"
#include "hk/util/Math.h"
#include "nvn/nvn_Cpp.h"

namespace hk::gfx {

    inline nvn::Format getAstcFormat(const void* tex) {
        const AstcHeader* header = reinterpret_cast<const AstcHeader*>(tex);
        nvn::Format format = nvn::Format::NONE;
        if (header->block_x == 4 && header->block_y == 4)
            format = nvn::Format::RGBA_ASTC_4x4_SRGB;
        else if (header->block_x == 5 && header->block_y == 4)
            format = nvn::Format::RGBA_ASTC_5x4_SRGB;
        else if (header->block_x == 5 && header->block_y == 5)
            format = nvn::Format::RGBA_ASTC_5x5_SRGB;
        else if (header->block_x == 6 && header->block_y == 5)
            format = nvn::Format::RGBA_ASTC_6x5_SRGB;
        else if (header->block_x == 6 && header->block_y == 6)
            format = nvn::Format::RGBA_ASTC_6x6_SRGB;
        else if (header->block_x == 8 && header->block_y == 5)
            format = nvn::Format::RGBA_ASTC_8x5_SRGB;
        else if (header->block_x == 8 && header->block_y == 6)
            format = nvn::Format::RGBA_ASTC_8x6_SRGB;
        else if (header->block_x == 8 && header->block_y == 8)
            format = nvn::Format::RGBA_ASTC_8x8_SRGB;
        else if (header->block_x == 10 && header->block_y == 5)
            format = nvn::Format::RGBA_ASTC_10x5_SRGB;
        else if (header->block_x == 10 && header->block_y == 6)
            format = nvn::Format::RGBA_ASTC_10x6_SRGB;
        else if (header->block_x == 10 && header->block_y == 8)
            format = nvn::Format::RGBA_ASTC_10x8_SRGB;
        else if (header->block_x == 10 && header->block_y == 10)
            format = nvn::Format::RGBA_ASTC_10x10_SRGB;
        else if (header->block_x == 12 && header->block_y == 10)
            format = nvn::Format::RGBA_ASTC_12x10_SRGB;
        else if (header->block_x == 12 && header->block_y == 12)
            format = nvn::Format::RGBA_ASTC_12x12_SRGB;
        else
            HK_ABORT("Unknown ASTC block type %d %d", header->block_x, header->block_y);

        return format;
    }

    inline util::Vector2i getAstcSize(const void* tex) {
        const AstcHeader* header = reinterpret_cast<const AstcHeader*>(tex);
        return { header->getWidth(), header->getHeight() };
    }

} // namespace hk::gfx
