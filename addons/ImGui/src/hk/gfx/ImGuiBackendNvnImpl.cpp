#include "gfx/Ubo.h"
#include "hk/diag/diag.h"
#include "hk/gfx/ImGuiBackendNvn.h"
#include "hk/gfx/Shader.h"
#include "hk/gfx/Texture.h"
#include "hk/types.h"
#include "hk/util/Math.h"
#include "hk/util/Storage.h"

#include "imgui.h"

#include "nvn/MemoryBuffer.h"
#include "nvn/nvn_Cpp.h"
#include "nvn/nvn_CppMethods.h"

#include "embed_shader.h"

namespace hk::gfx {

    static nvn::VertexAttribState* getImGuiAttribStates() {
        static nvn::VertexAttribState states[3];

        states[0].SetDefaults().SetFormat(nvn::Format::RG32F, offsetof(ImDrawVert, pos));
        states[1].SetDefaults().SetFormat(nvn::Format::RG32F, offsetof(ImDrawVert, uv));
        states[2].SetDefaults().SetFormat(nvn::Format::RGBA8, offsetof(ImDrawVert, col));
        return states;
    }

    static nvn::VertexStreamState* getImGuiStreamState() {
        static nvn::VertexStreamState state;

        state.SetDefaults().SetStride(sizeof(ImDrawVert));
        return &state;
    }

    class ImGuiBackendNvnImpl {
        constexpr static size cShaderBufferSize = alignUpPage(shader_bin_size);

        constexpr static size cInitialVtxBufferSize = 0x1000 * sizeof(ImDrawVert);
        constexpr static size cInitialIdxBufferSize = (cInitialVtxBufferSize / sizeof(ImDrawVert)) * 3 * sizeof(ImDrawIdx);

        constexpr static size cVtxIdxBuffering = 3;

        ImGuiBackendNvn::Allocator mAllocator;

        nvn::Device* mDevice = nullptr;
        util::Storage<Shader> mShader;
        hk::nvn::MemoryBuffer mVtxBuffer;
        hk::nvn::MemoryBuffer mIdxBuffer;
        util::Storage<Texture> mFontTexture;
        void* mFontTextureMemory = nullptr;
        nvn::TexturePool* mPrevTexturePool = nullptr;
        nvn::SamplerPool* mPrevSamplerPool = nullptr;
        nvn::Sync mBuffersSync;

        bool mInitialized = false;

    public:
        void setAllocator(const ImGuiBackendNvn::Allocator& allocator) { mAllocator = allocator; }
        void setDevice(nvn::Device* device) { mDevice = device; }
        void setPrevTexturePool(nvn::TexturePool* pool) { mPrevTexturePool = pool; }
        void setPrevSamplerPool(nvn::SamplerPool* pool) { mPrevSamplerPool = pool; }
        nvn::Device* getDevice() { return mDevice; }
        bool isInitialized() { return mInitialized; }

        bool tryInitialize() {
            if (mInitialized)
                return false;
            HK_ABORT_UNLESS(mAllocator.alloc && mAllocator.free, "No allocator set!", 0);
            initialize();
            mInitialized = true;
            return true;
        }

        void initBuffers(size vertSize, size idxSize) {
            if (mVtxBuffer.isInitialized()) {
                mAllocator.free(mVtxBuffer.getMemory());
                mVtxBuffer.finalize();
            }
            if (mIdxBuffer.isInitialized()) {
                mAllocator.free(mIdxBuffer.getMemory());
                mIdxBuffer.finalize();
            }

            vertSize = alignUpPage(vertSize);
            idxSize = alignUpPage(idxSize);

            void* vtxBuffer = mAllocator.alloc(vertSize, cPageSize);
            void* idxBuffer = mAllocator.alloc(idxSize, cPageSize);

            HK_ABORT_UNLESS(vtxBuffer != nullptr, "Vertex buffer allocation of 0x%lx returned nullptr", vertSize);
            HK_ABORT_UNLESS(idxBuffer != nullptr, "Index buffer allocation of 0x%lx returned nullptr", idxSize);

            mVtxBuffer.initialize(vtxBuffer, vertSize, mDevice, nvn::MemoryPoolFlags::CPU_UNCACHED | nvn::MemoryPoolFlags::GPU_CACHED);
            mIdxBuffer.initialize(idxBuffer, idxSize, mDevice, nvn::MemoryPoolFlags::CPU_UNCACHED | nvn::MemoryPoolFlags::GPU_CACHED);
            HK_ASSERT(mBuffersSync.Initialize(mDevice));
        }

