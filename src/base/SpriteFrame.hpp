#pragma once

#include "../2dfm/2dfmCommon.hpp"
#include <array>

class Renderer;

class SpriteFrame {
public:
    friend byte *extractPixelDataFromSpriteFrame(class SpriteFrame *spriteFrame, int paletteNo);

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
    void setSharedPalettes(const std::array<SDL_Palette *, 8> &palettes);
private:
    byte *rawData = nullptr;
    SDL_Palette *privatePalette = nullptr;
    std::array<SDL_Palette *, 8>sharedPalettes = { nullptr };
};

/// 从精灵帧数据中提取像素颜色信息
byte *extractPixelDataFromSpriteFrame(class SpriteFrame *spriteFrame, int paletteNo = 0);
