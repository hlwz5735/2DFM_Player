#pragma once

#include <vector>
#include "../base/SpriteFrame.hpp"

class SoundClip;

struct CommonResource {
    /// �ű���Ϣ

    /// ����֡��Ϣ
    std::vector<SpriteFrame> spriteFrames;
    /// ��ɫ����Ϣ
    std::vector<SDL_Palette *> sharedPalettes;
    /// ������Ϣ
    std::vector<SoundClip *> sounds;
};
