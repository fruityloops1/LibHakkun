#pragma once

#include "hk/diag/diag.h"
#include "hk/gfx/Texture.h"
#include "hk/types.h"
#include "hk/util/Math.h"
#include "hk/util/Storage.h"
#include <cmath>

namespace hk::gfx {

    class Font {
        constexpr static int cCharsPerRow = 32;
        constexpr static float cCharWidthUv = 1.0f / cCharsPerRow;

        const char16_t* mCharset = nullptr;
        size mNumChars = 0;
        util::Storage<Texture> mTexture;

        struct FontHeader {
            size charsetSize;
            int width;
            int height;
            size texSize;
            u8 data[];
        };

    public:
        float getCharWidthUv() const {
            return cCharWidthUv;
        }

        float getCharHeightUv() const {
            return 1.0f / std::ceilf(mNumChars / float(cCharsPerRow));
        }

        Font(void* fontData, void* device, void* memory);

        static size calcMemorySize(void* nvnDevice, void* fontData) {
            FontHeader* header = reinterpret_cast<FontHeader*>(fontData);
            return alignUpPage((header->charsetSize + 1) * sizeof(char16_t)) + Texture::calcMemorySize(nvnDevice, header->width * header->height * sizeof(u8));
        }

        Texture& getTexture() { return *mTexture.get(); }

        ~Font() {
            mTexture.tryDestroy();
        }

        template <typename Char>
        util::Vector2f getCharUvTopLeft(Char value) {
            size idx;
            for (idx = 0; idx < mNumChars; idx++) { // meh
                if (mCharset[idx] == value)
                    break;
            }

            int row = idx / cCharsPerRow;
            int col = idx % cCharsPerRow;

            return { col * getCharWidthUv(), row * getCharHeightUv() };
        }

        util::Vector2f getGlyphSize() {
            auto texSize = util::Vector2f(mTexture.get()->getSize());
            return texSize / util::Vector2f(cCharsPerRow, std::ceilf(mNumChars / float(cCharsPerRow)));
        }
    };

} // namespace hk::gfx
