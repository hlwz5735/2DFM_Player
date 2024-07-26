#pragma once

#include <vector>
#include "../base/SpriteFrame.hpp"

class Texture;
class SoundClip;

struct KgtScript {
    int flags;
    std::string name;
    int startIdx;
    int endIdx;
};

struct CommonResource {
    virtual ~CommonResource();

    /// 脚本信息
    std::vector<KgtScript> scripts;
    std::vector<_2dfm::ScriptItem *> scriptItems;
    /// 精灵帧信息
    std::vector<SpriteFrame> spriteFrames;
    /// 根据选择的色盘，这里直接保存其纹理
    std::vector<Texture *> pictures;
    /// 调色盘信息
    std::vector<SDL_Palette *> sharedPalettes;
    /// 声音信息
    std::vector<SoundClip *> sounds;
};

void cleanCommonResource(CommonResource *res);
