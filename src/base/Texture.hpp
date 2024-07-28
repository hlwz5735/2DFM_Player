#pragma once

#include "SpriteFrame.hpp"

class Texture {
public:
    explicit Texture(SpriteFrame *sf, int paletteNo = 0);
    ~Texture();

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    bool isBound() const { return bound; }

    void setActive() const;
private:
    int width;
    int height;
    bool bound = false;
    /// OpenGL 纹理ID
    unsigned int textureId;
};
