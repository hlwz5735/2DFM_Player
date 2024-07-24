#include "Texture.hpp"

Texture::Texture(Renderer *renderer, SpriteFrame *sf, int paletteNo) {
    width = sf->width;
    height = sf->height;
    sdlTexture = createTextureFromSpriteFrame(sf, renderer, paletteNo);
}

Texture::~Texture() {
    if (sdlTexture) {
        SDL_DestroyTexture(sdlTexture);
    }
}
