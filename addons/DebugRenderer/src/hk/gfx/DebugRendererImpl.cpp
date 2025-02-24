#include "MemoryBuffer.h"
#include "gfx/Texture.h"
#include "hk/diag/diag.h"
#include "hk/gfx/Vertex.h"
#include "hk/types.h"
#include "hk/util/Math.h"
#include "hk/util/Storage.h"
#include "nvn/nvn_Cpp.h"
#include "nvn/nvn_CppFuncPtrBase.h"
#include "nvn/nvn_CppMethods.h"
#include <cstdint>
#include <initializer_list>
#include <string>

#include "OBAMA_shader.h"
#include "base_shader.h"

#include "TextureImpl.cpp"

namespace hk::gfx {

    struct BinaryHeader {
        u32 fragmentControlOffset;
        u32 vertexControlOffset;
        u32 fragmentDataOffset;
        u32 vertexDataOffset;
    };

    class Font {
        constexpr static int cCharsPerRow = 32;
        constexpr static float cCharWidthUv = 1.0f / cCharsPerRow;

        const char16_t* mCharset = nullptr;
        size mNumChars = 0;
        util::Storage<Texture> mTexture;

        util::Vector2f getCharUvTopLeft(char16_t value) {
            size idx;
            for (idx = 0; idx < mNumChars; idx++) { // meh
                if (mCharset[idx] == value)
                    break;
            }

            int row = idx / cCharsPerRow;
            int col = idx % cCharsPerRow;

            return { col * getCharWidthUv(), row * getCharHeightUv() };
        }

        struct FontHeader {
            size charsetSize;
            int width;
            int height;
            u8 data[];
        };

    public:
        float getCharWidthUv() const {
            return cCharWidthUv;
        }

        float getCharHeightUv() const {
            return 1.0f / (mNumChars / float(cCharsPerRow));
        }

        Font(void* fontData, void* device, void* memory) {
            FontHeader* header = reinterpret_cast<FontHeader*>(fontData);
            char16_t* charset = reinterpret_cast<char16_t*>(header->data);
            u8* textureData = header->data + (header->charsetSize + 1) * sizeof(char16_t);

            std::memcpy((void*)mCharset, charset, (header->charsetSize + 1) * sizeof(char16_t));
            mNumChars = header->charsetSize;

            nvn::SamplerBuilder samp;
            samp.SetDefaults()
                .SetMinMagFilter(nvn::MinFilter::LINEAR, nvn::MagFilter::LINEAR)
                .SetWrapMode(nvn::WrapMode::CLAMP, nvn::WrapMode::CLAMP, nvn::WrapMode::CLAMP);
            nvn::TextureBuilder tex;
            tex.SetDefaults()
                .SetTarget(nvn::TextureTarget::TARGET_2D)
                .SetFormat(nvn::Format::R8)
                .SetSize2D(header->width, header->height);
            mTexture.create(device, &samp, &tex, header->width * header->height * 1, textureData, (void*)(uintptr_t(memory) + alignUpPage((header->charsetSize + 1) * sizeof(char16_t))));
        }

        static size calcMemorySize(void* nvnDevice, void* fontData) {
            FontHeader* header = reinterpret_cast<FontHeader*>(fontData);
            return alignUpPage((header->charsetSize + 1) * sizeof(char16_t)) + Texture::calcMemorySize(nvnDevice, header->width * header->height * sizeof(u8));
        }

        ~Font() {
            mTexture.tryDestroy();
        }
    };

    class DebugRendererImpl {
        constexpr static size cShaderBufferSize = alignUpPage(base_bin_size);
        constexpr static size cShaderUboBufferSize = 0x1000;
        constexpr static size cVtxBufferSize = alignUpPage(0x1000 * sizeof(Vertex));
        constexpr static size cIdxBufferSize = alignUpPage(0x3000 * sizeof(u16));

        nvn::Device* mDevice = nullptr;

        nvn::CommandBuffer* mCurCommandBuffer;
        struct {
            nvn::Program program;
            MemoryBuffer shaderBuffer;
            nvn::VertexStreamState streamState;
            nvn::VertexAttribState attribStates[3];
            nvn::ShaderData shaderDatas[2];
            MemoryBuffer uboBuffer;

            u8 uboPoolBuffer[cShaderBufferSize] __attribute__((aligned(cPageSize))) { 0 };
            bool initialized = false;
        } mShader;

