#include "MemoryBuffer.h"
#include "gfx/Vertex.h"
#include "hk/gfx/Shader.h"
#include "nvn/nvn_Cpp.h"
#include <new>

namespace hk::gfx {

    static nvn::VertexAttribState* getDefaultAttribStates() {
        static nvn::VertexAttribState states[3];

        states[0].SetDefaults().SetFormat(nvn::Format::RG32F, offsetof(Vertex, pos));
        states[1].SetDefaults().SetFormat(nvn::Format::RG32F, offsetof(Vertex, uv));
        states[2].SetDefaults().SetFormat(nvn::Format::RGBA8, offsetof(Vertex, color));
        return states;
    }

    static nvn::VertexStreamState* getDefaultStreamState() {
        static nvn::VertexStreamState state;

        state.SetDefaults().SetStride(sizeof(Vertex));
        return &state;
    }

    class ShaderImpl {
        struct BinaryHeader {
            u32 fragmentControlOffset;
            u32 vertexControlOffset;
            u32 fragmentDataOffset;
            u32 vertexDataOffset;
        };

        nvn::Program mProgram;
        MemoryBuffer mShaderBuffer;
        int mNumAttribStates = 3;
        nvn::VertexAttribState* mAttribStates = nullptr;
        nvn::VertexStreamState* mStreamState = nullptr;
        nvn::ShaderData mShaderDatas[2];

    public:
        ShaderImpl(u8* shaderData, size shaderSize, nvn::Device* device, nvn::VertexAttribState* attribStates, int numAttribStates, nvn::VertexStreamState* streamState, const char* shaderName) {
            HK_ABORT_UNLESS(alignUpPage(shaderData) == shaderData, "Memory must be page (%x) aligned (%p)", cPageSize, shaderData);

            const BinaryHeader* header = reinterpret_cast<const BinaryHeader*>(shaderData);

            HK_ASSERT(mProgram.Initialize(device));

            mShaderBuffer.initialize(shaderData, shaderSize, device,
                nvn::MemoryPoolFlags::CPU_UNCACHED | nvn::MemoryPoolFlags::GPU_CACHED | nvn::MemoryPoolFlags::SHADER_CODE);
            nvn::BufferAddress addr = mShaderBuffer.getAddress();

            mShaderDatas[0].data = addr + header->vertexDataOffset;
            mShaderDatas[0].control = shaderData + header->vertexControlOffset;
            mShaderDatas[1].data = addr + header->fragmentDataOffset;
            mShaderDatas[1].control = shaderData + header->fragmentControlOffset;

            HK_ASSERT(mProgram.SetShaders(2, mShaderDatas));

            if (shaderName)
                mProgram.SetDebugLabel(shaderName);

            if (attribStates)
                mNumAttribStates = numAttribStates;
            mAttribStates = attribStates;
            mStreamState = streamState;

            if (mAttribStates == nullptr)
                mAttribStates = getDefaultAttribStates();
            if (mStreamState == nullptr)
                mStreamState = getDefaultStreamState();
        }

        void use(nvn::CommandBuffer* commandBuffer) {
            commandBuffer->BindProgram(&mProgram, nvn::ShaderStageBits::ALL_GRAPHICS_BITS);
            commandBuffer->BindVertexAttribState(mNumAttribStates, mAttribStates);
            commandBuffer->BindVertexStreamState(1, mStreamState);
        }
    };

    // wrappers

    Shader::Shader(u8* shaderData, size shaderSize, void* nvnDevice, void* attribStates, int numAttribStates, void* streamState, const char* shaderName) {
        new (get()) ShaderImpl(shaderData, shaderSize, static_cast<nvn::Device*>(nvnDevice), static_cast<nvn::VertexAttribState*>(attribStates), numAttribStates, static_cast<nvn::VertexStreamState*>(streamState), shaderName);
    }

    Shader::~Shader() {
        get()->~ShaderImpl();
    }

    void Shader::use(void* nvnCommandBuffer) { get()->use(static_cast<nvn::CommandBuffer*>(nvnCommandBuffer)); }

} // namespace hk::gfx
