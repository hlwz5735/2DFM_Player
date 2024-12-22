#include "KgtPicture.hpp"
#include "2dfmFileReader.hpp"
#include "KgtPalette.hpp"
#include <stdexcept>

using _2dfm::ColorBgra;

namespace {
ColorBgra* buildColorData(int width, int height, const KgtPalette *palette, const void* picData) {
    if (auto pixels = static_cast<ColorBgra*>(malloc(width * height * sizeof(ColorBgra)))) {
            const auto rawData = static_cast<const byte *>(picData);
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    pixels[y * width + x] = palette->colors[rawData[y * width + x]];
                }
            }
            return pixels;
        }
        return nullptr;
    }
}

KgtPicture::KgtPicture(const KgtPicture &o) {
    width = o.width;
    height = o.height;
    hasPrivatePalette = o.hasPrivatePalette;
    compressed = o.compressed;
    sharedPalettes = o.sharedPalettes;

    auto size = o.width * o.height;
    if (size > 0) {
        rawData = static_cast<byte *>(malloc(size));
        memcpy(rawData, o.rawData, size);
    }
    if (hasPrivatePalette) {
        privatePalette = new KgtPalette;
        privatePalette->colors = o.privatePalette->colors;
    }
}

KgtPicture::KgtPicture(KgtPicture &&o) noexcept {
    width = o.width;
    height = o.height;
    hasPrivatePalette = o.hasPrivatePalette;
    compressed = o.compressed;
    sharedPalettes = std::move(o.sharedPalettes);
    rawData = o.rawData;
    o.rawData = nullptr;
    privatePalette = o.privatePalette;
    o.privatePalette = nullptr;
}

KgtPicture::~KgtPicture() {
    if (rawData) {
        free(rawData);
        rawData = nullptr;
    }
    if (privatePalette) {
        delete privatePalette;
        privatePalette = nullptr;
    }
}

KgtPicture &KgtPicture::operator=(const KgtPicture &o) {
    if (this == &o) {
        return *this;
    }
    width = o.width;
    height = o.height;
    hasPrivatePalette = o.hasPrivatePalette;
    compressed = o.compressed;
    sharedPalettes = o.sharedPalettes;

    if (rawData) {
        free(rawData);
        rawData = nullptr;
    }
    int size = o.width * o.height;
    if (size > 0) {
        rawData = static_cast<byte *>(malloc(size));
        memcpy(rawData, o.rawData, size);
    }
    if (privatePalette) {
        delete privatePalette;
        privatePalette = nullptr;
    }
    if (o.hasPrivatePalette) {
        privatePalette = new KgtPalette;
        privatePalette->colors = o.privatePalette->colors;
    }

    return *this;
}

KgtPicture & KgtPicture::operator=(KgtPicture &&o) noexcept {
    if (this == &o) {
        return *this;
    }
    width = o.width;
    height = o.height;
    hasPrivatePalette = o.hasPrivatePalette;
    compressed = o.compressed;
    sharedPalettes = o.sharedPalettes;

    if (rawData) {
        free(rawData);
        rawData = nullptr;
    }
    rawData = o.rawData;
    o.rawData = nullptr;

    if (privatePalette) {
        delete privatePalette;
        privatePalette = nullptr;
    }
    privatePalette = o.privatePalette;
    o.privatePalette = nullptr;

    return *this;
}

byte *KgtPicture::extractPixelData(int paletteNo) {
    KgtPalette *palette;
    if (hasPrivatePalette) {
        palette = privatePalette;
    } else {
        palette = sharedPalettes.at(paletteNo);
        if (!palette) {
            throw std::runtime_error("当前精灵帧既没有专用调色盘，也没有指定调色盘！");
        }
    }
    
    return reinterpret_cast<byte *>(buildColorData(width, height, palette, rawData));
}

ax::Texture2D *KgtPicture::createTexture(int paletteNo) {
    if (this->width == 0 || this->height == 0) {
        return nullptr;
    }
    byte *imageData = this->extractPixelData(paletteNo);

    auto tex = new ax::Texture2D();
    // 设置纹理过滤参数
    ax::Texture2D::TexParams texParams(ax::backend::SamplerFilter::NEAREST,    // 最小过滤：临近过滤
                                       ax::backend::SamplerFilter::NEAREST,    // 最大过滤：临近过滤
                                       ax::backend::SamplerAddressMode::CLAMP_TO_EDGE, // 横向纹理环绕模式：边缘钳制
                                       ax::backend::SamplerAddressMode::CLAMP_TO_EDGE  // 纵向纹理环绕模式：边缘钳制
    );
    tex->initWithData(imageData, width * height * sizeof(_2dfm::ColorBgra), ax::PixelFormat::BGRA8, width, height);
    tex->setTexParameters(texParams);

    free(imageData);

    return tex;
}

void KgtPicture::setFrom2dfmPicture(_2dfm::Picture *picture) {
    width = picture->header.width;
    height = picture->header.height;
    compressed = picture->header.size != 0;
    hasPrivatePalette = picture->header.hasPrivatePalette;
    auto size = get2dfmPictureSize(&picture->header);
    auto realSize = get2dfmPictureRealSize(&picture->header);

    byte *content;
    if (compressed) {
        content = _2dfm::decompress(picture->content, size, realSize);
    } else {
        content = picture->content;
    }
    
    if (hasPrivatePalette) {
        privatePalette = createPalette(reinterpret_cast<ColorBgra *>(content), true);
    } else {
        if (privatePalette) {
            delete privatePalette;
            privatePalette = nullptr;
        }
    }
    const auto picSize = hasPrivatePalette ? realSize - 1024 : realSize;
    const auto ppic = content + (hasPrivatePalette ? 1024 : 0);
    if (rawData) {
        free(rawData);
    }
    rawData = static_cast<byte *>(malloc(picSize));
    memcpy(rawData, ppic, picSize);
}

void KgtPicture::setSharedPalettes(const std::array<KgtPalette *, 8> &palettes) {
    sharedPalettes = palettes;
}
