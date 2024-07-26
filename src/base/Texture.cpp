#include "Texture.hpp"

Texture::Texture(Renderer *renderer, SpriteFrame *sf, int paletteNo) {
    width = sf->width;
    height = sf->height;

    byte *imageData = extractPixelDataFromSpriteFrame(sf, paletteNo);

    // 生成纹理对象，绑定纹理数据
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    // 传输图像数据至显卡
    // 参数含义：纹理类型，级别，OpenGL颜色格式，纹理宽，纹理高，边界（必须为0），像素颜色格式，输入数据的位宽， 图片数据的指针
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
    // 纹理信息传送至显卡后，即可销毁内存副本
    free(imageData);

    // 使用双线性过滤
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Texture::~Texture() {
    glDeleteTextures(1, &textureId);
}

void Texture::setActive() const {
    glBindTexture(GL_TEXTURE_2D, textureId);
}
