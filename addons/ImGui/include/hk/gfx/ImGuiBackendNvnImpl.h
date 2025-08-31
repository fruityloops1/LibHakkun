#pragma once

#include "hk/gfx/ImGuiAlloc.h"
#include "hk/gfx/Shader.h"
#include "hk/gfx/Texture.h"
#include "hk/types.h"
#include "hk/util/Storage.h"
#include "imgui.h"
#include "hk/nvn/MemoryBuffer.h"
#include "nvn/nvn_Cpp.h"

namespace hk::gfx {
    class ImGuiBackendNvnImpl {
        static size mShaderBufferSize;

        constexpr static size cInitialVtxBufferSize = 0x1000 * sizeof(ImDrawVert);
        constexpr static size cInitialIdxBufferSize = (cInitialVtxBufferSize / sizeof(ImDrawVert)) * 3 * sizeof(ImDrawIdx);

        constexpr static size cVtxIdxBuffering = 3;

        Allocator mAllocator;

        nvn::Device* mDevice = nullptr;
        util::Storage<Shader> mShader;
        hk::nvn::MemoryBuffer mVtxBuffer;
        hk::nvn::MemoryBuffer mIdxBuffer;
        util::Storage<Texture> mFontTexture;
        void* mFontTextureMemory = nullptr;
        nvn::TexturePool* mPrevTexturePool = nullptr;
        nvn::SamplerPool* mPrevSamplerPool = nullptr;
        nvn::Sync mBuffersSync;

        bool mInitialized = false;

    public:
        bool isInitialized();
        void setAllocator(const Allocator& allocator);
        void setDevice(nvn::Device* device);
        void setPrevTexturePool(nvn::TexturePool* pool);
        void setPrevSamplerPool(nvn::SamplerPool* pool);
        nvn::Device* getDevice();
        bool tryInitialize();
        void initBuffers(size vertSize, size idxSize);
        void initTexture(bool useLinearFilter = true);
        void bindTexture(nvn::CommandBuffer* cmdBuffer, const TextureHandle& tex);
        void setDrawState(nvn::CommandBuffer* cmdBuffer);
        void initialize();
        void update();
        void draw(const ImDrawData& drawData, nvn::CommandBuffer* cmdBuffer);
    };

} // namespace hk::gfx