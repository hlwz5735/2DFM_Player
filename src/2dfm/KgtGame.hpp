//
// Created by 厉猛 on 2024-07-17.
//

#ifndef INC_2DFM_PLAYER_KGTGAME_HPP
#define INC_2DFM_PLAYER_KGTGAME_HPP

#include <string>
#include <vector>
#include "../math/Vector.hpp"
#include "2dfmFile.hpp"
#include "../base/SpriteFrame.hpp"

struct Reaction {
    std::string name;
    bool isHurtAction;
};

struct KgtGame {
    std::string projectName;
    // 脚本信息
    // 精灵帧信息
    std::vector<SpriteFrame> spriteFrames;
    // 调色盘信息
    std::vector<SDL_Palette *> sharedPalettes;
    // 声音信息
    std::vector<_2dfm::Sound *> sounds;
    // 角色名信息
    std::vector<std::string> playerNames;
    /// 受击反应列表
    std::vector<Reaction> reactions;
    /// 投掷反应列表
    std::vector<std::string> throwReactions;
    std::vector<std::string> stageNames;
    std::vector<std::string> demoNames;
    /// 影片配置信息
    _2dfm::DemoConfig demoConfig;
    /// 角色选择界面配置
    struct {
        Vector2 selectBoxStartPos;
        Vector2 playerAvatarIconSize;
        int rowCount;
        int columnCount;
        Vector2 player1PortraitPos;
        Vector2 player1PortraitOffset;
        Vector2 player2PortraitPos;
        Vector2 player2PortraitOffset;
    } charSelectConfig;
};

#endif //INC_2DFM_PLAYER_KGTGAME_HPP
