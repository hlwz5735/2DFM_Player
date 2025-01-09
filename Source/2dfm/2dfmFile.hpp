//
// Created by 厉猛 on 2024-07-17.
//

#ifndef INC_2DFM_PLAYER_2DFMFILE_HPP
#define INC_2DFM_PLAYER_2DFMFILE_HPP

#include <vector>
#include "2dfmCommon.hpp"

namespace _2dfm {
    /// 最多256字节的名字信息
    struct NameInfo {
        char name[256];
    };

    /// 受击动作列表
    struct ReactionItem {
        char reactionName[32];
        int isHurtAction;
    };

    /// 投掷反应列表
    struct ThrowReaction {
        char name[32];
    };

    struct CommonResourcePart {
        int scriptCount = 0;
        byte *rawScriptsData = nullptr;
        int scriptItemCount = 0;
        ScriptItem *scriptItems = nullptr;
        int pictureCount = 0;
        // 精灵帧信息是变长的
        std::vector<Picture *> pictures;
        ColorBgra *sharedPalettes[8];
        int soundCount;
        std::vector<Sound *> sounds;
    };

    /// 清理资源数据
    void freeCommonResourcePart(CommonResourcePart *crp);

    constexpr int KGT_FILE_HEADER_SIZE = 16 + 256;
    struct KgtFileHeader {
        byte fileSignature[16];
        NameInfo name;
    };

    struct RecoverTimeConfig {
        byte gap;
        byte attackRecoverTime;
        byte defenceRecoverTime;
        byte clashRecoverTime;
    };

    constexpr int DEMO_CONFIG_SIZE = 8;
    struct GameDemoConfig {
        byte titleDemoId;
        byte storyModeCharSelectDemoId;
        byte oneVsOneModeCharSelectDemoId;
        byte teamModeCharSelectDemoId;
        byte continueDemoId;
        byte openingDemoId;
        byte unknownTag1;
        byte unkownTag2;
    };

    union ProjectBaseConfig {
        std::int32_t rawValue;
        // x,标题画面按键才出现光标，小血槽旁显示血量，使用团队对战模式，使用单人对战模式，使用故事模式，攻击互抵，生成不能被编辑器读取的文件
        struct {
            int encryptGame: 1;
            int allowClash: 1;
            int enableStoryMode: 1;
            int enable1V1Mode: 1;
            int enableTeamMode: 1;
            int showHpAfterHpBar: 1;
            int pressToStart: 1;
        } value;
    };

    struct CharSelectConfig {
        std::int16_t selectBoxStartX;
        std::int16_t selectBoxStartY;
        std::int16_t iconWidth;
        std::int16_t iconHeight;
        std::int16_t columnNum;
        std::int16_t rowNum;
        std::int16_t player1PortraitX;
        std::int16_t player1PortraitY;
        std::int16_t player1PortraitTeamOffsetX;
        std::int16_t player1PortraitTeamOffsetY;
        std::int16_t player2PortraitX;
        std::int16_t player2PortraitY;
        std::int16_t player2PortraitTeamOffsetX;
        std::int16_t player2PortraitTeamOffsetY;
    };

    struct KgtDemoConfig {
        int16_t bgmSoundId;
        bool pressToSkip;
        int16_t unknownGap;
        int32_t totalTime;
    };

    struct KgtStageConfig {
        int32_t bgmSoundId;
    };
}

#endif //INC_2DFM_PLAYER_2DFMFILE_HPP
