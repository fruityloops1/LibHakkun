#pragma once

#include "hk/gfx/Ubo.h"
#include "hk/types.h"

namespace hk::gfx {

    class ShaderImpl;
    constexpr static size cShaderImplSize = 568; // This needs to GO

    class Shader {
        u8 mStorage[cShaderImplSize];
        Ubo* mUbo = nullptr;
        u32 mUboBinding = 0;
        u32 mReserved;

    public:
        ShaderImpl* get() { return reinterpret_cast<ShaderImpl*>(mStorage); }

        Shader(u8* shaderData, size shaderSize, void* nvnDevice, void* attribStates, int numAttribStates, void* streamState, const char* shaderName);
        ~Shader();

        void use(void* nvnCommandBuffer);

        void setUbo(Ubo* ubo, u32 binding) {
            mUbo = ubo;
            mUboBinding = binding;
        }
    };

} // namespace hk::gfx
