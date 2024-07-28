#include <stdexcept>
#include "../2dfm/2dfmFileReader.hpp"
#include "SpriteFrame.hpp"
#include "Renderer.hpp"
namespace {
    SDL_Color *buildColorData(int width, int height, SDL_Palette *palette, const void *picData) {
        if (auto pixels = static_cast<SDL_Color *>(malloc(width * height * sizeof(SDL_Color)))) {
            const auto rawData = static_cast<const byte *>(picData);
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    pixels[y * width + x] = palette->colors[rawData[y * width + x]];
                }
            }
            return pixels;
        }
        return nullptr;
    }
}

SpriteFrame::SpriteFrame(const SpriteFrame &o) {
    width = o.width;
    height = o.height;
    hasPrivatePalette = o.hasPrivatePalette;
    compressed = o.compressed;
    sharedPalettes = o.sharedPalettes;

    int size = o.width * o.height * sizeof(byte);
    if (size > 0) {
        rawData = static_cast<byte *>(malloc(size));
        memcpy(rawData, o.rawData, size);
    }
    if (o.privatePalette) {
        privatePalette = SDL_AllocPalette(o.privatePalette->ncolors);
        memcpy(privatePalette->colors, o.privatePalette->colors, o.privatePalette->ncolors * sizeof(SDL_Color));
    }
}

SpriteFrame::SpriteFrame(SpriteFrame &&o) noexcept {
    width = o.width;
    height = o.height;
    hasPrivatePalette = o.hasPrivatePalette;
    compressed = o.compressed;
    sharedPalettes = o.sharedPalettes;

    rawData = o.rawData;
    o.rawData = nullptr;
    privatePalette = o.privatePalette;
    o.privatePalette = nullptr;
}

SpriteFrame::~SpriteFrame() {
    if (privatePalette) {
        SDL_FreePalette(privatePalette);
        privatePalette = nullptr;
    }
    if (rawData) {
        free(rawData);
        rawData = nullptr;
    }
}

SpriteFrame &SpriteFrame::operator=(const SpriteFrame &o) {
    if (this == &o) {
        return *this;
    }
    width = o.width;
    height = o.height;
    hasPrivatePalette = o.hasPrivatePalette;
    compressed = o.compressed;
    sharedPalettes = o.sharedPalettes;

    if (rawData) {
        free(rawData);
        rawData = nullptr;
    }
    int size = o.width * o.height * sizeof(byte);
    if (size > 0) {
        rawData = static_cast<byte *>(malloc(size));
        memcpy(rawData, o.rawData, size);
    }
    if (privatePalette) {
        SDL_FreePalette(privatePalette);
        privatePalette = nullptr;
    }
    if (o.hasPrivatePalette) {
        privatePalette = SDL_AllocPalette(o.privatePalette->ncolors);
        memcpy(privatePalette->colors, o.privatePalette->colors, o.privatePalette->ncolors * sizeof(SDL_Color));
    }

    return *this;
}

SpriteFrame & SpriteFrame::operator=(SpriteFrame &&o) noexcept {
    if (this == &o) {
        return *this;
    }
    width = o.width;
    height = o.height;
    hasPrivatePalette = o.hasPrivatePalette;
    compressed = o.compressed;
    sharedPalettes = o.sharedPalettes;
    if (rawData) {
        free(rawData);
        rawData = nullptr;
    }
    rawData = o.rawData;
    o.rawData = nullptr;
    if (privatePalette) {
        SDL_FreePalette(privatePalette);
        privatePalette = nullptr;
    }
    privatePalette = o.privatePalette;
    o.privatePalette = nullptr;

    return *this;
}

byte *extractPixelDataFromSpriteFrame(class SpriteFrame *spriteFrame, int paletteNo) {
    SDL_Palette *palette;
    if (spriteFrame->hasPrivatePalette) {
        palette = spriteFrame->privatePalette;
    } else {
        palette = spriteFrame->sharedPalettes[paletteNo];
        if (!palette) {
            throw std::runtime_error("当前精灵帧既没有专用调色盘，也没有指定调色盘！");
        }
    }
    
    return reinterpret_cast<byte *>(
            buildColorData(spriteFrame->width, spriteFrame->height, palette, spriteFrame->rawData)
    );
}

void SpriteFrame::setFrom2dfmPicture(_2dfm::Picture *picture) {
    width = picture->header.width;
    height = picture->header.height;
    compressed = picture->header.size != 0;
    hasPrivatePalette = picture->header.hasPrivatePalette;
    auto size = get2dfmPictureSize(&picture->header);
    auto realSize = get2dfmPictureRealSize(&picture->header);

    byte *content;
    if (compressed) {
        content = _2dfm::decompress(picture->content, size, realSize);
    } else {
        content = picture->content;
    }
    
    if (hasPrivatePalette) {
        privatePalette = createSdlPalette(reinterpret_cast<_2dfm::ColorBgra *>(content), true);
    } else {
        if (privatePalette) {
            SDL_FreePalette(privatePalette);
            privatePalette = nullptr;
        }
    }
    const auto picSize = hasPrivatePalette ? realSize - 1024 : realSize;
    const auto ppic = content + (hasPrivatePalette ? 1024 : 0);
    if (rawData) {
        free(rawData);
    }
    rawData = static_cast<byte *>(malloc(picSize));
    memcpy(rawData, ppic, picSize);
}

void SpriteFrame::setSharedPalettes(const std::array<SDL_Palette *, 8> &palettes) {
    sharedPalettes = palettes;
}
