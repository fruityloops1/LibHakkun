#include "hk/gfx/DebugRenderer.h"
#include "hk/diag/diag.h"
#include "hk/hook/Trampoline.h"
#include "hk/util/Math.h"

#include "nvn/nvn_Cpp.h"
#include "nvn/nvn_CppFuncPtrBase.h"

#include "DebugRendererImpl.cpp"
#include <cstdarg>

namespace hk::gfx {

    static_assert(sizeof(DebugRendererImpl) == sizeof(DebugRenderer));

    DebugRenderer DebugRenderer::sInstance;

    DebugRenderer::DebugRenderer() {
        new (get()) DebugRendererImpl;
    }

    struct NvnBootstrapOverride {
        const char* const name;
        void* const func;
        void* origFunc = 0;

        template <typename T>
        T getOrigFunc() {
            HK_ASSERT(origFunc != nullptr);
            return reinterpret_cast<T>(origFunc);
        }
    };

    // nvn variables

    extern NvnBootstrapOverride sNvnOverrides[];

    // override functions

    static NVNboolean nvnDeviceInitialize(nvn::Device* device, const nvn::DeviceBuilder* deviceBuilder) {
        NVNboolean success = sNvnOverrides[0].getOrigFunc<nvn::DeviceInitializeFunc>()(device, deviceBuilder);

        nvn::nvnLoadCPPProcs(device, sNvnOverrides[1].getOrigFunc<nvn::DeviceGetProcAddressFunc>());
        DebugRenderer::instance()->get()->setDevice(device);

        return success;
    }

    nvn::GenericFuncPtrFunc nvnDeviceGetProcAddress(nvn::Device* device, const char* symbol);

    NVNboolean nvnCommandBufferInitialize(nvn::CommandBuffer* buf, nvn::Device* device) {
        NVNboolean success = sNvnOverrides[2].getOrigFunc<nvn::CommandBufferInitializeFunc>()(buf, device);

        DebugRenderer::instance()->get()->tryInitializeProgram();

        return success;
    }

    static void nvnCommandBufferSetTexturePool(nvn::CommandBuffer* buf, nvn::TexturePool* pool) {
        sNvnOverrides[3].getOrigFunc<nvn::CommandBufferSetTexturePoolFunc>()(buf, pool);
        DebugRenderer::instance()->get()->setTexturePool(buf, pool);
    }

    void nvnCommandBufferSetSamplerPool(nvn::CommandBuffer* buf, nvn::SamplerPool* pool) {
        sNvnOverrides[4].getOrigFunc<nvn::CommandBufferSetSamplerPoolFunc>()(buf, pool);
        DebugRenderer::instance()->get()->setSamplerPool(buf, pool);
    }

    //

    NvnBootstrapOverride sNvnOverrides[] {
        { "nvnDeviceInitialize", (void*)nvnDeviceInitialize },
        { "nvnDeviceGetProcAddress", (void*)nvnDeviceGetProcAddress },
        { "nvnCommandBufferInitialize", (void*)nvnCommandBufferInitialize },
        { "nvnCommandBufferSetTexturePool", (void*)nvnCommandBufferSetTexturePool },
        { "nvnCommandBufferSetSamplerPool", (void*)nvnCommandBufferSetSamplerPool },
    };

    HkTrampoline<void*, const char*> nvnBootstrap = hook::trampoline([](const char* symbol) -> void* {
        void* func = nvnBootstrap.orig(symbol);

        for (int i = 0; i < util::arraySize(sNvnOverrides); i++) {
            auto& override = sNvnOverrides[i];
            if (__builtin_strcmp(override.name, symbol) == 0) {
                override.origFunc = func;
                return override.func;
            }
        }
        return func;
    });

    nvn::GenericFuncPtrFunc nvnDeviceGetProcAddress(nvn::Device* device, const char* symbol) {
        nvn::GenericFuncPtrFunc func = sNvnOverrides[1].getOrigFunc<nvn::DeviceGetProcAddressFunc>()(device, symbol);

        for (int i = 0; i < util::arraySize(sNvnOverrides); i++) {
            auto& override = sNvnOverrides[i];
            if (__builtin_strcmp(override.name, symbol) == 0) {
                override.origFunc = (void*)func;
                return (nvn::GenericFuncPtrFunc) override.func;
            }
        }

        return func;
    }

    void DebugRenderer::installHooks() {
        nvnBootstrap.installAtSym<"nvnBootstrapLoader">();
    }

    // wrappers

    void DebugRenderer::setResolution(const util::Vector2f& res) { get()->setResolution(res); }
    void DebugRenderer::setGlyphSize(const util::Vector2f& size) { get()->setGlyphSize(size); }
    void DebugRenderer::setGlyphSize(float scale) { get()->setGlyphSize(scale); }
    void DebugRenderer::setFont(Font* font) { get()->setFont(font); }
    void DebugRenderer::setCursor(const util::Vector2f& pos) { get()->setCursor(pos); }
    void DebugRenderer::setPrintColor(u32 color) { get()->setPrintColor(color); }

    void DebugRenderer::bindTexture(const TextureHandle& tex) { get()->bindTexture(tex); }
    void DebugRenderer::bindDefaultTexture() { get()->bindDefaultTexture(); }

    void DebugRenderer::clear() { get()->clear(); }
    void DebugRenderer::begin(void* commandBuffer) { get()->begin(reinterpret_cast<nvn::CommandBuffer*>(commandBuffer)); }
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

    void* DebugRenderer::getDevice() { return get()->getDevice(); }

} // namespace hk::gfx
