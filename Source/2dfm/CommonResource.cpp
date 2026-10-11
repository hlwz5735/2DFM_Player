#include "CommonResource.hpp"

#include "engine/SoundClip.hpp"

#include <axmol.h>

USING_NS_AX;

CommonResource::~CommonResource() {
    for (auto t : this->pictures) {
        if (t) {
            t->release();
        }
    }
    this->pictures.clear();
    this->scripts.clear();
    for (auto s : this->scriptItems) {
        delete s;
    }
    this->scriptItems.clear();
    for (auto &p : this->sharedPalettes) {
        delete p;
        p = nullptr;
    }
    for (auto s : this->sounds) {
        delete s;
    }
    this->sounds.clear();
}

std::vector<KgtScript>::iterator CommonResource::findNextWithFlag(
    std::vector<KgtScript>::iterator it,
    std::vector<KgtScript>::iterator end,
    ScriptSpecialFlag flag
) {
    while (it != end) {
        if (it->flags == flag) {
            return it;
        }
        ++it;
    }
    return end;
}

void createTexturesForCommonResource(CommonResource *cr, int paletteNo) {
    for (auto tex : cr->pictures) {
        if (tex) {
            tex->release();
        }
    }
    cr->pictures.clear();

    // 注意：pictures 必须与 spriteFrames 一一对应（下标即"图"脚本项里的图号），
    // 空槽位也要占位放 nullptr，不能压缩掉
    cr->pictures.reserve(cr->spriteFrames.size());
    size_t skipped = 0;  // 空槽位：2DFM 图片表按上限预留，正常跳过
    size_t failed = 0;   // 非空图像却创建失败，才是真问题
    for (auto &sf : cr->spriteFrames) {
        if (sf.isEmpty()) {
            ++skipped;
            cr->pictures.emplace_back(nullptr);
            continue;
        }
        auto *tex = sf.createTexture(paletteNo);
        if (tex == nullptr) {
            ++failed;
        }
        cr->pictures.emplace_back(tex);
    }

    if (failed > 0) {
        AXLOGE("[Tex] serial={}: {}/{} 张非空图像创建纹理失败（另有 {} 个空槽位正常跳过）", cr->serialNo, failed,
               cr->spriteFrames.size() - skipped, skipped);
    } else {
        AXLOGI("[Tex] serial={}: 创建 {} 张纹理，跳过 {} 个空槽位", cr->serialNo,
               cr->spriteFrames.size() - skipped, skipped);
    }
}
