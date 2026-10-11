//
// PlayerCommandMatcher: 出招指令匹配
//

#include "PlayerCommandMatcher.hpp"

#include "engine/Input.hpp"

/// 方向值的便捷别名（using 声明不能用于嵌套类型，这里用类型别名）
using DPad = Input::DPad;

namespace {
/// 指令里的方向要求是否与当前方向值相符。
/// 指令方向沿用 2DFM 的小键盘记法，与 Input::DPad 的 1~9 完全对应。
bool matchesDirection(CommandDirection required, DPad current) {
    switch (required) {
    case CommandDirection::FREE: // 不限方向
        return true;
    case CommandDirection::D5: // 回中
        return current == DPad::Neutral;
    case CommandDirection::D6: // 右
        return current == DPad::Right;
    case CommandDirection::D3: // 右下
        return current == DPad::RightDown;
    case CommandDirection::D2: // 下
        return current == DPad::Down;
    case CommandDirection::D1: // 左下
        return current == DPad::LeftDown;
    case CommandDirection::D4: // 左
        return current == DPad::Left;
    case CommandDirection::D7: // 左上
        return current == DPad::LeftUp;
    case CommandDirection::D8: // 上
        return current == DPad::Up;
    case CommandDirection::D9: // 右上
        return current == DPad::RightUp;
    case CommandDirection::D741: // 左半圈：左上、左、左下
        return current == DPad::LeftUp || current == DPad::Left || current == DPad::LeftDown;
    case CommandDirection::D789: // 上排：左上、上、右上
        return current == DPad::LeftUp || current == DPad::Up || current == DPad::RightUp;
    case CommandDirection::D963: // 右半圈：右上、右、右下
        return current == DPad::RightUp || current == DPad::Right || current == DPad::RightDown;
    case CommandDirection::D123: // 下排：左下、下、右下
        return current == DPad::LeftDown || current == DPad::Down || current == DPad::RightDown;
    }
    return false;
}

/// 某个攻击键这一帧是否按着（含刚按下的那一帧）
bool isAttackDown(Input::AttackButton button) {
    const auto &input = Input::getInstance();
    return input.isAttackPressed(button) || input.isAttackHeld(button);
}

/**
 * 指令项要求的按键是否满足（位 0 = A … 位 5 = F）。
 *
 * 判定为：这些按键**这一帧都按着**，且其中**至少有一个是这一帧新按下的**。
 * 用“都按着 + 有新按下”而不是“同一帧同时新按下”，是为了让组合键（例如 A+C）能真正按出来：
 * 玩家先按住 A，再按 C 时就会命中，而不是要求两个键落在同一帧。
 * 单键指令下两种写法等价。
 */
bool matchesButtons(byte buttons, Input::PlayerIndex player) {
    if (buttons == 0) {
        // 纯方向指令靠方向变化触发，本版先不处理
        return false;
    }
    const auto &input = Input::getInstance();
    bool anyFresh = false;
    for (int i = 0; i < static_cast<int>(Input::AttackButton::Count); ++i) {
        if ((buttons & (1 << i)) == 0) {
            continue;
        }
        const auto button = static_cast<Input::AttackButton>(i);
        if (!isAttackDown(button)) {
            return false;
        }
        if (input.isAttackPressed(button, player)) {
            anyFresh = true;
        }
    }
    return anyFresh;
}
} // namespace

PlayerCommandMatcher::Match PlayerCommandMatcher::match() const {
    Match result;
    if (playerData == nullptr) {
        return result;
    }

    const auto currentDir = Input::getInstance().getDPad(Input::PlayerIndex::P1);
    for (const auto &command : playerData->commands) {
        // 本版只做单指令项
        if (command.steps.size() != 1) {
            continue;
        }
        const auto &step = command.steps.front();
        // 连按 / 蓄力 / 回转先不做
        if (step.mark != CommandStepMark::NORMAL) {
            continue;
        }
        if (!matchesDirection(step.direction, currentDir)) {
            continue;
        }
        if (!matchesButtons(step.buttons, Input::PlayerIndex::P1)) {
            continue;
        }

        // 命中的第一条即结果：越靠前优先级越高
        result.command = &command;
        return result;
    }
    return result;
}
