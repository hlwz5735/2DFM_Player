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

    /// 空槽位：2DFM 的图片表按文件类型的上限预留槽位，绝大多数槽位整块头部为 0（宽高都是 0），
    /// 既没有像素数据也没有专用调色盘，应当直接跳过、不创建纹理
    bool isEmpty() const { return width <= 0 || height <= 0; }

    ax::Texture2D *createTexture(int paletteNo = 0);
protected:
    /// 从精灵帧数据中提取像素颜色信息
    byte *extractPixelData(int paletteNo = 0);
private:
    byte *rawData = nullptr;
    KgtPalette *privatePalette = nullptr;
    std::array<KgtPalette *, 8> sharedPalettes;
};

/// 创建调色盘。纯黑视为透明色；专用调色盘的所有颜色视为不透明
class KgtPalette *createPalette(const _2dfm::ColorBgra *originPalette, bool isPrivate = false);

/// 图片解压后的真实数据大小（像素数据 + 可能的专用调色盘 1024 字节）
int get2dfmPictureRealSize(const _2dfm::PictureHeader *header);
/// 图片在文件中的存储大小（size 为 0 时即真实大小，否则为压缩后大小）
int get2dfmPictureSize(const _2dfm::PictureHeader *header);
