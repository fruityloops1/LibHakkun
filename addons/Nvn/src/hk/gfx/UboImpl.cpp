#include "hk/gfx/Ubo.h"

#include "nvn/MemoryBuffer.h"
#include "nvn/nvn_Cpp.h"
#include "nvn/nvn_CppMethods.h"

namespace hk::gfx {

    struct UboImpl {
        void* memory;
        size uboSize;
        hk::nvn::MemoryBuffer uboBuffer;
        nvn::ShaderStage stage;
    };

    Ubo::Ubo(void* device, void* memory, size uboSize, int stage) {
        HK_ABORT_UNLESS(alignUp(memory, 0x100) == memory, "Memory must be (%x) aligned (%p)", 0x100, memory);

        auto* instance = get();
        instance->uboSize = uboSize;
        instance->memory = memory;
        instance->uboBuffer.initialize(memory, uboSize, static_cast<nvn::Device*>(device), nvn::MemoryPoolFlags::CPU_CACHED | nvn::MemoryPoolFlags::GPU_CACHED);
        instance->stage = nvn::ShaderStage::Enum(stage);
    }

    Ubo::~Ubo() {
        auto* instance = get();
        instance->uboBuffer.finalize();
        instance->uboSize = 0;
        instance->memory = nullptr;
    }

    void Ubo::bind(void* cmdBufPtr, u32 binding) {
        nvn::CommandBuffer* cmdBuf = static_cast<nvn::CommandBuffer*>(cmdBufPtr);
        nvn::BufferAddress addr = get()->uboBuffer.getAddress();

        cmdBuf->BindUniformBuffer(get()->stage, binding, addr, get()->uboSize);
    }

    void Ubo::update(void* cmdBufPtr, u32 binding, const void* data, size size) {
        nvn::CommandBuffer* cmdBuf = static_cast<nvn::CommandBuffer*>(cmdBufPtr);
        nvn::BufferAddress addr = get()->uboBuffer.getAddress();

        cmdBuf->BindUniformBuffer(get()->stage, binding, addr, get()->uboSize);
        cmdBuf->UpdateUniformBuffer(addr, get()->uboSize, binding, size, data);
    }

    static_assert(sizeof(UboImpl) == sizeof(Ubo));

} // namespace hk::gfx
