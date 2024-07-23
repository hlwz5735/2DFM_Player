#pragma once

#include <vector>
#include <cstddef>
#include "../2dfm/2dfmCommon.hpp"

class SpriteFrame {
public:
    ~SpriteFrame();

    int width;
    int height;
    bool hasPrivatePalette;
    bool compressed;

    SDL_Texture *getTexture(class Renderer *renderer, int paletteNo = 0) const;

    void setFrom2dfmPicture(_2dfm::Picture *picture);
    void setSharedPalettes(SDL_Palette **palettes);
private:
    byte *rawData;
    SDL_Palette *privatePalette;
    SDL_Palette *sharedPalettes[8];
};
