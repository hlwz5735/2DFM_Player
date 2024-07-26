#pragma once

#include "SpriteFrame.hpp"
#include <SDL.h>
#include <GL/glew.h>

class Renderer;

class Texture {
public:
    explicit Texture(Renderer *renderer, SpriteFrame *sf, int paletteNo = 0);
    ~Texture();

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    void setActive() const;
private:
    int width;
    int height;
    /// OpenGL 纹理ID
    unsigned int textureId;
};
