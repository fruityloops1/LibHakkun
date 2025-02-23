#pragma once

#include "hk/types.h"

namespace hk::gfx {

    class TextureImpl;
    constexpr static size cTextureImplSize = 872; // This needs to GO

    class Texture {
        u8 mStorage[cTextureImplSize];

    public:
        TextureImpl* get() { return reinterpret_cast<TextureImpl*>(mStorage); }

        Texture(void* nvnDevice, void* samplerBuilder, void* textureBuilder, size texSize, void* texData, void* memory);
        ~Texture();

        static size calcMemorySize(void* nvnDevice, size texSize);
    };

} // namespace hk::gfx
