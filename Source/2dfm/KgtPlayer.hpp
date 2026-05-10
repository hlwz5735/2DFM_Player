//
// Created by limen on 2025/1/9.
//

#ifndef KGTPLAYER_HPP
#define KGTPLAYER_HPP

#include "CommonResource.hpp"
#include "2dfmFile.hpp"
#include <string>
#include <array>

// ==================== 指令相关 ====================

/// 指令方向枚举
enum class CommandDirection : byte {
    FREE = 0,
    D5 = 1,   // 中性(5)
    D6 = 2,   // 右(6)
    D3 = 3,   // 右下(3)
    D2 = 4,   // 下(2)
    D1 = 5,   // 左下(1)
    D4 = 6,   // 左(4)
    D7 = 7,   // 左上(7)
    D8 = 8,   // 上(8)
    D9 = 9,   // 右上(9)
    D741 = 10, // 左上、左、左下
    D789 = 11, // 左上、上、右上
    D963 = 12, // 右上、右、右下
    D123 = 13, // 左下、下、右下
};

/// 指令步骤状态
enum class CommandStepState : byte {
    INACTIVE = 0,   // 未活动
    EXCLUDED = 1,   // 未参与（位于终结步骤之后）
    FINAL = 2,      // 结尾步骤
    NORMAL = 3,     // 正常步骤
};

/// 指令步骤特殊标记
enum class CommandStepMark : byte {
    NORMAL = 0,     // 一般步骤
    RAPID = 1,      // 连按
    CHARGE = 2,     // 蓄力
    ROTATE = 3,     // 回转
};

/// 解析后的指令步骤
struct CommandStep {
    CommandDirection direction;
    /// 按键标记（A-F 6个按键，位0=A，位5=F）
    byte buttons;
    CommandStepState state;
    CommandStepMark mark;
    /// 附加数据（连打次数/蓄力时间/回转时间）
    int16_t extraData;
};

/// 出招指令
struct PlayerCommand {
    std::string name;
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
    /// 指令步骤（最多10步，仅包含有效步骤）
    std::vector<CommandStep> steps;
};

// ==================== 投掷反应 ====================

/// 投掷动作信息
struct ThrowActionInfo {
    /// 图号
    int16_t picNo;
    /// X偏移量
    int16_t offsetX;
    /// Y偏移量
    int16_t offsetY;
};

// ==================== AI ====================

/// AI单条指令
struct AiCommandEntry {
    /// 方向
    byte dir;
    /// 是否为结束标记
    bool isEnd;
    /// 指令序号
    int16_t cmdIdx;
    /// 延迟时间
    int16_t delayTime;
};

/// AI档位
struct AiSlotInfo {
    std::string name;
    /// 空中标记（第1位=我方在空中，第2位=对方在空中）
    byte airFlag;
    /// 确率
    byte probability;
    /// 距离最小值
    int16_t rangeMin;
    /// 距离最大值
    int16_t rangeMax;
    /// AI指令序列
    std::vector<AiCommandEntry> commands;
};

// ==================== 故事模式 ====================

/// 故事项类型
enum class StoryItemType : byte {
    BATTLE = 1,
    DEMO = 2,
    SWITCH = 3,
    END = 4,
};

/// 故事模式CPU配置
struct StoryCpuInfo {
    /// 标记位
    int32_t flags;
    /// CPU角色ID（0=不出现）
    byte cpuId;
    /// CPU级别
    byte cpuLevel;
    /// 攻击对象表（按位）
    byte atkBitmap;
    /// 出现X坐标
    int16_t startXPos;
    /// 出现方式为"按时间"时延迟秒数
    byte showTimeSecond;
    /// 额外延迟随机秒数
    byte showTimeExtraRandomSecond;
    /// 出现方式为"按血量"时比对谁（0=玩家，1-7=CPU1-7）
    byte showWhenWhosHpIsShort;
    /// 血量小于该值时出现
    byte showWhenWhosHpIsLessThan;
    /// 胜利点数
    byte winPoint;
    /// CPU倒下时HP增加量
    byte addHpWhenKo;
    /// CPU倒下时SP增加量
    byte addSpWhenKo;
    /// CPU倒下时胜利点赋予给谁
    byte winPointTarget;
    /// 时间到时和谁比对体力
    byte compareWithWhoWhenTimeout;
    /// 时间到时获胜者获得的点数
    byte winPointGetWhenTimeout;
};

