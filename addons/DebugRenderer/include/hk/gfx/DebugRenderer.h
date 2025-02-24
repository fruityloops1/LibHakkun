#pragma once

#include "hk/gfx/Vertex.h"
#include "hk/util/Math.h"

namespace hk::gfx {

    class DebugRendererImpl;
    constexpr static size cDebugRendererImplSize = 184320; // This needs to GO

    class Texture;

    class DebugRenderer {
        u8 mStorage[cDebugRendererImplSize] __attribute__((aligned(cPageSize)));

        static DebugRenderer sInstance;

    public:
        DebugRenderer();

        DebugRendererImpl* get() { return reinterpret_cast<DebugRendererImpl*>(mStorage); }

        static DebugRenderer* instance() { return &sInstance; }

        void installHooks();

        void setResolution(const util::Vector2f& res);
        void setGlyphSize(const util::Vector2f& size);

        void bindTexture(Texture& tex);
        void bindDefaultTexture();

        void clear();
        void begin(void* commandBuffer /* nvn::CommandBuffer* */);
        void drawTri(const Vertex& a, const Vertex& b, const Vertex& c);
        void drawQuad(const Vertex& tl, const Vertex& tr, const Vertex& br, const Vertex& bl);
        void drawString(const util::Vector2f& pos, const char* str, u32 color);
        void drawString(const util::Vector2f& pos, const char16_t* str, u32 color);
        void end();
    };

} // namespace hk::gfx