        MemoryBuffer mVtxBuffer;
        MemoryBuffer mIdxBuffer;
        u8 mVtxBufferData[cVtxBufferSize] __attribute__((aligned(cPageSize))) { 0 };
        u8 mIdxBufferData[cIdxBufferSize] __attribute__((aligned(cPageSize))) { 0 };

        struct {
        } mUBO;

        util::Vector2f mResolution { 1, 1 };
        uintptr_t mVtxOffset = 0;
        uintptr_t mCurVtxMap = 0;

        hk::util::Storage<Texture> mDefaultTexture;
        u8 mDefaultTextureBuffer[0x20000] __attribute__((aligned(cPageSize))) { 0 };

        nvn::TexturePool* mPrevTexturePool = nullptr;
        nvn::SamplerPool* mPrevSamplerPool = nullptr;

    public:
        void setDevice(nvn::Device* device) { mDevice = device; }
        void setResolution(const util::Vector2f& res) { mResolution = res; }

        void setTexturePool(nvn::CommandBuffer* cmdBuf, nvn::TexturePool* pool) { mPrevTexturePool = pool; }
        void setSamplerPool(nvn::CommandBuffer* cmdBuf, nvn::SamplerPool* pool) { mPrevSamplerPool = pool; }

        bool tryInitializeProgram() {
            if (mShader.initialized)
                return false;
            initialize((u8*)base_bin);
            mShader.initialized = true;
            return true;
        }

        struct astc_header {
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

