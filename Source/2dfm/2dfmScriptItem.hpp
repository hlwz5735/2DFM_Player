#pragma once
#include <math/Vec2.h>

namespace _2dfm {
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

    enum class ColorBlendType {
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

    struct ColorSetCmd {
        byte type;
        byte colorBlendType;
        int8_t red;
        int8_t green;
        int8_t blue;
        int8_t alpha;

        ColorBlendType getColorBlendType() const {
            return static_cast<ColorBlendType>(colorBlendType);
        }
    };

    /// 跳转和调用脚本都复用这个
    struct JumpCmd {
        byte type;
        uint16_t jumpId;
        uint8_t jumpPos;
    };

    struct LoopCmd {
        byte type;
        uint8_t loopCount;
        uint16_t targetScriptId;
        uint8_t targetPos;
    };

    struct RandomCmd {
        byte type;
        uint16_t randomMaxVal;
        uint16_t moreThanVal;
        // 中间有1字节空隙，不知为何
        byte unknownGap;
        uint16_t targetScriptId;
        uint8_t targetPos;
    };

    struct ObjectCmd {
        byte type;
        byte flags;
        uint16_t targetScriptId;
        uint8_t targetPos;
        uint16_t targetScriptIdIfExists;
        uint8_t targetPosIfExists;
        int16_t posX;
        int16_t posY;
        /// 管理号（0-9）
        uint8_t manageNo;
        /// 指定的层级
        int8_t layer;

        /// 拿取图层信息<br>
        /// 一共可以有127层<br>
        /// 70～80号是角色用的图层<br>
        /// >80显示在角色的前面<br>
        /// <70显示在角色的里面
        int getLayer() const {
            switch (flags & 0b0011) {
            case 0:
                return 0;
            case 1:
                return 127;
            case 2:
            default:
                return layer;
            }
        }

        /// 是否挂载为子节点
        bool isAttachAsChild() const {
            return flags & 0b00100000;
        }

        /// 是否无条件出现
        bool isUnconditionally() const {
            return flags & 0b0100;
        }

        /// 是否显示影子
        bool isShowShadow() const {
            return flags & 0b1000;
        }

        /// 是否使用窗口坐标
        bool isUseWindowPosition() const {
            return flags & 0b01000000;
        }

        int getManageNo() const {
            if (isUnconditionally()) {
                return -1;
            } else if (manageNo >= 0 && manageNo < 10) {
                return manageNo;
            }
            return -1;
        }

        ax::Vec2 getPosition() const {
            return ax::Vec2(posX, posY);
        }
    };

    /// 变量条件分歧
    struct VariableCmd {
        /*
         * 变量编号规则：
         * 任务变量A-P: 00-0F
         * 系统变量A-P: 80-8F
         * 角色变量A-P: 40-4F（仅限角色变量脚本）
         * 特定值8个: C0-C7（仅用于条件分歧比较，不可赋值）
         *  C0 X坐标
         *  C1 Y坐标
         *  C2 地图X坐标
         *  C3 地图Y坐标
         *  C4 上级X坐标
         *  C5 上级Y坐标
         *  C6 时间
         *  C7 回合数
         */
        enum class Operation {
            NONE, ASSIGNMENT, ADDITION,
        };
        enum class Condition {
            NONE, EQUALS, LESS, GREATER,
        };
        byte type;
        uint16_t targetScriptId;
        uint8_t targetPos;
        byte targetVariable;
        /// 操作标记位
        byte opFlags;
        /// 参与运算的变量
        byte compareVariable;
        /// 代入或相加的数值
        int16_t operationValue;
        /// 条件分歧比较的数值
        int16_t compareValue;

        Operation getOperation() const {
            int v = opFlags & 0b0011;
            switch (v) {
            case 0:
                return Operation::NONE;
            case 1:
                return Operation::ASSIGNMENT;
            case 2:
            default:
                return Operation::ADDITION;
            }
        }

        bool isCompareWithOtherVariable() const {
            return opFlags & 0b10000000;
        }

        Condition getCondition() const {
            int v = opFlags & 0b00001100;
            switch (v) {
            case 0:
                return Condition::NONE;
            case 4: // 0100b
                return Condition::EQUALS;
            case 8: // 1000b
                return Condition::GREATER;
            case 12: // 1100b
            default:
                return Condition::LESS;
            }
        }
    };

    struct AfterimageCmd {
        enum class AfterimageColorType {
            /// 不对残影施加任何颜色效果（包括半透明颜色指定），但50%半透和加法/减法混合不受影响
            NONE,
            /// 残影使用固定的颜色+半透明效果
            FIXED,
            /// 按残影个数和时间间隔计算残影存活时间，然后按时间线性插值过渡颜色
            LINEAR_SMOOTH,
            /// 整体趋势和线性平滑相同，但每隔一帧消失一次（模拟半透明）
            LINEAR_BLINK,
            /// 根据颜色分项的方向，每帧进行一次随机取值，如果没有指定透明度，则透明度不参与随机
            RANDOM,
        };

        byte type;
        uint16_t unknownGap;
        /// 最大残影个数，残影数量超过则销毁最早的残影
        uint8_t afterimageMaxCount;
        /// 间隔，0-255，每隔多少时间（百分之一秒）生成一个残影
        uint8_t afterimageGap;
        byte colorBlendType;
        /// 残像颜色模式
        byte afterimageColorType;
        int8_t red;
        int8_t green;
        int8_t blue;
        int8_t alpha;

        ColorBlendType getColorBlendType() const {
            return static_cast<ColorBlendType>(colorBlendType);
        }

        AfterimageColorType getAfterimageColorType() const {
            return static_cast<AfterimageColorType>(afterimageColorType);
        }
    };
#pragma pack(pop)

    struct Unknown {
        byte type;
        byte bytes[15];
    };
}
