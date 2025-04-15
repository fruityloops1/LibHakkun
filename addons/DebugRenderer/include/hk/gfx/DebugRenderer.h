#pragma once

#include "hk/gfx/Font.h"
#include "hk/gfx/Vertex.h"
#include "hk/util/Math.h"

namespace hk::gfx {

    class DebugRendererImpl;
    constexpr static size cDebugRendererImplSize = 155648; // This needs to GO

    class Texture;

    class DebugRenderer {
        u8 mStorage[cDebugRendererImplSize] __attribute__((aligned(cPageSize)));

        static DebugRenderer sInstance;

    public:
        DebugRenderer();

        DebugRendererImpl* get() { return reinterpret_cast<DebugRendererImpl*>(mStorage); }

        static DebugRenderer* instance() { return &sInstance; }

        void installHooks(bool initializeAutomatically = true);
        bool tryInitialize();

        void setResolution(const util::Vector2f& res);
        void setGlyphSize(const util::Vector2f& size);
        void setGlyphSize(float scale);
        void setGlyphHeight(float height);
        void setFont(Font* font);
        void setCursor(const util::Vector2f& pos);
        void setPrintColor(u32 color);

        void bindTexture(const TextureHandle& tex);
        void bindDefaultTexture();

        void clear();
        void begin(void* commandBuffer /* nvn::CommandBuffer* */);
        void drawTri(const Vertex& a, const Vertex& b, const Vertex& c);
        void drawQuad(const Vertex& tl, const Vertex& tr, const Vertex& br, const Vertex& bl, f32 round = 0.0f, u32 numSides = 4);
        void drawLine(const Vertex& a, const Vertex& b, f32 width);
        void drawCircle(const Vertex& center, f32 radius, f32 width = 1.0f, u32 numSides = 16);
        void drawDisk(const Vertex& center, f32 radius, u32 numSides = 16);
        util::Vector2f drawString(const util::Vector2f& pos, const char* str, u32 color);
        util::Vector2f drawString(const util::Vector2f& pos, const char16_t* str, u32 color);
        void printf(const char* fmt, ...);
        void end();

        void setDevice(void* device /* nvn::Device* */);
        void* getDevice();
        void setPrevTexturePool(void* pool /* nvn::TexturePool* */);
        void setPrevSamplerPool(void* pool /* nvn::SamplerPool* */);
    };

} // namespace hk::gfx
