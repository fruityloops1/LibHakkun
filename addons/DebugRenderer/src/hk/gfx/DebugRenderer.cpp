#include <cstdarg>

#include "hk/diag/diag.h"
#include "hk/gfx/DebugRenderer.h"
#include "hk/hook/Trampoline.h"
#include "hk/util/Math.h"

#include "nvn/nvn_Cpp.h"
#include "nvn/nvn_CppFuncPtrBase.h"

#include "DebugRendererImpl.cpp"

namespace hk::gfx {

    static_assert(sizeof(DebugRendererImpl) == sizeof(DebugRenderer));

    DebugRenderer DebugRenderer::sInstance;

    DebugRenderer::DebugRenderer() {
        new (get()) DebugRendererImpl;
    }

    // wrappers

    bool DebugRenderer::tryInitialize() { return get()->tryInitialize(); }

    void DebugRenderer::setResolution(const util::Vector2f& res) { get()->setResolution(res); }
    void DebugRenderer::setGlyphSize(const util::Vector2f& size) { get()->setGlyphSize(size); }
    void DebugRenderer::setGlyphSize(float scale) { get()->setGlyphSize(scale); }
    void DebugRenderer::setGlyphHeight(float height) { get()->setGlyphHeight(height); }
    void DebugRenderer::setFont(Font* font) { get()->setFont(font); }
    void DebugRenderer::setCursor(const util::Vector2f& pos) { get()->setCursor(pos); }
    void DebugRenderer::setPrintColor(u32 color) { get()->setPrintColor(color); }

    void DebugRenderer::bindTexture(const TextureHandle& tex) { get()->bindTexture(tex); }
    void DebugRenderer::bindDefaultTexture() { get()->bindDefaultTexture(); }

    void DebugRenderer::clear() { get()->clear(); }
    void DebugRenderer::begin(void* commandBuffer) { get()->begin(static_cast<nvn::CommandBuffer*>(commandBuffer)); }
    void DebugRenderer::drawTri(const Vertex& a, const Vertex& b, const Vertex& c) { get()->drawTri(a, b, c); }
    void DebugRenderer::drawQuad(const Vertex& tl, const Vertex& tr, const Vertex& br, const Vertex& bl) { get()->drawQuad(tl, tr, br, bl); }
    util::Vector2f DebugRenderer::drawString(const util::Vector2f& pos, const char* str, u32 color) { return get()->drawString(pos, str, color); }
    util::Vector2f DebugRenderer::drawString(const util::Vector2f& pos, const char16_t* str, u32 color) { return get()->drawString(pos, str, color); }
    void DebugRenderer::printf(const char* fmt, ...) {
        std::va_list arg;
        va_start(arg, fmt);

        get()->printf(fmt, arg);

        va_end(arg);
    }
    void DebugRenderer::end() { get()->end(); }

    void DebugRenderer::setDevice(void* device) { get()->setDevice(static_cast<nvn::Device*>(device)); }
    void* DebugRenderer::getDevice() { return get()->getDevice(); }
    void DebugRenderer::setPrevTexturePool(void* pool) { get()->setPrevTexturePool(static_cast<nvn::TexturePool*>(pool)); }
    void DebugRenderer::setPrevSamplerPool(void* pool) { get()->setPrevSamplerPool(static_cast<nvn::SamplerPool*>(pool)); }

} // namespace hk::gfx
