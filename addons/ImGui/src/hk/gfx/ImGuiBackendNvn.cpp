#include "hk/gfx/ImGuiBackendNvn.h"

#include <new>

#include "ImGuiBackendNvnImpl.cpp"
#include "imgui.h"

namespace hk::gfx {

    static_assert(sizeof(ImGuiBackendNvnImpl) == sizeof(ImGuiBackendNvn));

    ImGuiBackendNvn ImGuiBackendNvn::sInstance;

    ImGuiBackendNvn::ImGuiBackendNvn() {
        new (get()) ImGuiBackendNvnImpl;
    }

    bool ImGuiBackendNvn::tryInitialize() {
        return get()->tryInitialize();
    }
    void ImGuiBackendNvn::initTexture(bool useLinearFilter) { get()->initTexture(useLinearFilter); }

    void ImGuiBackendNvn::update() {
        get()->update();
    }

    void ImGuiBackendNvn::draw(const void* drawData, void* cmdBuf) {
        get()->draw(*static_cast<const ImDrawData*>(drawData), static_cast<nvn::CommandBuffer*>(cmdBuf));
    }

    void ImGuiBackendNvn::setAllocator(const Allocator& allocator) { get()->setAllocator(allocator); }
    void ImGuiBackendNvn::setDevice(void* device) { get()->setDevice(static_cast<nvn::Device*>(device)); }
    void ImGuiBackendNvn::setPrevTexturePool(void* pool) { get()->setPrevTexturePool(static_cast<nvn::TexturePool*>(pool)); }
    void ImGuiBackendNvn::setPrevSamplerPool(void* pool) { get()->setPrevSamplerPool(static_cast<nvn::SamplerPool*>(pool)); }

    void ImGuiBackendNvn::setResolution(const util::Vector2f& res) {
        ImGui::GetIO().DisplaySize = ImVec2(res.x, res.y);
    }

} // namespace hk::gfx
