#include "hk/gfx/DebugRenderer.h"
#include "hk/diag/diag.h"
#include "hk/hook/Trampoline.h"
#include "hk/util/Math.h"

#include "nvn/nvn_Cpp.h"
#include "nvn/nvn_CppFuncPtrBase.h"

#include "DebugRendererImpl.cpp"

namespace hk::gfx {

    static_assert(sizeof(DebugRendererImpl) == sizeof(DebugRenderer));
    static_assert(sizeof(TextureImpl) == sizeof(Texture));

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

    //

    NvnBootstrapOverride sNvnOverrides[] {
        { "nvnDeviceInitialize", (void*)nvnDeviceInitialize },
        { "nvnDeviceGetProcAddress", (void*)nvnDeviceGetProcAddress },
        { "nvnCommandBufferInitialize", (void*)nvnCommandBufferInitialize },
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

    void DebugRenderer::clear() { get()->clear(); }
    void DebugRenderer::begin(void* commandBuffer) { get()->begin(reinterpret_cast<nvn::CommandBuffer*>(commandBuffer)); }
    void DebugRenderer::drawTri(const Vertex& a, const Vertex& b, const Vertex& c) { get()->drawTri(a, b, c); }
    void DebugRenderer::drawQuad(const Vertex& tl, const Vertex& tr, const Vertex& br, const Vertex& bl) { get()->drawQuad(tl, tr, br, bl); }
    void DebugRenderer::drawTest() { get()->drawTest(); }
    void DebugRenderer::end() { get()->end(); }

} // namespace hk::gfx
