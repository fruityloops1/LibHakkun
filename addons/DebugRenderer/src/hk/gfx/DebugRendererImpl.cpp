#include "gfx/DebugRendererSettings.h"
#include "hk/diag/diag.h"
#include "hk/gfx/Font.h"
#include "hk/gfx/Shader.h"
#include "hk/gfx/Texture.h"
#include "hk/gfx/Util.h"
#include "hk/gfx/Vertex.h"
#include "hk/nvn/MemoryBuffer.h"
#include "hk/types.h"
#include "hk/util/Math.h"
#include "hk/util/Storage.h"

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <initializer_list>
#include <string>

#include "nvn/nvn_Cpp.h"
#include "nvn/nvn_CppFuncPtrBase.h"
#include "nvn/nvn_CppMethods.h"

#include "embed_font.h"
#include "embed_shader.h"

__attribute__((weak)) hk::gfx::DebugRendererSettings hkDebugRendererSettings;
extern "C" __attribute__((weak)) void hkDebugRendererAfterInit(nvn::Device* device);

namespace hk::gfx {
    class DebugRendererImpl {
        constexpr static size cShaderBufferSize = alignUpPage(shader_bin_size);
        constexpr static size cVtxBufferSize = alignUpPage(0x1000 * sizeof(Vertex));
        constexpr static size cIdxBufferSize = alignUpPage(0x3000 * sizeof(u16));
        constexpr static size cDefaultTextureMemorySize = cPageSize * 2;
        constexpr static size cDefaultFontMemorySize = 0x8000;

        nvn::Device* mDevice = nullptr;
        nvn::CommandBuffer* mCurCommandBuffer;
        nvn::TexturePool* mPrevTexturePool = nullptr;
        nvn::SamplerPool* mPrevSamplerPool = nullptr;
        util::Storage<Shader> mShader;
        hk::util::Storage<Texture> mDefaultTexture;
        util::Storage<Font> mFont;
        uintptr_t mVtxOffset = 0;
        uintptr_t mCurVtxMap = 0;
        hk::nvn::MemoryBuffer mVtxBuffer;
        hk::nvn::MemoryBuffer mIdxBuffer;
        u8 mVtxBufferData[cVtxBufferSize] __attribute__((aligned(cPageSize))) { 0 };
        u8 mIdxBufferData[cIdxBufferSize] __attribute__((aligned(cPageSize))) { 0 };
        u8 mDefaultTextureBuffer[cDefaultTextureMemorySize] __attribute__((aligned(cPageSize))) { 0 };
        u8 mFontBuffer[cDefaultFontMemorySize] __attribute__((aligned(cPageSize))) { 0 };

        util::Vector2f mResolution { 1280, 720 };
        util::Vector2f mFontTextureGlyphSize;
        util::Vector2f mGlyphSize { 16, 24 };
        util::Vector2f mCursor { 0, 0 };
        u32 mPrintColor = rgba(255, 255, 255, 255);
        Font* mCurrentFont = mFont.get();
        bool mInitialized = false;

    public:
        void setDevice(nvn::Device* device) { mDevice = device; }
        void setResolution(const util::Vector2f& res) { mResolution = res; }
        void setGlyphSize(const util::Vector2f& size) { mGlyphSize = size; }
        void setGlyphSize(float scale) { mGlyphSize = mFontTextureGlyphSize * scale; }
        void setGlyphHeight(float height) { setGlyphSize(height / mFontTextureGlyphSize.y); }
        void setFont(Font* font) { mCurrentFont = font; }
        void setCursor(const util::Vector2f& pos) { mCursor = pos; }
        void setPrintColor(u32 color) { mPrintColor = color; }

        void setTexturePool(nvn::CommandBuffer* cmdBuf, nvn::TexturePool* pool) { mPrevTexturePool = pool; }
        void setSamplerPool(nvn::CommandBuffer* cmdBuf, nvn::SamplerPool* pool) { mPrevSamplerPool = pool; }

        bool tryInitializeProgram() {
            if (mInitialized)
                return false;
            initialize((u8*)shader_bin);
            mInitialized = true;
            return true;
        }

