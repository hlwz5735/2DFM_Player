#pragma once

#include "KgtPicture.hpp"
#include <renderer/Texture2D.h>
#include <array>
#include <vector>

class Texture;
class SoundClip;

/// 脚本项特殊标记
enum class ScriptSpecialFlag {
    /// 一般脚本
    NORMAL = 0,
    /// 基本背景，基本图像中也是标题画面光标、续局光标、角色选择光标、暂停光标
    BACKGROUND = 1,
    /// 系统图像层级（DEMO）/角色位置层级（场景），基本图像中则是光标位置（标题、续局）
    SYSTEM = 3,
    /// 场景主UI 1-10+血槽+气槽
    STAGE_MAIN_UI = 9,
    /// 连击数字0-9+攻击互抵符号
    COMBO_SYMBOL = 33,
    /// 回合开始/结束时的动画、回合报数等
    ROUND = 57,
    /// 时间数字
    TIME_NUMBER = 65,
    /// 连击符号HIT
    HIT_SYMBOL = 97,
    /// 气槽数字
    SKILL_POINT_NUMBER = 129,
    /// 胜利标志
    VICTORY_FLAG = 193,
    /// 计时器位置
    TIMER_POS = 131,
    PLAYER_1_AVATAR_POS = 195,
    PLAYER_2_AVATAR_POS = 259,
    PLAYER_1_SKILL_POINT_POS = 323,
    PLAYER_2_SKILL_POINT_POS = 387,
    PLAYER_1_VICTORY_POS = 451,
    PLAYER_2_VICTORY_POS = 515,
};

struct KgtScript {
    ScriptSpecialFlag flags;
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
    std::vector<KgtPicture> spriteFrames;
    /// 根据选择的色盘，这里直接保存其纹理
    std::vector<ax::Texture2D *> pictures;
    /// 调色盘信息
    std::array<KgtPalette *, 8> sharedPalettes;
    /// 声音信息
    std::vector<SoundClip *> sounds;

    std::vector<KgtScript>::iterator findNextWithFlag(
        std::vector<KgtScript>::iterator it,
        std::vector<KgtScript>::iterator end,
        ScriptSpecialFlag flag
    );
};

// void cleanCommonResource(CommonResource *res);
