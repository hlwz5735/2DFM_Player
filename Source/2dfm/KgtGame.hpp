//
// Created by 厉猛 on 2024-07-17.
//

#ifndef INC_2DFM_PLAYER_KGTGAME_HPP
#define INC_2DFM_PLAYER_KGTGAME_HPP

#include "2dfmFile.hpp"
#include "CommonResource.hpp"
#include <axmol.h>
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
        ax::Vec2 selectBoxStartPos;
        ax::Vec2 playerAvatarIconSize;
        int rowCount;
        int columnCount;
        ax::Vec2 player1PortraitPos;
        ax::Vec2 player1PortraitOffset;
        ax::Vec2 player2PortraitPos;
        ax::Vec2 player2PortraitOffset;
    } charSelectConfig;

    bool isComboRight;

    int comboHitScriptId = 0;
    std::array<int, 10> comboNumberScriptIds {};
    int roundStartScriptId = 0;
    int roundEndScriptId = 0;
    std::array<int, 10> roundNumberScriptIds {};
    int finalRoundScriptId = 0;
    int fightScriptId = 0;
    int koScriptId = 0;
    int perfectGameScriptId = 0;
    int youWinScriptId;
    int youLoseScriptId;
    int player1WinScriptId = 0;
    int player2WinScriptId = 0;
    int drawGameScriptId = 0;
    int doubleKillScriptId = 0;
    int timeNumberInfinityScriptId = 0;
    std::array<int, 10> timeNumberScriptIds {};
    std::array<int, 10> skillPointNumberScriptIds {};
    int victorySymbolFillScriptId = 0;
    int victorySymbolOutlineScriptId = 0;
    std::array<int, 10> stageGuiScripts {};
    int player1HpBarScriptId = 0;
    int player2HpBarScriptId = 0;
    int player1SpBarScriptId = 0;
    int player2SpBarScriptId = 0;
    int timerPositionScriptId = 0;
    int player1AvatarPositionScriptId = 0;
    int player2AvatarPositionScriptId = 0;
    int player1SkillPointPositionScriptId = 0;
    int player2SkillPointPositionScriptId = 0;
    int player1VictoryFlagPositionScriptId = 0;
    int player2VictoryFlagPositionScriptId = 0;
    int titleCursorScriptId = 0;
    int storyModePositionScriptId;
    int pvpModePositionScriptId;
    int continueCursorScriptId = 0;
    int continuePositionScriptId = 0;
    int giveUpPositionScriptId = 0;
    int player1CharSelCursorScriptId = 0;
    int player2CharSelCursorScriptId = 0;
    int player1CharSelConfirmCursorScriptId = 0;
    int player2CharSelConfirmCursorScriptId = 0;
    int teamModePositionScriptId = 0;
    int pauseScriptId = 0;

    /// 游戏角色可选择性
    std::array<byte, _2dfm::maxPlayerNum> playerSelectableInfos{};

    int getOpeningDemoNo() const { return static_cast<int>(demoConfig.openingDemoId) - 1; }
    int getTitleDemoNo() const { return static_cast<int>(demoConfig.titleDemoId) - 1; }
    int getStoryModeCharSelectionDemoNo() const { return static_cast<int>(demoConfig.storyModeCharSelectDemoId) - 1; }

    ax::Vec2 getTitleStoryModePos() const;
    ax::Vec2 getTitlePvpModePos() const;
    ax::Vec2 getTitleTeamModePos() const;

    /// 初始化基础图像脚本信息
    void initBasicScriptInfos();

    bool isPlayerInStoryMode(int playerNo) const;
    bool isPlayerInPvpMode(int playerNo) const;
};

#endif //INC_2DFM_PLAYER_KGTGAME_HPP
