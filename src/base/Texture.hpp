#pragma once

#include "SpriteFrame.hpp"
#include <SDL.h>

class Renderer;

class Texture {
public:
    explicit Texture(Renderer *renderer, SpriteFrame *sf, int paletteNo = 0);
    ~Texture();

    SDL_Texture * getSdlTexture() const { return sdlTexture; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
private:
    int width;
    int height;
    SDL_Texture *sdlTexture;
};