        void initTexture(bool useLinearFilter = true) {
            auto& io = ImGui::GetIO();

            mFontTexture.tryDestroy();

            u8* pixels;
            int width, height;

            io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

            size texSize = width * height * sizeof(u8);
            size texMemorySize = Texture::calcMemorySize(mDevice, texSize);
            if (mFontTextureMemory)
                mAllocator.free(mFontTextureMemory);
            mFontTextureMemory = mAllocator.alloc(texMemorySize, cPageSize);
            HK_ABORT_UNLESS(mFontTextureMemory != nullptr, "Font texture allocation of 0x%lx returned nullptr", texMemorySize);

            {
                nvn::SamplerBuilder samp;
                samp.SetDefaults()
                    .SetMinMagFilter(
                        useLinearFilter ? nvn::MinFilter::LINEAR : nvn::MinFilter::NEAREST,
                        useLinearFilter ? nvn::MagFilter::LINEAR : nvn::MagFilter::NEAREST)
                    .SetWrapMode(nvn::WrapMode::CLAMP, nvn::WrapMode::CLAMP, nvn::WrapMode::CLAMP);
                nvn::TextureBuilder tex;
                tex.SetDefaults()
                    .SetTarget(nvn::TextureTarget::TARGET_2D)
                    .SetFormat(nvn::Format::R8)
                    .SetSize2D(width, height)
                    .SetSwizzle(nvn::TextureSwizzle::ONE, nvn::TextureSwizzle::ONE, nvn::TextureSwizzle::ONE, nvn::TextureSwizzle::R);
                mFontTexture.create(mDevice, &samp, &tex, texSize, pixels, mFontTextureMemory);
            }
        }

        void initialize() {
            ImGui::SetAllocatorFunctions(
                [](size_t size, void* backend) -> void* {
                    return static_cast<ImGuiBackendNvnImpl*>(backend)->mAllocator.alloc(size, 8);
                },
                [](void* ptr, void* backend) -> void {
                    static_cast<ImGuiBackendNvnImpl*>(backend)->mAllocator.free(ptr);
                },
                this);

            HK_ASSERT(ImGui::GetCurrentContext() == nullptr);

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();

            auto& io = ImGui::GetIO();
            io.IniFilename = nullptr;
            io.BackendPlatformName = "Horizon";
            io.BackendRendererName = "HkAddon ImGuiNvn";
            io.DisplaySize = { 1920, 1080 };

            mShader.create((u8*)shader_bin, cShaderBufferSize, mDevice, getImGuiAttribStates(), 3, getImGuiStreamState(), "hk::gfx::ImGuiBackendNvnImpl");

            initBuffers(cInitialVtxBufferSize, cInitialIdxBufferSize);
            initTexture(false);
        }

        void bindTexture(nvn::CommandBuffer* cmdBuffer, const TextureHandle& tex) {
            cmdBuffer->SetTexturePool(static_cast<nvn::TexturePool*>(tex.texturePool));
            cmdBuffer->SetSamplerPool(static_cast<nvn::SamplerPool*>(tex.samplerPool));
            cmdBuffer->BindTexture(nvn::ShaderStage::FRAGMENT, 0, mDevice->GetTextureHandle(tex.textureId, tex.samplerId));
        }

        void setDrawState(nvn::CommandBuffer* cmdBuffer) {
            nvn::PolygonState polyState;
            polyState.SetDefaults();
            polyState.SetPolygonMode(nvn::PolygonMode::FILL);
            polyState.SetFrontFace(nvn::FrontFace::CCW);
            cmdBuffer->BindPolygonState(&polyState);

            nvn::ColorState colorState;
            colorState.SetDefaults();
            colorState.SetLogicOp(nvn::LogicOp::COPY);
            colorState.SetAlphaTest(nvn::AlphaFunc::ALWAYS);
            for (int i = 0; i < 8; ++i) {
                colorState.SetBlendEnable(i, true);
            }
            cmdBuffer->BindColorState(&colorState);

            nvn::BlendState blendState;
            blendState.SetDefaults();
            blendState.SetBlendFunc(nvn::BlendFunc::SRC_ALPHA,
                nvn::BlendFunc::ONE_MINUS_SRC_ALPHA,
                nvn::BlendFunc::ONE, nvn::BlendFunc::ZERO);
            blendState.SetBlendEquation(nvn::BlendEquation::ADD, nvn::BlendEquation::ADD);
            cmdBuffer->BindBlendState(&blendState);

            nvn::DepthStencilState depthStencilState;
            depthStencilState.SetDefaults();
            depthStencilState.SetDepthWriteEnable(false);
            cmdBuffer->BindDepthStencilState(&depthStencilState);

            bindTexture(cmdBuffer, mFontTexture.get()->getTextureHandle());

            auto& dispSize = ImGui::GetIO().DisplaySize;
            mShader.get()->use(cmdBuffer);
        }

