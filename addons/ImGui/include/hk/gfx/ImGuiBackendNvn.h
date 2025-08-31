#pragma once

#include "hk/types.h"
#include "hk/util/Math.h"
#include "hk/gfx/ImGuiBackendNvnImpl.h"

namespace hk::gfx {
    constexpr static size cImGuiBackendNvnImplSize = sizeof(ImGuiBackendNvnImpl);

    class ImGuiBackendNvn {
        u8 mStorage[cImGuiBackendNvnImplSize];

        static ImGuiBackendNvn sInstance;

    public:
        ImGuiBackendNvnImpl* get() { return reinterpret_cast<ImGuiBackendNvnImpl*>(mStorage); }

        ImGuiBackendNvn();

        void installHooks(bool initializeAutomatically = true);

        bool tryInitialize();
        void initTexture(bool useLinearFilter = false);

        void update();
        void draw(const void* imDrawData, void* cmdBuf /* nvn::CommandBuffer* */);

        void setAllocator(const Allocator& allocator);
        void setDevice(void* device /* nvn::Device* */);
        void setPrevTexturePool(void* pool /* nvn::TexturePool* */);
        void setPrevSamplerPool(void* pool /* nvn::SamplerPool* */);
        void setResolution(const util::Vector2f& res);
        void* /* nvn::Device* */ getDevice();
        bool isInitialized();

        static ImGuiBackendNvn* instance() { return &sInstance; }
    };

} // namespace hk::gfx