/// 战斗故事项
struct BattleStoryInfo {
    /// 场景ID
    byte stageId;
    /// 胜场回合数
    byte winRoundCount;
    /// HP回复率
    byte hpRecoverRate;
    /// 开始回合数
    byte startRoundNum;
    /// 回合时间（0=无限）
    int16_t roundTime;
    /// 角色在场景中的X坐标
    int16_t playerStartXPos;
    /// 标志位
    int32_t flags;
    /// 时间到时玩家和谁比对体力
    byte compareWithWhoWhenTimeout;
    /// 时间到时获胜者获得的点数
    byte winPointGetWhenTimeout;
    /// 玩家倒下时为谁添加胜利点数
    byte whoWillGotPointWhenKo;
    /// 玩家倒下时获得的胜利点数
    byte winPointGetWhenKo;
    /// 7个CPU配置
    std::array<StoryCpuInfo, 7> cpuConfigs;
};

/// DEMO故事项
struct DemoStoryInfo {
    /// DEMO编号（0=无DEMO）
    byte demoId;
};

/// 分歧故事项
struct SwitchStoryInfo {
    /// 分歧条件（0=无条件；1=上一舞台输了；2=生命槽小于x；3=全胜时）
    byte conditionType;
    /// 条件比较用值
    byte conditionValue;
    /// 跳转偏移量
    byte jumpOffset;
};

/// 故事项（统一容器）
struct StoryItem {
    StoryItemType type;
    /// 战斗数据（仅 type==BATTLE 时有效）
    BattleStoryInfo battleInfo;
    /// DEMO数据（仅 type==DEMO 时有效）
    DemoStoryInfo demoInfo;
    /// 分歧数据（仅 type==SWITCH 时有效）
    SwitchStoryInfo switchInfo;
};

// ==================== 角色属性 ====================

/// 角色属性配置
struct PlayerStats {
    int age;
    byte sexFlag;
    int16_t hpYPos;
    int16_t nearThreshold;
    byte defenceAdjustRate;
    byte hpAdjustStartPos;
    byte hpAdjustRate;
    byte comboAdjustRate;
    /// 防御按键（0-5=A-F）
    byte defenceKey;
    int32_t hp;
    int32_t sp;
    int32_t maxSpSlot;
    /// 玩家配置标记位
    int32_t playerFlags;
    int16_t spAddWhenIAtk;
    int16_t spAddWhenEnemyAtk;
    int16_t initSpSlot;
};

// ==================== KgtPlayer 主结构 ====================

struct KgtPlayer : CommonResource {
    std::string playerName;

    /// 中招反应绑定信息（脚本ID数组，索引对应KGT中定义的中招反应列表）
    std::vector<int> hurtBinds;
    /// 投掷动作绑定信息
    std::vector<ThrowActionInfo> throwActions;
    /// 出招指令列表
    std::vector<PlayerCommand> commands;
    /// AI档位列表
    std::vector<AiSlotInfo> aiSlots;
    /// 故事条目列表
    std::vector<StoryItem> storyItems;
    /// 角色属性
    PlayerStats stats{};

    /// 通常站立 脚本项ID
    int standScriptId = 0;
    /// 前行 脚本项ID
    int forwardScriptId = 0;
    /// 后退 脚本项ID
    int backwardScriptId = 0;
    /// 上跳 脚本项ID
    int jumpUpScriptId = 0;
    /// 前跳 脚本项ID
    int jumpForwardScriptId = 0;
    /// 后跳 脚本项ID
    int jumpBackwardScriptId = 0;
    /// 空中落下 脚本项ID
    int fallingScriptId = 0;
    /// 下蹲 脚本项ID
    int crouchScriptId = 0;
    /// 蹲着 脚本项ID
    int crouchingScriptId = 0;
    /// 从下蹲站起 脚本项ID
    int standUpScriptId = 0;
    /// 蹲着前行 脚本项ID
    int crouchMoveForwardScriptId = 0;
    /// 蹲着后退 脚本项ID
    int crouchMoveBackwardScriptId = 0;
    /// 站立转身 脚本项ID
    int turnAroundScriptId = 0;
    /// 蹲着转身 脚本项ID
    int crouchTurnAroundScriptId = 0;
    /// 按键站立防御 脚本项ID
    int buttonBlockScriptId = 0;
    /// 按键蹲着防御 脚本项ID
    int buttonCrouchBlockScriptId = 0;
    /// 按键空中防御 脚本项ID
    int buttonAirBlockScriptId = 0;
    /// 开始时 脚本项ID
    int startActionScriptId = 0;
    /// 胜利 脚本项ID
    int victoryScriptId = 0;
    /// 失败 脚本项ID
    int failScriptId = 0;
    /// 平局 脚本项ID
    int drawGameScriptId = 0;
    /// 角色选择段落 脚本项ID
    int portraitScriptId = 0;
    /// 场景段落 脚本项ID
    int stageAvatarScriptId = 0;
    /// 影 脚本项ID
    int shadowScriptId = 0;


    /// 初始化角色基础脚本
    void initBasicScriptInfos();
};

#endif //KGTPLAYER_HPP
