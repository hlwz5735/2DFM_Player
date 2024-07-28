#include "Texture.hpp"
#include <GL/glew.h>

void WriteToFile(char *name, void *buffer, size_t size) {
    FILE *fp;
    fopen_s(&fp, name, "wb");
    fwrite(buffer, 1, size, fp);
    fclose(fp);
}

Texture::Texture(SpriteFrame *sf, int paletteNo): textureId(0) {
    width = sf->width;
    height = sf->height;
    if (width == 0 || height == 0) {
        return;
    }

    byte *imageData = extractPixelDataFromSpriteFrame(sf, paletteNo);

    // 生成纹理对象，绑定纹理数据
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    // 传输图像数据至显卡
    // 参数含义：纹理类型，级别，OpenGL颜色格式，纹理宽，纹理高，边界（必须为0），像素颜色格式，输入数据的位宽， 图片数据的指针
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
    // 校验纹理信息
    //GLenum error = glGetError();
    //if (error != GL_NO_ERROR) {
    //    SDL_Log("OpenGL error %d occurred！", error);
    //}
    //if (glIsTexture(textureId) == GL_FALSE) {
    //    SDL_Log("OpenGL texture mapping failed!");
    //}
    //GLint tw, th;
    //glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tw);
    //glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &th);
    //if (tw != width || th != height) {
    //    SDL_Log("Texture creation error!");
    //}

    // 纹理信息传送至显卡后，即可销毁内存副本
    free(imageData);

    // 使用双线性过滤
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    bound = true;
}

Texture::~Texture() {
    glDeleteTextures(1, &textureId);
}

void Texture::setActive() const {
    if (bound) {
        glBindTexture(GL_TEXTURE_2D, textureId);
    }
}
