#include <stdexcept>
#include "../2dfm/2dfmFileReader.hpp"
#include "SpriteFrame.hpp"
#include "Renderer.hpp"

SDL_Surface *buildIndexSurfaceBySDL(int width, int height, SDL_Palette *palette, const void *picData) {
    if (const auto surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 8, 0, 0, 0, 0)) {
        if (SDL_LockSurface(surface) != 0) {
            SDL_FreeSurface(surface);
            return nullptr;
        }
        const auto pixels = static_cast<byte *>(surface->pixels);
        const auto rawData = static_cast<const byte *>(picData);
        auto res = SDL_SetSurfacePalette(surface, palette);
        if (res != 0) {
            SDL_Log("Error occured when setting palette for surface: %s", SDL_GetError());
            SDL_FreeSurface(surface );
            return nullptr;
        }
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                pixels[y * surface->pitch + x] = rawData[y * width + x];
            }
        }
        SDL_UnlockSurface(surface);
        return surface;
    }
    return nullptr;
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

SDL_Texture *SpriteFrame::getTexture(Renderer *renderer, int paletteNo) const {
    SDL_Palette *palette;
    if (hasPrivatePalette) {
        palette = privatePalette;
    } else {
        palette = sharedPalettes[paletteNo];
        if (!palette) {
            throw std::runtime_error("当前精灵帧既没有专用调色盘，也没有指定调色盘！");
        }
    }
    
    if (auto surface = buildIndexSurfaceBySDL(width, height, palette, rawData)) {
        auto tex = SDL_CreateTextureFromSurface(renderer->getSdlRenderer(), surface);
        SDL_FreeSurface(surface);
        return tex;
    }
    return nullptr;
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
        privatePalette = createSdlPalette(reinterpret_cast<_2dfm::ColorBgra *>(content));
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

void SpriteFrame::setSharedPalettes(SDL_Palette **palettes) {
    for (int i = 0; i < 8; ++i) {
        sharedPalettes[i] = palettes[i];
    }
}
