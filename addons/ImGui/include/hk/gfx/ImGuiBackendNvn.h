#pragma once

#include "hk/types.h"
#include "hk/util/Math.h"

namespace hk::gfx {
    class ImGuiBackendNvnImpl;
    constexpr static size cImGuiBackendNvnImplSize = 2232; // This needs to GO

    class ImGuiBackendNvn {
        u8 mStorage[cImGuiBackendNvnImplSize];

        static ImGuiBackendNvn sInstance;

    public:
        ImGuiBackendNvnImpl* get() { return reinterpret_cast<ImGuiBackendNvnImpl*>(mStorage); }

        ImGuiBackendNvn();

        struct Allocator {
            using Alloc = void* (*)(size, size);
            using Free = void (*)(void*);

            Alloc alloc = nullptr;
            Free free = nullptr;
        };

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

        static ImGuiBackendNvn* instance() { return &sInstance; }
    };

} // namespace hk::gfx
