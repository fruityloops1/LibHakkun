#pragma once

#include "hk/types.h"

namespace hk::gfx {

    class ShaderImpl;
    constexpr static size cShaderImplSize = // This needs to GO
#ifdef __aarch64__
        552
#else
        544
#endif
        ;
    class Shader {
        u8 mStorage[cShaderImplSize];

    public:
        ShaderImpl* get() { return reinterpret_cast<ShaderImpl*>(mStorage); }

        Shader(u8* shaderData, size shaderSize, void* nvnDevice, void* attribStates, int numAttribStates, void* streamState, const char* shaderName);
        ~Shader();

        void use(void* nvnCommandBuffer);
    };

} // namespace hk::gfx