        nvn::Format getAstcFormat(void* tex) {
            const astc_header* header = reinterpret_cast<const astc_header*>(tex);
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

        void initialize(u8* program) {
            const BinaryHeader* header = reinterpret_cast<const BinaryHeader*>(program);

            HK_ASSERT(mShader.program.Initialize(mDevice));

            mShader.shaderBuffer.initialize(program, cShaderBufferSize, mDevice,
                nvn::MemoryPoolFlags::CPU_UNCACHED | nvn::MemoryPoolFlags::GPU_CACHED | nvn::MemoryPoolFlags::SHADER_CODE);
            nvn::BufferAddress addr = mShader.shaderBuffer.getAddress();

            mShader.shaderDatas[0].data = addr + header->vertexDataOffset;
            mShader.shaderDatas[0].control = program + header->vertexControlOffset;
            mShader.shaderDatas[1].data = addr + header->fragmentDataOffset;
            mShader.shaderDatas[1].control = program + header->fragmentControlOffset;

            HK_ASSERT(mShader.program.SetShaders(2, mShader.shaderDatas));

            mShader.program.SetDebugLabel("hk::gfx::DebugRenderer");

            mShader.attribStates[0].SetDefaults().SetFormat(nvn::Format::RG32F, offsetof(Vertex, pos));
            mShader.attribStates[1].SetDefaults().SetFormat(nvn::Format::RG32F, offsetof(Vertex, uv));
            mShader.attribStates[2].SetDefaults().SetFormat(nvn::Format::RGBA8, offsetof(Vertex, color));

            mShader.streamState.SetDefaults().SetStride(sizeof(Vertex));

            // shader.uboBuffer.initialize(shader.uboPoolBuffer, cShaderUboBufferSize, device,
            //     nvn::MemoryPoolFlags::CPU_UNCACHED | nvn::MemoryPoolFlags::GPU_CACHED);

            mVtxBuffer.initialize(mVtxBufferData, cVtxBufferSize, mDevice, nvn::MemoryPoolFlags::CPU_UNCACHED | nvn::MemoryPoolFlags::GPU_CACHED);
            mIdxBuffer.initialize(mIdxBufferData, cIdxBufferSize, mDevice, nvn::MemoryPoolFlags::CPU_UNCACHED | nvn::MemoryPoolFlags::GPU_CACHED);

            {
                nvn::SamplerBuilder samp;
                samp.SetDefaults()
                    .SetMinMagFilter(nvn::MinFilter::NEAREST, nvn::MagFilter::NEAREST)
                    .SetWrapMode(nvn::WrapMode::CLAMP, nvn::WrapMode::CLAMP, nvn::WrapMode::CLAMP);
                nvn::TextureBuilder tex;
                tex.SetDefaults()
                    .SetTarget(nvn::TextureTarget::TARGET_2D)
                    .SetFormat(nvn::Format::RGBA8)
                    .SetSize2D(2, 2);
                u32 texture[] { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
                mDefaultTexture.create(mDevice, &samp, &tex, sizeof(texture), texture, mDefaultTextureBuffer);
            }
        }

        void checkVtxBuffer() {
            HK_ABORT_UNLESS(mVtxOffset < cVtxBufferSize, "Vertex Buffer full!", 0);
        }

        void clear() {
            mCurVtxMap = uintptr_t(mVtxBuffer.map());
            mVtxOffset = 0;
        }

        void begin(nvn::CommandBuffer* cmdBuffer) {
            mCurCommandBuffer = cmdBuffer;

            cmdBuffer->BindProgram(&mShader.program, nvn::ShaderStageBits::ALL_GRAPHICS_BITS);

            // orthoRH_ZO(ubo.mProjMatrix, 0.0f, 1600, 900, 0.0f, -1.0f, 1.0f);
            // cmdBuffer->BindUniformBuffer(nvn::ShaderStage::VERTEX, 0, shader.uboBuffer.getAddress(), cShaderUboBufferSize);
            // cmdBuffer->BindUniformBuffer(nvn::ShaderStage::FRAGMENT, 0, shader.uboBuffer.getAddress(), cShaderUboBufferSize);
            // cmdBuffer->UpdateUniformBuffer(shader.uboBuffer.getAddress(), cShaderUboBufferSize, 0, sizeof(ubo), &ubo);

            nvn::PolygonState polyState;
            polyState.SetDefaults();
            polyState.SetPolygonMode(nvn::PolygonMode::FILL);
            polyState.SetFrontFace(nvn::FrontFace::CCW);
            cmdBuffer->BindPolygonState(&polyState);

            nvn::ColorState colorState;
            colorState.SetDefaults();
            colorState.SetLogicOp(nvn::LogicOp::COPY);
            colorState.SetAlphaTest(nvn::AlphaFunc::ALWAYS);
            for (int i = 0; i < 8; ++i) {
                colorState.SetBlendEnable(i, true);
            }
            cmdBuffer->BindColorState(&colorState);

            nvn::BlendState blendState;
            blendState.SetDefaults();
            blendState.SetBlendFunc(nvn::BlendFunc::SRC_ALPHA,
                nvn::BlendFunc::ONE_MINUS_SRC_ALPHA,
                nvn::BlendFunc::ONE, nvn::BlendFunc::ZERO);
            blendState.SetBlendEquation(nvn::BlendEquation::ADD, nvn::BlendEquation::ADD);
            cmdBuffer->BindBlendState(&blendState);

            nvn::DepthStencilState depthStencilState;
            depthStencilState.SetDefaults();
            depthStencilState.SetDepthWriteEnable(false);
            cmdBuffer->BindDepthStencilState(&depthStencilState);

            cmdBuffer->BindVertexAttribState(3, mShader.attribStates);
            cmdBuffer->BindVertexStreamState(1, &mShader.streamState);

            cmdBuffer->BindVertexBuffer(0, mVtxBuffer.getAddress(), cVtxBufferSize);

            bindDefaultTexture();
        }

        void bindTexture(Texture& tex) {
            auto handle = tex.get()->getHandle(mCurCommandBuffer);
            mCurCommandBuffer->BindTexture(nvn::ShaderStage::FRAGMENT, 0, handle);
        }

        void bindDefaultTexture() {
            bindTexture(*mDefaultTexture.get());
        }

        void drawTri(const Vertex& a, const Vertex& b, const Vertex& c) {
            checkVtxBuffer();

            Vertex* cur = reinterpret_cast<Vertex*>(mCurVtxMap + mVtxOffset * sizeof(Vertex));
            {
                int i = 0;
                for (const Vertex* vtx : { &a, &b, &c })
                    cur[i++] = { vtx->pos / mResolution, vtx->uv, vtx->color };
            }

            mCurCommandBuffer->DrawArrays(nvn::DrawPrimitive::TRIANGLES, mVtxOffset, 3);

            mVtxOffset += 3;
        }

        void drawQuad(const Vertex& tl, const Vertex& tr, const Vertex& br, const Vertex& bl) {
            checkVtxBuffer();

            Vertex* cur = reinterpret_cast<Vertex*>(mCurVtxMap + mVtxOffset * sizeof(Vertex));
            {
                int i = 0;
                for (const Vertex* vtx : { &tl, &tr, &br, &bl })
                    cur[i++] = { vtx->pos / mResolution, vtx->uv, vtx->color };
            }

            mCurCommandBuffer->DrawArrays(nvn::DrawPrimitive::QUADS, mVtxOffset, 4);

            mVtxOffset += 4;
        }

        void end() {
            mCurCommandBuffer->SetTexturePool(mPrevTexturePool);
            mCurCommandBuffer->SetSamplerPool(mPrevSamplerPool);
        }
    };

} // namespace hk::gfx
