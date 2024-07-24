#pragma once

#include <vector>
#include "../base/SpriteFrame.hpp"

class Texture;
class SoundClip;

struct CommonResource {
    /// �ű���Ϣ

    /// ����֡��Ϣ
    std::vector<SpriteFrame> spriteFrames;
    /// ����ѡ���ɫ�̣�����ֱ�ӱ���������
    std::vector<Texture *> pictures;
    /// ��ɫ����Ϣ
    std::vector<SDL_Palette *> sharedPalettes;
    /// ������Ϣ
    std::vector<SoundClip *> sounds;
};