        void initialize(u8* program) {
            mShader.create(program, cShaderBufferSize, mDevice, nullptr, 0, nullptr, "hk::gfx::DebugRenderer");

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

            {
                mFont.create((void*)font_hkf, mDevice, mFontBuffer);
                mFontTextureGlyphSize = mFont.get()->getGlyphSize();
                mGlyphSize = mFontTextureGlyphSize;
            }

            if (hkDebugRendererAfterInit)
                hkDebugRendererAfterInit(mDevice);
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

            mShader.get()->use(cmdBuffer);

            nvn::PolygonState polyState;
            polyState.SetDefaults();
            polyState.SetPolygonMode(hkDebugRendererSettings.polygonMode);
            polyState.SetFrontFace(hkDebugRendererSettings.frontFace);
            cmdBuffer->BindPolygonState(&polyState);

            nvn::ColorState colorState;
            colorState.SetDefaults();
            colorState.SetLogicOp(hkDebugRendererSettings.logicOp);
            colorState.SetAlphaTest(hkDebugRendererSettings.alphaFunc);
            for (int i = 0; i < 8; ++i) {
                colorState.SetBlendEnable(i, hkDebugRendererSettings.blendEnable[i]);
            }
            cmdBuffer->BindColorState(&colorState);

            nvn::BlendState blendState;
            blendState.SetDefaults();
            blendState.SetBlendFunc(hkDebugRendererSettings.srcBlendFunc,
                hkDebugRendererSettings.dstBlendFunc,
                hkDebugRendererSettings.srcBlendFuncAlpha, hkDebugRendererSettings.dstBlendFuncAlpha);
            blendState.SetBlendEquation(hkDebugRendererSettings.blendEquationColor, hkDebugRendererSettings.blendEquationAlpha);
            cmdBuffer->BindBlendState(&blendState);

            nvn::DepthStencilState depthStencilState;
            depthStencilState.SetDefaults();
            depthStencilState.SetDepthWriteEnable(hkDebugRendererSettings.depthWriteEnable);
            cmdBuffer->BindDepthStencilState(&depthStencilState);

            cmdBuffer->BindVertexBuffer(0, mVtxBuffer.getAddress(), cVtxBufferSize);

            bindDefaultTexture();
        }

        void bindTexture(const TextureHandle& tex) {
            mCurCommandBuffer->SetTexturePool(static_cast<nvn::TexturePool*>(tex.texturePool));
            mCurCommandBuffer->SetSamplerPool(static_cast<nvn::SamplerPool*>(tex.samplerPool));
            mCurCommandBuffer->BindTexture(nvn::ShaderStage::FRAGMENT, 0, mDevice->GetTextureHandle(tex.textureId, tex.samplerId));
        }

        void bindDefaultTexture() {
            bindTexture(mDefaultTexture.get()->getTextureHandle());
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

        template <typename Char>
        util::Vector2f drawString(const util::Vector2f& pos, const Char* str, u32 color) {
            checkVtxBuffer();

            Vertex* vertices = reinterpret_cast<Vertex*>(mCurVtxMap);

            const uintptr_t initialOffset = mVtxOffset;
            util::Vector2f glyphSize = mGlyphSize / mResolution;

            float charWidthUv = mCurrentFont->getCharWidthUv();
            float charHeightUv = mCurrentFont->getCharHeightUv();

            auto adjustmentTL = mGlyphSize / mResolution / 15;
            auto adjustmentTR = adjustmentTL * util::Vector2f(-1, 1);
            auto adjustmentBR = adjustmentTL * util::Vector2f(-1, -1);
            auto adjustmentBL = adjustmentTL * util::Vector2f(1, -1);

            util::Vector2f curPos = pos / mResolution;
            float initialX = curPos.x;
            while (*str) {
                if (*str == '\n') {
                    curPos.x = initialX;
                    curPos.y += glyphSize.y;
                    str++;
                    continue;
                }
                if (*str == '\t') {
                    curPos.x += glyphSize.x * 4;
                    str++;
                    continue;
                }
                if (*str == '\r') {
                    curPos.x = initialX;
                    str++;
                    continue;
                }

                util::Vector2f tl = mCurrentFont->getCharUvTopLeft(*str);

                vertices[mVtxOffset++] = { curPos, tl + adjustmentTL, color };
                vertices[mVtxOffset++] = { curPos + util::Vector2f { glyphSize.x, 0 }, tl + util::Vector2f { charWidthUv, 0 } + adjustmentTR, color };
                vertices[mVtxOffset++] = { curPos + util::Vector2f { glyphSize.x, glyphSize.y }, tl + util::Vector2f { charWidthUv, charHeightUv } + adjustmentBR, color };
                vertices[mVtxOffset++] = { curPos + util::Vector2f { 0, glyphSize.y }, tl + util::Vector2f { 0, charHeightUv } + adjustmentBL, color };

                str++;
                curPos.x += glyphSize.x;
            }

            bindTexture(mCurrentFont->getTexture().getTextureHandle());

            mCurCommandBuffer->DrawArrays(nvn::DrawPrimitive::QUADS, initialOffset, mVtxOffset - initialOffset);

            bindDefaultTexture();

            return curPos;
        }

        void printf(const char* fmt, std::va_list arg) {
            size_t size = vsnprintf(nullptr, 0, fmt, arg);
            char* buf = (char*)__builtin_alloca(size + 1);
            vsnprintf(buf, size + 1, fmt, arg);

            mCursor = drawString(mCursor, buf, mPrintColor);
        }

        void end() {
            mCurCommandBuffer->SetTexturePool(mPrevTexturePool);
            mCurCommandBuffer->SetSamplerPool(mPrevSamplerPool);
        }

        nvn::Device* getDevice() const { return mDevice; }
    };

} // namespace hk::gfx
