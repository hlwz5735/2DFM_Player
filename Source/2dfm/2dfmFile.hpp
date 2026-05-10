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

    // ==================== Player 文件特有数据结构 ====================
#pragma pack(push, 1)

    constexpr int COMMAND_ITEM_SIZE = 82;
    /// 指令项（出招表条目），82字节
    struct CommandItem {
        char name[32];
        /// 指令受理时间
        int16_t timeWindow;
        /// 空中执行脚本ID
        int16_t airScriptId;
        /// 站立近距执行脚本ID
        int16_t standNearScriptId;
        /// 站立远距执行脚本ID
        int16_t standFarScriptId;
        /// 下蹲时执行脚本ID
        int16_t crouchScriptId;
        /// 指令步骤信息（最多10步）
        /// 位结构: [15-14]特殊标记 [13-12]步骤状态 [11-10]无用 [9-4]A-F按键 [3-0]方向
        int16_t cmdData[10];
        /// 指令步骤附加信息（连打/蓄力/回转的时间或次数）
        int16_t cmdExtraData[10];
    };

    constexpr int THROW_ACTION_SIZE = 6;
    /// 投掷反应项，6字节
    struct PlayerThrowAction {
        int16_t picNo;
        int16_t offsetX;
        int16_t offsetY;
    };

    constexpr int AI_COMMAND_SIZE = 7;
    /// AI指令，7字节
    struct AiCommand {
        /// 头，无意义
        byte head;
        /// 方向
        byte dir;
        /// 结束标记
        byte flag;
        /// 指令序号
        int16_t cmdIdx;
        /// 延迟时间
        int16_t delayTime;
    };

    constexpr int AI_SLOT_SIZE = 111;
    /// AI档位，111字节
    struct AiSlot {
        char name[32];
        /// 第1位我方在空中，第2位对方在空中
        byte aiAirFlag;
        /// 确率
        byte aiProbability;
        /// 距离最小值
        int16_t rangeMin;
        /// 距离最大值
        int16_t rangeMax;
        /// 未使用的3字节空隙
        byte aiUnusedGap[3];
        /// AI指令序列（最多10条）
        AiCommand aiCmds[10];
    };
    constexpr int AI_SLOT_COUNT = 100;

    constexpr int STORY_ITEM_SIZE = 206;
    /// 故事项（原始数据），206字节
    struct StoryItemRaw {
        byte itemType;
        byte rawData[205];
    };

    /// 故事CPU配置，26字节
    struct StoryCpuConfig {
        /// 标记位（显示小血条/出现方式/战败后处理/战败效果/忽略方向）
        int32_t flags;
        /// CPU角色ID，0表示不出现
        byte cpuId;
        /// CPU级别
        byte cpuLevel;
        /// 按位设置的攻击对象表（从1-8分别对应角色和CPU1-7）
        byte atkBitmap;
        /// CPU出现的X坐标
        int16_t startXPos;
        /// 未知2字节
        int16_t unknown2B;
        /// 出现方式为"按时间"时，延迟多少秒出现
        byte showTimeSecond;
        /// 出现方式为"按时间"时，额外延迟随机多少秒以内出现
        byte showTimeExtraRandomSecond;
        /// 出现方式为"按血量"时，比对谁的血量（0=玩家，1-7=CPU1-7）
        byte showWhenWhosHpIsShort;
        /// 出现方式为"按血量"时，血量小于的值
        byte showWhenWhosHpIsLessThan;
        /// 胜利点数
        byte winPoint;
        /// CPU倒下时对应角色HP增加量（可为负）
        byte addHpWhenKo;
        /// CPU倒下时对应角色SP增加量（可为负）
        byte addSpWhenKo;
        /// CPU倒下时将胜利点赋予给谁（0=最后攻击者；1=玩家；2-8=CPU1-7）
        byte winPointTarget;
        /// 时间到时该CPU和谁比对体力（0=玩家，1-7=CPU1-7）
        byte compareWithWhoWhenTimeout;
        /// 时间到时体力比对获胜者获得的点数（100=该角色获胜）
        byte winPointGetWhenTimeout;
        /// 未知5字节
        byte unknown5B[5];
    };

    /// 战斗故事项（用于解释 StoryItemRaw，当 itemType==1 时），206字节
    struct BattleStoryItem {
        byte itemType;          // 固定为1
        byte stageId;           // 场景ID
        byte winRoundCount;     // 胜场回合数
        byte unknown1B;
        byte hpRecoverRate;     // HP回复率（100=全恢复）
        byte startRoundNum;     // 开始回合数
        int16_t roundTime;      // 回合时间（0=无限，最多999秒）
        int16_t playerStartXPos;// 角色在场景中的X坐标
        int16_t unknown2B;
        int32_t flags;          // 标志位（显示回合/显示开战/角色同屏）
        byte compareWithWhoWhenTimeout;
        byte winPointGetWhenTimeout;
        byte whoWillGotPointWhenKo;
        byte winPointGetWhenKo;
        int32_t unknown4B;
        StoryCpuConfig cpuConfigs[7];
    };

    /// DEMO故事项（用于解释 StoryItemRaw，当 itemType==2 时）
    struct DemoStoryItem {
        byte itemType;          // 固定为2
        byte demoId;            // DEMO编号（0=无DEMO）
    };

    /// 分歧故事项（用于解释 StoryItemRaw，当 itemType==3 时）
    struct SwitchStoryItem {
        byte itemType;          // 固定为3
        /// 分歧条件（0=无条件；1=上一舞台输了；2=生命槽小于x；3=全胜时）
        byte conditionType;
        /// 条件比较用值
        byte conditionValue;
        int16_t unknownShort;
        /// 跳转偏移量
        byte jumpOffset;
    };

    constexpr int STORY_ITEM_COUNT = 100;

    /// 角色属性配置
    struct PlayerStatsConfig {
        int16_t hpYPos;              // 多人混战时小HP槽的Y坐标
        int16_t nearThreshold;       // 近身判断阈值
        byte defenceAdjustRate;      // 防御攻击削减率（0视为1）
        byte hpAdjustStartPos;       // 生命修正开始计算的位置（百分比）
        byte hpAdjustRate;           // 生命修正率（0视为1）
        byte comboAdjustRate;        // 连击修正率（0视为1）
        byte defenceKey;             // 防御按键（0-5=A-F）
        int32_t hp;                  // 最大生命值
        int32_t sp;                  // 气力槽最大值
        int32_t maxSpSlot;           // 最大气点数
        int32_t playerFlags;         // 玩家配置标记位
        byte unknownGap4B[4];
        int16_t spAddWhenIAtk;       // 我方攻击成功时SP增加量
        int16_t spAddWhenEnemyAtk;   // 对方攻击成功时SP增加量
        int16_t initSpSlot;          // 初始气点数
        byte unknownGap5B[5];
    };

#pragma pack(pop)

    // 编译时验证结构体大小与文件格式一致
    static_assert(sizeof(CommandItem) == COMMAND_ITEM_SIZE, "CommandItem size mismatch");
    static_assert(sizeof(PlayerThrowAction) == THROW_ACTION_SIZE, "PlayerThrowAction size mismatch");
    static_assert(sizeof(AiCommand) == AI_COMMAND_SIZE, "AiCommand size mismatch");
    static_assert(sizeof(AiSlot) == AI_SLOT_SIZE, "AiSlot size mismatch");
    static_assert(sizeof(StoryItemRaw) == STORY_ITEM_SIZE, "StoryItemRaw size mismatch");
    static_assert(sizeof(StoryCpuConfig) == 26, "StoryCpuConfig size mismatch");
    static_assert(sizeof(BattleStoryItem) == STORY_ITEM_SIZE, "BattleStoryItem size mismatch");
}

#endif //INC_2DFM_PLAYER_2DFMFILE_HPP