        void update() { }
        void draw(const ImDrawData& drawData, nvn::CommandBuffer* cmdBuffer) {
            if (drawData.TotalVtxCount == 0)
                return;
            mBuffersSync.Wait(UINT64_MAX);
            if (drawData.TotalVtxCount > mVtxBuffer.getSize() / sizeof(ImDrawVert) or drawData.TotalIdxCount > mIdxBuffer.getSize() / sizeof(ImDrawIdx)) {
                initBuffers(drawData.TotalVtxCount * sizeof(ImDrawVert), drawData.TotalIdxCount * sizeof(ImDrawIdx));
            }
            setDrawState(cmdBuffer);

            ImDrawVert* curVtx = reinterpret_cast<ImDrawVert*>(mVtxBuffer.map());
            ImDrawIdx* curIdx = reinterpret_cast<ImDrawIdx*>(mIdxBuffer.map());

            uintptr_t vtxGpuAddr = mVtxBuffer.getAddress();
            uintptr_t idxGpuAddr = mIdxBuffer.getAddress();

            const auto dispSize = ImGui::GetIO().DisplaySize;

            for (int i = 0; i < drawData.CmdListsCount; ++i) {
                const ImDrawList* cmdList = drawData.CmdLists[i];

                for (int j = 0; j < cmdList->VtxBuffer.Size; ++j) {
                    curVtx[j] = cmdList->VtxBuffer[j];
                    curVtx[j].pos.x /= dispSize.x;
                    curVtx[j].pos.y /= dispSize.y;
                }
                memcpy(curIdx, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
                mVtxBuffer.flush(vtxGpuAddr - mVtxBuffer.getAddress(), cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
                mIdxBuffer.flush(idxGpuAddr - mIdxBuffer.getAddress(), cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));

                cmdBuffer->BindVertexBuffer(0, vtxGpuAddr, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));

                for (int j = 0; j < cmdList->CmdBuffer.Size; j++) {
                    const ImDrawCmd* cmd = &cmdList->CmdBuffer[j];

                    const ImVec4& clipRec = cmd->ClipRect;
                    const util::Vector2f min { clipRec.x, clipRec.y };
                    const util::Vector2f max { clipRec.z, clipRec.w };
                    const util::Vector2f size = max - min;

                    cmdBuffer->SetScissor(min.x, min.y, size.x, size.y);
                    
                    #if IMGUI_VERSION_NUM>=19200
                    TextureHandle* texHandle = reinterpret_cast<TextureHandle*>(cmd->TexRef.GetTexID());
                    #else
                    TextureHandle* texHandle = reinterpret_cast<TextureHandle*>(cmd->GetTexID());
                    #endif
                    if (texHandle != nullptr) {
                        bindTexture(cmdBuffer, *texHandle);
                    } else {
                        bindTexture(cmdBuffer, mFontTexture.get()->getTextureHandle());
                    }

                    cmdBuffer->DrawElementsBaseVertex(
                        nvn::DrawPrimitive::TRIANGLES,
                        nvn::IndexType::UNSIGNED_SHORT,
                        cmd->ElemCount,
                        idxGpuAddr + cmd->IdxOffset * sizeof(ImDrawIdx), cmd->VtxOffset);
                }

                curVtx += cmdList->VtxBuffer.Size;
                curIdx += cmdList->IdxBuffer.Size;
                vtxGpuAddr += cmdList->VtxBuffer.Size * sizeof(ImDrawVert);
                idxGpuAddr += cmdList->IdxBuffer.Size * sizeof(ImDrawIdx);
            }

            cmdBuffer->FenceSync(&mBuffersSync, nvn::SyncCondition::ALL_GPU_COMMANDS_COMPLETE, nvn::SyncFlagBits());

            cmdBuffer->SetTexturePool(mPrevTexturePool);
            cmdBuffer->SetSamplerPool(mPrevSamplerPool);
        }
    };

} // namespace hk::gfx
