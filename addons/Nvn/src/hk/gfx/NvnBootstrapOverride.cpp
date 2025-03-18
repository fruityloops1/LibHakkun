#include <cstdarg>

#include "hk/diag/diag.h"
#include "hk/hook/Trampoline.h"
#include "hk/util/Math.h"

#include "nvn/nvn_Cpp.h"
#include "nvn/nvn_CppFuncPtrBase.h"

#ifdef HK_ADDON_DebugRenderer
#include "hk/gfx/DebugRenderer.h"
#endif

#ifdef HK_ADDON_ImGui
#include "hk/gfx/ImGuiBackendNvn.h"
#endif

namespace hk::gfx {

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

    extern NvnBootstrapOverride sNvnOverrides[];

#ifdef HK_ADDON_DebugRenderer
    static bool sInitializeDebugRendererAutomatically = false;
#endif
#ifdef HK_ADDON_ImGui
    static bool sInitializeImGuiAutomatically = false;
#endif

    static bool sHooksInstalled = false;

    // override functions

    static NVNboolean nvnDeviceInitialize(nvn::Device* device, const nvn::DeviceBuilder* deviceBuilder) {
        NVNboolean success = sNvnOverrides[0].getOrigFunc<nvn::DeviceInitializeFunc>()(device, deviceBuilder);

        nvn::nvnLoadCPPProcs(device, sNvnOverrides[1].getOrigFunc<nvn::DeviceGetProcAddressFunc>());
#ifdef HK_ADDON_DebugRenderer
        DebugRenderer::instance()->setDevice(device);
#endif
#ifdef HK_ADDON_ImGui
        ImGuiBackendNvn::instance()->setDevice(device);
#endif

        return success;
    }

    nvn::GenericFuncPtrFunc nvnDeviceGetProcAddress(nvn::Device* device, const char* symbol);

    NVNboolean nvnCommandBufferInitialize(nvn::CommandBuffer* buf, nvn::Device* device) {
        NVNboolean success = sNvnOverrides[2].getOrigFunc<nvn::CommandBufferInitializeFunc>()(buf, device);

#ifdef HK_ADDON_DebugRenderer
        if (sInitializeDebugRendererAutomatically)
            DebugRenderer::instance()->tryInitialize();
#endif
#ifdef HK_ADDON_ImGui
        if (sInitializeImGuiAutomatically)
            ImGuiBackendNvn::instance()->tryInitialize();
#endif

        return success;
    }

    static void nvnCommandBufferSetTexturePool(nvn::CommandBuffer* buf, nvn::TexturePool* pool) {
        sNvnOverrides[3].getOrigFunc<nvn::CommandBufferSetTexturePoolFunc>()(buf, pool);
#ifdef HK_ADDON_DebugRenderer
        DebugRenderer::instance()->setPrevTexturePool(pool);
#endif
#ifdef HK_ADDON_ImGui
        ImGuiBackendNvn::instance()->setPrevTexturePool(pool);
#endif
    }

    void nvnCommandBufferSetSamplerPool(nvn::CommandBuffer* buf, nvn::SamplerPool* pool) {
        sNvnOverrides[4].getOrigFunc<nvn::CommandBufferSetSamplerPoolFunc>()(buf, pool);
#ifdef HK_ADDON_DebugRenderer
        DebugRenderer::instance()->setPrevSamplerPool(pool);
#endif
#ifdef HK_ADDON_ImGui
        ImGuiBackendNvn::instance()->setPrevSamplerPool(pool);
#endif
    }

    void nvnWindowSetCrop(nvn::Window* window, int x, int y, int width, int height) {
        sNvnOverrides[5].getOrigFunc<nvn::WindowSetCropFunc>()(window, x, y, width, height);

#ifdef HK_ADDON_DebugRenderer
        DebugRenderer::instance()->setResolution({ f32(width), f32(height) });
#endif
#ifdef HK_ADDON_ImGui
        ImGuiBackendNvn::instance()->setResolution({ f32(width), f32(height) });
#endif
    }

    //

    NvnBootstrapOverride sNvnOverrides[] {
        { "nvnDeviceInitialize", (void*)nvnDeviceInitialize },
        { "nvnDeviceGetProcAddress", (void*)nvnDeviceGetProcAddress },
        { "nvnCommandBufferInitialize", (void*)nvnCommandBufferInitialize },
        { "nvnCommandBufferSetTexturePool", (void*)nvnCommandBufferSetTexturePool },
        { "nvnCommandBufferSetSamplerPool", (void*)nvnCommandBufferSetSamplerPool },
        { "nvnWindowSetCrop", (void*)nvnWindowSetCrop },
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

    static void tryInstallBootstrapHook() {
        if (!sHooksInstalled)
            nvnBootstrap.installAtSym<"nvnBootstrapLoader">();
        sHooksInstalled = true;
    }

#ifdef HK_ADDON_DebugRenderer
    void DebugRenderer::installHooks(bool initializeAutomatically) {
        sInitializeDebugRendererAutomatically = initializeAutomatically;
        tryInstallBootstrapHook();
    }
#endif
#ifdef HK_ADDON_ImGui
    void ImGuiBackendNvn::installHooks(bool initializeAutomatically) {
        sInitializeImGuiAutomatically = initializeAutomatically;
        tryInstallBootstrapHook();
    }
#endif

} // namespace hk::gfx
