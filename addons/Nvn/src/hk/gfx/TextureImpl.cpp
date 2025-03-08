#include "hk/diag/diag.h"
#include "hk/gfx/Texture.h"

#include "nvn/nvn_Cpp.h"
#include "nvn/nvn_CppMethods.h"

#include <new>

namespace hk::gfx {

    static int cSamplerDescriptorSize = 0;
    static int cTextureDescriptorSize = 0;

    class TextureImpl {
        nvn::Device* mDevice = nullptr;
        nvn::MemoryPool mMemForPools;
        nvn::TexturePool mTexturePool;
        nvn::SamplerPool mSamplerPool;
        nvn::MemoryPool mMemForTexture;
        nvn::Texture mTexture;
        nvn::Sampler mSampler;

    public:
        static size calcPoolsMemSize(nvn::Device* device) {
            device->GetInteger(nvn::DeviceInfo::SAMPLER_DESCRIPTOR_SIZE, &cSamplerDescriptorSize);
            device->GetInteger(nvn::DeviceInfo::TEXTURE_DESCRIPTOR_SIZE, &cTextureDescriptorSize);
            return alignUpPage(cSamplerDescriptorSize * 2 + cTextureDescriptorSize * 2);
        }

        TextureImpl(nvn::Device* device, nvn::SamplerBuilder& samplerBuilder, nvn::TextureBuilder& textureBuilder, size texSize, void* texData, void* memory) {
            HK_ABORT_UNLESS(alignUpPage(memory) == memory, "Memory must be page (%x) aligned (%p)", cPageSize, memory);

            size poolsSize = calcPoolsMemSize(device);
            {
                nvn::MemoryPoolBuilder builder;
                builder.SetDefaults()
                    .SetDevice(device)
                    .SetFlags(nvn::MemoryPoolFlags::CPU_UNCACHED | nvn::MemoryPoolFlags::GPU_CACHED)
                    .SetStorage(memory, poolsSize);

                HK_ASSERT(mMemForPools.Initialize(&builder));
            }

            HK_ASSERT(mSamplerPool.Initialize(&mMemForPools, 0, 2));
            HK_ASSERT(mTexturePool.Initialize(&mMemForPools, cSamplerDescriptorSize * 2, 2));

            {
                nvn::MemoryPoolBuilder builder;
                builder.SetDefaults()
                    .SetDevice(device)
                    .SetFlags(nvn::MemoryPoolFlags::CPU_UNCACHED | nvn::MemoryPoolFlags::GPU_CACHED)
                    .SetStorage((void*)(uintptr_t(memory) + poolsSize), alignUpPage(texSize));

                HK_ASSERT(mMemForTexture.Initialize(&builder));
            }

            textureBuilder = textureBuilder.SetStorage(&mMemForTexture, 0);
            textureBuilder = textureBuilder.SetDevice(device);
            samplerBuilder = samplerBuilder.SetDevice(device);

            HK_ASSERT(mTexture.Initialize(&textureBuilder));
            write(mTexture.GetWidth(), mTexture.GetHeight(), texData);

            HK_ASSERT(mSampler.Initialize(&samplerBuilder));

            mTexturePool.RegisterTexture(0, &mTexture, nullptr);
            mSamplerPool.RegisterSampler(0, &mSampler);
        }

        void write(int width, int height, void* data) {
            const nvn::CopyRegion region = {
                .xoffset = 0,
                .yoffset = 0,
                .zoffset = 0,
                .width = width,
                .height = height,
                .depth = 1
            };
            mTexture.WriteTexels(nullptr, &region, data);
            mTexture.FlushTexels(nullptr, &region);
        }

        ~TextureImpl() {
            mTexturePool.Finalize();
            mSamplerPool.Finalize();
            mTexture.Finalize();
            mSampler.Finalize();
            mMemForTexture.Finalize();
            mMemForPools.Finalize();
        }

        nvn::Texture& getTexture() { return mTexture; }
        TextureHandle getTextureHandle() {
            return { &mTexturePool, &mSamplerPool, 0, 0 };
        }
    };

    Texture::Texture(void* nvnDevice, void* samplerBuilder, void* textureBuilder, size texSize, void* texData, void* memory) {
        new (get()) TextureImpl(reinterpret_cast<nvn::Device*>(nvnDevice), *reinterpret_cast<nvn::SamplerBuilder*>(samplerBuilder), *reinterpret_cast<nvn::TextureBuilder*>(textureBuilder), texSize, texData, memory);
    }

    Texture::~Texture() {
        get()->~TextureImpl();
    }

    TextureHandle Texture::getTextureHandle() {
        return { get()->getTextureHandle() };
    }

    size Texture::calcMemorySize(void* nvnDevice, size texSize) {
        nvn::Device* device = reinterpret_cast<nvn::Device*>(nvnDevice);

        return TextureImpl::calcPoolsMemSize(device) + alignUpPage(texSize);
    }

    util::Vector2i Texture::getSize() {
        auto& tex = get()->getTexture();
        return { tex.GetWidth(), tex.GetHeight() };
    }

    static_assert(sizeof(TextureImpl) == sizeof(Texture));

} // namespace hk::gfx
