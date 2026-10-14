#pragma once
#include <math/Vec2.h>

namespace _2dfm {
    /// 脚本项类型枚举
    /// 命名规则：条件分歧类统一以 _BRANCH 结尾；设置 / 改变某物用名词短语；
    ///           控制流沿用 JUMP / CALL / LOOP 等传统叫法。取值即文件里的原始 type。
    enum class CommonScriptItemTypes {
        /// 初 —— 初始设置：场景用来设置视差效果与是否启用循环模式；角色用来设置当前动作脚本的招式级别
        START = 0,
        /// 图 —— 显示图像并等待
        SHOW_PIC = 12,
        /// 移 —— 使实体产生移动（或停止移动）
        MOVE = 1,
        /// 声 —— 播放声音
        PLAY_SOUND = 3,
        /// 物 —— 出现物体，用于角色实现飞行道具等
        SPAWN_OBJECT = 4,
        /// 色 —— 指定当前实体的颜色叠加或透明度
        COLOR = 35,
        /// 变 —— 变量分歧：对任务 / 角色 / 系统级变量赋值或条件判断，满足条件则跳转别处，不满足继续向后执行
        VARIABLE_BRANCH = 31,
        /// 随 —— 随机分歧：生成一个随机数并判断，满足随机条件则跳转，不满足继续向后执行
        RANDOM_BRANCH = 32,
        /// 跳 —— 无条件把当前执行的脚本序号改到别处；角色只有走到脚本末尾或显式的“完”才算动作最终完成
        JUMP = 10,
        /// 调 —— 当前执行现场入栈，转到指定位置执行，执行完弹回当前栈继续（栈底弹出后动作才算最终完成）
        CALL = 11,
        /// 循 —— 现场入栈并转到指定位置，弹栈时若还剩循环次数则再次进入，直至次数用完；循环无法被打断
        LOOP = 9,
        /// 残 —— 残像设置，多用于超必杀期间留下残影
        AFTERIMAGE = 37,
        /// 完 —— 显式声明当前脚本执行到此结束（4 类文件具体执行的动作不同）
        END = 5,

        // --- 以下为 Player 专属 ---

        /// 防 —— 受击框设置：设置角色当前的受伤包围盒、碰撞包围盒
        HURT_BOX = 25,
        /// 攻 —— 攻击框设置（attack box set）
        ATTACK_BOX = 23,
        /// 敌 —— 对方的受击动作设置：攻击命中后，对方播放哪个受击动作（中招反应）
        ENEMY_HURT_ACTION = 24,
        /// 销 —— 动作取消条件的设置
        CANCEL = 30,
        /// 招 —— 招式分歧：招式过程中持续判定，满足条件（着地、撞墙、攻击被防御/抵消等）则跳转
        MOVE_BRANCH = 2,
        /// 动 —— 行动分歧：以角色的基本行动状态为条件跳转（跟随角色的“物”等也按此改变）
        ACTION_BRANCH = 22,
        /// 令 —— 指令输入分歧：只判定当前时刻输入缓冲能否匹配上某条指令，匹配则跳转
        COMMAND_BRANCH = 36,
        /// 改 —— 对方的投掷动作设置：改变对手的图像和位置（投掷反应，需配合“控”才能解除）
        ENEMY_THROW_ACTION = 7,
        /// 控 —— 强制控制对手进入指定的受击脚本 / 位置，恢复其控制权
        CONTROL_OPPONENT = 20,
        /// 气 —— 必杀豆（气点）分歧：增减气点，满足条件则继续执行，否则跳转
        SP_BRANCH = 16,
        /// 血 —— 血槽分歧：当前血量与给定值比较，满足则向后执行，否则跳转
        HP_BRANCH = 17,
        /// 槽 —— 增减生命槽 / 气槽的数值（可同时作用于自己和对方）
        GAUGE_CHANGE = 21,
        /// 效 —— 视觉效果设置：调色盘闪烁（无视独立调色盘）、屏幕摇晃（震屏）
        EFFECT = 14,
        /// 停 —— 时间冻结：延长己方或对方的等待时间（对角色创建的“物”不生效，2DFM 的 BUG）
        TIME_FREEZE = 26
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

    /// “招”（招式分歧，仅角色文件使用）：招式过程中持续监听条件，满足则跳转。
    ///
    /// 生命周期：脚本里下了这条命令就建立监听，范围是当前这一轮脚本执行
    /// （含“跳 / 调 / 循”等跳转），直到最外层脚本跑到末尾或遇到显式的“完”才清空。
    /// 解释器侧见 ScriptInterceptorComponent::interceptMoveBranchCmd / notifyMoveBranch。
    ///
    /// 字段布局已在真实 .player 上核对（角色 1.player 14 项、DONGDONG.player 101 项）：
    /// 条件字节在偏移 1，取值全部落在 0-6；偏移 4-5 是**目标脚本内的偏移**
    /// （相对该脚本起点，与“跳 / 调”的 jumpPos 同义），不是 scriptItems 的全局下标 ——
    /// 按全局下标只有 3/14、12/101 自洽，按段内偏移则全部自洽
    /// （含偏移恰等于脚本长度，含义是直接跳到该段末尾）。
    struct MoveBranchCmd {
        /// 生效条件
        enum class Cond {
            /// 无
            NONE = 0,
            /// 着地后
            LANDED = 1,
            /// 攻击命中时
            ATTACK_HIT = 2,
            /// 攻击被防御时
            ATTACK_GUARDED = 3,
            /// 碰到墙壁时
            HIT_WALL = 4,
            /// 攻击互抵时
            ATTACK_CLASH = 5,
            /// 被投掷时受攻击
            THROWN_HIT = 6,
        };

        byte type;
        /// 生效条件，取值为 Cond
        byte condition;
        /// 目标脚本下标（脚本条目）
        uint16_t targetScriptId;
        /// 目标脚本内的偏移（脚本项；相对该脚本起点，等于脚本长度表示该段直接结束）
        uint16_t targetPos;

        Cond getCondition() const {
            return static_cast<Cond>(condition);
        }
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
