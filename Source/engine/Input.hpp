//
// Created by limen on 2024/11/23.
//
#pragma once

// 已按 InputRefactorPlan.md 完成 1P 重构：最多 2 个控制器、每个控制器保存
// “上一帧 / 当前帧”两份状态，对外只暴露 按下 / 按住 / 抬起 三种判定。
// 2P 目前只是把状态格子留着（全部回中），键位等以后要用时再接。

#include <array>
#include <cstdint>
#include <unordered_map>

#include "Singleton.hpp"

#include <EventKeyboard.h>
#include <EventListenerKeyboard.h>

/**
 * 输入。
 *
 * 每个控制器独占一份状态，包含：
 *   - D-Pad：4 个方向键（上/下/左/右）的按下情况，组合成 1~9 的方向值（小键盘顺序）
 *   - 攻击键：A~F 共 6 个
 *   - 开始 / 暂停
 * 每份状态都保存“上一帧”与“当前帧”，因此按下/按住/抬起都是查表判定，不再各自记 prev。
 *
 * 注意翻页时序：update() 由调度器以优先级 2 驱动，而场景 / 节点是优先级 0，
 * 所以每帧次序是“节点先读输入 → update() 再翻页”，读到的“上一帧”才是真正的上一帧。
 * 任何地方都不要再手动调一次 update()。
 */
class Input final : public Singleton<Input> {
public:
    /// 控制器数量上限（目前只接了 1P 的键盘）
    static constexpr int maxControllerCount = 2;

    /// 控制器
    enum class PlayerIndex : uint8_t {
        P1 = 0,
        P2 = 1,
    };

    /// D-Pad 方向值，按小键盘顺序
    enum class DPad : uint8_t {
        LeftDown = 1,  // 1 左下
        Down = 2,      // 2 下
        RightDown = 3, // 3 右下
        Left = 4,      // 4 左
        Neutral = 5,   // 5 回中
        Right = 6,     // 6 右
        LeftUp = 7,    // 7 左上
        Up = 8,        // 8 上
        RightUp = 9,   // 9 右上
    };

    /// D-Pad 的四个分量，同时也对应四个方向键
    enum class DPadAxis : uint8_t {
        Up = 0,
        Down,
        Left,
        Right,
        Count,
    };

    /// 攻击键 A~F
    enum class AttackButton : uint8_t {
        A = 0,
        B,
        C,
        D,
        E,
        F,
        Count,
    };

    /// 开始 / 暂停
    enum class Button : uint8_t {
        Start = 0,
        Pause,
        Count,
    };

    bool init();
    void cleanup();
    void update(float delta);

    // ─────────────── D-Pad ───────────────

    /// 当前方向值（1~9）。上下同按、左右同按都会抵消成回中。
    DPad getDPad(PlayerIndex player = PlayerIndex::P1) const;

    /// 某个方向分量这一帧是否“新按下”（上一帧没按、这一帧按了）
    bool isDirPressed(DPadAxis axis, PlayerIndex player = PlayerIndex::P1) const;
    /** 某个方向分量是否“按住”（上一帧按着、这一帧还按着）。
     *  “按住不放”的完整语义是 isDirPressed(axis) || isDirHeld(axis)（= 这一帧确实按着）。*/
    bool isDirHeld(DPadAxis axis, PlayerIndex player = PlayerIndex::P1) const;
    /// 某个方向分量是否“刚抬起”（上一帧按着、这一帧松开）
    bool isDirReleased(DPadAxis axis, PlayerIndex player = PlayerIndex::P1) const;

    /// 方向值里是否含“上 / 下 / 左 / 右”分量（左上、右上等都算）
    static bool isUpDirection(DPad dir);
    static bool isDownDirection(DPad dir);
    static bool isLeftDirection(DPad dir);
    static bool isRightDirection(DPad dir);

    // ─────────────── 攻击键 ───────────────

    bool isAttackPressed(AttackButton button, PlayerIndex player = PlayerIndex::P1) const;
    bool isAttackHeld(AttackButton button, PlayerIndex player = PlayerIndex::P1) const;
    bool isAttackReleased(AttackButton button, PlayerIndex player = PlayerIndex::P1) const;
    /// 任意攻击键“新按下”（菜单确认之类场合用）
    bool isAnyAttackPressed(PlayerIndex player = PlayerIndex::P1) const;
    /// 已经按住多久（秒）；没按住时为 0。留给以后的蓄力类指令用。
    float getAttackHeldTime(AttackButton button, PlayerIndex player = PlayerIndex::P1) const;

    // ─────────────── 开始 / 暂停 ───────────────

    bool isButtonPressed(Button button, PlayerIndex player = PlayerIndex::P1) const;
    bool isButtonHeld(Button button, PlayerIndex player = PlayerIndex::P1) const;
    bool isButtonReleased(Button button, PlayerIndex player = PlayerIndex::P1) const;
    float getButtonHeldTime(Button button, PlayerIndex player = PlayerIndex::P1) const;

private:
    static constexpr int axisCount = static_cast<int>(DPadAxis::Count);
    static constexpr int attackCount = static_cast<int>(AttackButton::Count);
    static constexpr int buttonCount = static_cast<int>(Button::Count);

    /// 一个控制器的全部状态：当前帧 + 上一帧
    struct ControllerState {
        std::array<bool, axisCount> dirs{};
        std::array<bool, axisCount> prevDirs{};

        std::array<bool, attackCount> attacks{};
        std::array<bool, attackCount> prevAttacks{};
        std::array<float, attackCount> attackHeldTime{};

        std::array<bool, buttonCount> buttons{};
        std::array<bool, buttonCount> prevButtons{};
        std::array<float, buttonCount> buttonHeldTime{};
    };

    /// 键盘按键绑到“哪个控制器的哪个动作”
    struct KeyBinding {
        enum class Kind : uint8_t {
            Dir,
            Attack,
            Button,
        };

        Kind kind = Kind::Dir;
        PlayerIndex player = PlayerIndex::P1;
        /// Dir 时是 DPadAxis，Attack 时是 AttackButton，Button 时是 Button
        uint8_t code = 0;
    };

    void onKeyPressed(ax::EventKeyboard::KeyCode code, ax::Event *event);
    void onKeyReleased(ax::EventKeyboard::KeyCode code, ax::Event *event);
    /// 把一次按下/松开写进对应控制器的“当前帧”
    void applyKey(const KeyBinding &binding, bool down);

    ControllerState &stateOf(PlayerIndex player) { return controllers[static_cast<int>(player)]; }
    const ControllerState &stateOf(PlayerIndex player) const { return controllers[static_cast<int>(player)]; }

    std::array<ControllerState, maxControllerCount> controllers{};
    std::unordered_map<ax::EventKeyboard::KeyCode, KeyBinding> keyMapping;

    ax::EventListenerKeyboard *keyboardListener = nullptr;
};
