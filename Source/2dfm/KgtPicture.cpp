#include "KgtPicture.hpp"
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
    // 空槽位直接跳过：不提取像素、不分配纹理
    if (isEmpty()) {
        return nullptr;
    }

    byte *imageData = this->extractPixelData(paletteNo);
    if (imageData == nullptr) {
        // 像素缓冲分配失败，交给调用方按"真失败"统计
        return nullptr;
    }

    auto tex = new ax::Texture2D();
    // 设置纹理过滤参数
    ax::Texture2D::TexParams texParams(ax::backend::SamplerFilter::NEAREST,    // 最小过滤：临近过滤
                                       ax::backend::SamplerFilter::NEAREST,    // 最大过滤：临近过滤
                                       ax::backend::SamplerAddressMode::CLAMP_TO_EDGE, // 横向纹理环绕模式：边缘钳制
                                       ax::backend::SamplerAddressMode::CLAMP_TO_EDGE  // 纵向纹理环绕模式：边缘钳制
    );
    const bool created = tex->initWithData(imageData, width * height * sizeof(_2dfm::ColorBgra),
                                           ax::PixelFormat::BGRA8, width, height);

    free(imageData);

    if (!created) {
        // 不返回无法使用的纹理，避免占用显存/内存
        delete tex;
        return nullptr;
    }
    tex->setTexParameters(texParams);

    return tex;
}

void KgtPicture::setFrom2dfmPicture(_2dfm::Picture *picture) {
    width = picture->header.width;
    height = picture->header.height;
    compressed = picture->header.size != 0;
    hasPrivatePalette = picture->header.hasPrivatePalette;

    if (isEmpty()) {
        // 空槽位（实测：整块头部为 0）。这里必须提前返回：
        // 否则下面的 malloc(0)/memcpy(0) 是无意义操作，而一旦这种槽位带上专用调色盘标记，
        // 还会去读 content 的 1024 字节——那是越界读
        if (rawData) {
            free(rawData);
            rawData = nullptr;
        }
        if (privatePalette) {
            delete privatePalette;
            privatePalette = nullptr;
        }
        return;
    }

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
    if (compressed) {
        // 解压出的临时缓冲已拷贝完成，释放，避免每帧泄漏
        free(content);
    }
}

void KgtPicture::setSharedPalettes(const std::array<KgtPalette *, 8> &palettes) {
    sharedPalettes = palettes;
}

KgtPalette *createPalette(const _2dfm::ColorBgra *originPalette, bool isPrivate) {
    auto palette = new KgtPalette();

    for (int i = 0; i < 256; ++i) {
        const auto &color = originPalette[i];
        palette->colors[i] = color;

        if (color.channel.red == 0 && color.channel.green == 0 && color.channel.blue == 0) {
            palette->colors[i].channel.alpha = 0;
        } else {
            if (isPrivate) {
                palette->colors[i].channel.alpha = 255;
            } else {
                palette->colors[i].channel.alpha = color.channel.alpha == 1 ? 255 : 0;
            }
        }
    }
    return palette;
}

int get2dfmPictureRealSize(const _2dfm::PictureHeader *sfh) {
    return sfh->width * sfh->height + (sfh->hasPrivatePalette ? 1024 : 0);
}

int get2dfmPictureSize(const _2dfm::PictureHeader *sfh) {
    if (sfh->size == 0) {
        return get2dfmPictureRealSize(sfh);
    }
    return sfh->size;
}
