#pragma once

#include "../2dfm/2dfmCommon.hpp"
#include <cstddef>

class Renderer;

class SpriteFrame {
public:
    friend SDL_Texture *createTextureFromSpriteFrame(SpriteFrame *spriteFrame, Renderer *renderer, int paletteNo);

    SpriteFrame() = default;
    SpriteFrame(const SpriteFrame &o);
    SpriteFrame(SpriteFrame &&o) noexcept;
    ~SpriteFrame();
    SpriteFrame &operator=(const SpriteFrame &o);
    SpriteFrame &operator=(SpriteFrame &&o) noexcept;

    int width = 0;
    int height = 0;
    bool hasPrivatePalette = false;
    bool compressed = false;

    void setFrom2dfmPicture(_2dfm::Picture *picture);
    void setSharedPalettes(SDL_Palette *palettes[8]);
private:
    byte *rawData = nullptr;
    SDL_Palette *privatePalette = nullptr;
    SDL_Palette *sharedPalettes[8] = { nullptr };
};

SDL_Texture *createTextureFromSpriteFrame(class SpriteFrame *spriteFrame, Renderer *renderer, int paletteNo = 0);
