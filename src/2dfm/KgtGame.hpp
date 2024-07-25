//
// Created by 厉猛 on 2024-07-17.
//

#ifndef INC_2DFM_PLAYER_KGTGAME_HPP
#define INC_2DFM_PLAYER_KGTGAME_HPP

#include "2dfmFile.hpp"
#include "../math/Vector.hpp"
#include "CommonResource.hpp"
#include <string>
#include <vector>

struct Reaction {
    std::string name;
    bool isHurtAction;
};

struct KgtGame : CommonResource {
    ~KgtGame() override;

    std::string projectName;
    /// 角色名称列表
    std::vector<std::string> playerNames;
    /// 受击反应列表
    std::vector<Reaction> reactions;
    /// 硬直时间配置
    _2dfm::RecoverTimeConfig recoverTimeConfig;
    /// 投掷反应列表
    std::vector<std::string> throwReactions;
    /// 场景列表
    std::vector<std::string> stageNames;
    /// 影片列表
    std::vector<std::string> demoNames;
    /// 影片配置信息
    _2dfm::GameDemoConfig demoConfig;

    /// 游戏基础设置
    struct {
        bool encryptGame;
        bool allowClash;
        bool enableStoryMode;
        bool enable1V1Mode;
        bool enableTeamMode;
        bool showHpAfterHpBar;
        bool pressToStart;
    } projectBaseConfig;

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

void cleanKgtGame(KgtGame *game);

#endif //INC_2DFM_PLAYER_KGTGAME_HPP
