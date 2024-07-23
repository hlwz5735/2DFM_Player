#pragma once

#include <vector>
#include "../base/SpriteFrame.hpp"

class SoundClip;

struct CommonResource {
    /// 脚本信息

    /// 精灵帧信息
    std::vector<SpriteFrame> spriteFrames;
    /// 调色盘信息
    std::vector<SDL_Palette *> sharedPalettes;
    /// 声音信息
    std::vector<SoundClip *> sounds;
};
