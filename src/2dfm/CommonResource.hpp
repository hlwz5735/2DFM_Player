#pragma once

#include <vector>
#include "../base/SpriteFrame.hpp"

class Texture;
class SoundClip;

struct CommonResource {
    virtual ~CommonResource();

    /// �ű���Ϣ
    std::vector<_2dfm::Script *>scripts;
    std::vector<_2dfm::ScriptItem *>scriptItems;
    /// ����֡��Ϣ
    std::vector<SpriteFrame> spriteFrames;
    /// ����ѡ���ɫ�̣�����ֱ�ӱ���������
    std::vector<Texture *> pictures;
    /// ��ɫ����Ϣ
    std::vector<SDL_Palette *> sharedPalettes;
    /// ������Ϣ
    std::vector<SoundClip *> sounds;
};

void cleanCommonResource(CommonResource *res);
