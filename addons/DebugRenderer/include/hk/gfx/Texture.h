#pragma once

#include "hk/types.h"
#include "hk/util/Math.h"

namespace hk::gfx {

    class TextureImpl;
    constexpr static size cTextureImplSize = 872; // This needs to GO

    class Texture {
        u8 mStorage[cTextureImplSize];

    public:
        TextureImpl* get() { return reinterpret_cast<TextureImpl*>(mStorage); }

        Texture(void* nvnDevice, void* samplerBuilder, void* textureBuilder, size texSize, void* texData, void* memory);
        ~Texture();

        util::Vector2i getSize();

        static size calcMemorySize(void* nvnDevice, size texSize);
    };

    struct AstcHeader {
        uint8_t magic[4];
        uint8_t block_x;
        uint8_t block_y;
        uint8_t block_z;
        uint8_t dim_x[3];
        uint8_t dim_y[3];
        uint8_t dim_z[3];

        int getWidth() const { return dim_x[0] + (dim_x[1] << 8) + (dim_x[2] << 16); }
        int getHeight() const { return dim_y[0] + (dim_y[1] << 8) + (dim_y[2] << 16); }
    };

} // namespace hk::gfx
