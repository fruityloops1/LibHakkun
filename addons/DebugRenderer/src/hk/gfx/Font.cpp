#include "hk/gfx/Font.h"
#include "gfx/Nvn.h"
#include "hk/diag/diag.h"
#include "nvn/nvn_Cpp.h"
#include "nvn/nvn_CppMethods.h"

namespace hk::gfx {
    Font::Font(void* fontData, void* device, void* memory) {
        HK_ABORT_UNLESS(alignUpPage(memory) == memory, "Memory must be page (%x) aligned (%p)", cPageSize, memory);

        FontHeader* header = reinterpret_cast<FontHeader*>(fontData);
        char16_t* charset = reinterpret_cast<char16_t*>(header->data);
        u8* textureData = header->data + (header->charsetSize + 1) * sizeof(char16_t);

        mCharset = reinterpret_cast<char16_t*>(memory);
        std::memcpy((void*)mCharset, charset, (header->charsetSize + 1) * sizeof(char16_t));
        mNumChars = header->charsetSize;

        nvn::SamplerBuilder samp;
        samp.SetDefaults()
            .SetMinMagFilter(nvn::MinFilter::LINEAR, nvn::MagFilter::LINEAR)
            .SetWrapMode(nvn::WrapMode::CLAMP, nvn::WrapMode::CLAMP, nvn::WrapMode::CLAMP);
        nvn::TextureBuilder tex;
        tex.SetDefaults()
            .SetTarget(nvn::TextureTarget::TARGET_2D)
            .SetFormat(getAstcFormat(textureData))
            .SetSize2D(header->width, header->height);
        uintptr_t textureBufferOffset = alignUpPage((header->charsetSize + 1) * sizeof(char16_t));
        mTexture.create(device, &samp, &tex, header->texSize, (void*)(uintptr_t(textureData) + sizeof(AstcHeader)), (void*)(uintptr_t(memory) + textureBufferOffset));
    }

} // namespace hk::gfx
