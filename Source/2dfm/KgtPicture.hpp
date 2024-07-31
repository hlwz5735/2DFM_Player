#pragma once

#include <renderer/Texture2D.h>
#include <array>
#include "2dfmCommon.hpp"
#include "KgtPalette.hpp"

class Renderer;

class KgtPicture {
public:
    KgtPicture() = default;
    KgtPicture(const KgtPicture& o);
    KgtPicture(KgtPicture&& o) noexcept;
    ~KgtPicture();
    KgtPicture& operator=(const KgtPicture& o);
    KgtPicture& operator=(KgtPicture&& o) noexcept;

    int width = 0;
    int height = 0;
    bool hasPrivatePalette = false;
    bool compressed = false;

    void setFrom2dfmPicture(_2dfm::Picture *picture);
    void setSharedPalettes(const std::array<KgtPalette*, 8> &palettes);

    ax::Texture2D *createTexture(int paletteNo = 0);
protected:
    /// 从精灵帧数据中提取像素颜色信息
    byte *extractPixelData(int paletteNo = 0);
private:
    byte *rawData = nullptr;
    KgtPalette *privatePalette = nullptr;
    std::array<KgtPalette *, 8> sharedPalettes;
};
