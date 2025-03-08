#pragma once

#include "hk/types.h"

namespace hk::gfx {

    struct UboImpl;
    constexpr static size cUboImplSize = 344; // This needs to GO

    class Ubo {
        u8 mStorage[cUboImplSize];

    public:
        UboImpl* get() { return reinterpret_cast<UboImpl*>(mStorage); }

        Ubo(void* device /* nvn::Device* */, void* memory, size uboSize, int stage = 0 /* nvn::ShaderStage::VERTEX */);
        ~Ubo();

        void bind(void* cmdBuf /* nvn::CommandBuffer* */, u32 binding);
        void update(void* cmdBuf, u32 binding, const void* data, size size);

        template <typename T>
        void update(void* cmdBuf, u32 binding, const T& data) {
            update(cmdBuf, binding, &data, sizeof(T));
        }
    };

} // namespace hk::gfx
