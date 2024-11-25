#pragma once
#include <math/Vec2.h>

namespace _2dfm {
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

    /// 常规脚本项枚举（基本图形、场景、DEMO用）
    enum class CommonScriptItemTypes {
        /// 初
        START = 0,
        /// 图
        PIC = 12,
        /// 移
        MOVE = 1,
        /// 声
        SOUND = 3,
        /// 物
        OBJECT = 4,
        /// 色
        COLOR = 35,
        /// 变
        VARIABLE = 31,
        /// 随
        RANDOM = 32,
        /// 跳
        JUMP = 10,
        /// 调
        CALL = 11,
        /// 循
        LOOP = 9,
        /// 残
        AFTERIMAGE = 37,
        /// 完
        END = 5
    };

    enum class ColorSetType {
        NORMAL = 0,
        TRANSPARENCY,
        ADD_BLEND,
        MINUS_BLEND,
        ALPHA_BLEND,
    };
#pragma pack(push, 1)
    struct KgtHitSymbol {
        byte type;
        bool placement;
        byte width;
    };

    struct KgtShowTime {
        byte type;
        uint16_t showTime;
    };

    struct KgtPos {
        byte type;
        int16_t x;
        int16_t y;

        ax::Vec2 getPosition() const { return ax::Vec2(x, y); }
    };

    struct KgtTimerPos : KgtPos {
        byte width;
    };

    struct KgtPosAndOffset : KgtPos {
        signed char offsetX;
        signed char offsetY;

        ax::Vec2 getOffset() const { return ax::Vec2(offsetX, offsetY); }
    };

    /// 场景初始脚本
    struct StageStart {
        byte type;
        byte flags;
        /// 水平滚轴值
        int16_t horiScroll;
        /// 垂直滚轴值
        int16_t vertScroll;

        bool isHoriLoop() const {
            return flags & 0x2;
        }
        bool isVertLoop() const {
            return flags & 0x4;
        }
        bool isHoriScroll() const {
            return flags & 0x08;
        }
        bool isVertScroll() const {
            return flags & 0x10;
        }
    };

    struct ShowPic {
        byte type; // 0
        uint16_t keepTime; // 1
        uint16_t idxAndFlip; // 3
        int16_t offsetX; // 5
        int16_t offsetY; // 7
        byte fixDir; // 9

        int getPicIdx() const { return idxAndFlip & 0x3fff; }
        bool isFlipX() const { return (idxAndFlip & 0x4000) != 0; }
        bool isFlipY() const { return (idxAndFlip & 0x8000) != 0;  }
        ax::Vec2 getOffset() const { return ax::Vec2(offsetX, offsetY); }
    };

    struct MoveCmd {
        byte type;
        int16_t accelX;
        int16_t moveX;
        int16_t moveY;
        int16_t accelY;
        byte flags;

        /// 是否为相加模式
        bool isAdd() const {
            return flags & 0x1;
        }
        bool isIgnoreMoveX() const {
            return flags & 0x2;
        }
        bool isIgnoreMoveY() const {
            return flags & 0x4;
        }
        bool isIgnoreAccelX() const {
            return flags & 0x8;
        }
        bool isIgnoreAccelY() const {
            return flags & 0x16;
        }
    };

    struct PlaySoundCmd {
        byte type;
        byte unknown;
        uint16_t soundIdx;
    };

    struct ColorSet {
        byte type;
        byte colorSetType;
        byte red;
        byte green;
        byte blue;
        byte alpha;
    };

    struct JumpCmd {
        byte type;
        uint16_t jumpId;
        uint16_t jumpPos;
    };
#pragma pack(pop)

    struct Unknown {
        byte type;
        byte bytes[15];
    };
}
