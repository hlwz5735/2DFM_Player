//
// Created by limen on 2024/11/23.
//

#include "Input.hpp"
#include <axmol.h>

USING_NS_AX;

namespace {
/// 方向键 → DPadAxis 的便捷写法
constexpr uint8_t axisCode(Input::DPadAxis axis) {
    return static_cast<uint8_t>(axis);
}
constexpr uint8_t attackCode(Input::AttackButton button) {
    return static_cast<uint8_t>(button);
}
constexpr uint8_t buttonCode(Input::Button button) {
    return static_cast<uint8_t>(button);
}
} // namespace

bool Input::init() {
    using KeyCode = EventKeyboard::KeyCode;
    using Kind = KeyBinding::Kind;
    constexpr auto P1 = PlayerIndex::P1;

    // 1P 键位（见 InputRefactorPlan.md）：
    //   上/下/左/右 = W/S/A/D，攻击 A~F = J/U/K/I/O/L，开始 = 数字 1，暂停 = ESC
    // 2P 以后在这里追加一份 player = P2 的绑定即可，状态格子已经留好。
    keyMapping = {
        {KeyCode::KEY_W, {Kind::Dir, P1, axisCode(DPadAxis::Up)}},
        {KeyCode::KEY_S, {Kind::Dir, P1, axisCode(DPadAxis::Down)}},
        {KeyCode::KEY_A, {Kind::Dir, P1, axisCode(DPadAxis::Left)}},
        {KeyCode::KEY_D, {Kind::Dir, P1, axisCode(DPadAxis::Right)}},

        {KeyCode::KEY_J, {Kind::Attack, P1, attackCode(AttackButton::A)}},
        {KeyCode::KEY_U, {Kind::Attack, P1, attackCode(AttackButton::B)}},
        {KeyCode::KEY_K, {Kind::Attack, P1, attackCode(AttackButton::C)}},
        {KeyCode::KEY_I, {Kind::Attack, P1, attackCode(AttackButton::D)}},
        {KeyCode::KEY_O, {Kind::Attack, P1, attackCode(AttackButton::E)}},
        {KeyCode::KEY_L, {Kind::Attack, P1, attackCode(AttackButton::F)}},

        {KeyCode::KEY_1, {Kind::Button, P1, buttonCode(Button::Start)}},
        {KeyCode::KEY_ESCAPE, {Kind::Button, P1, buttonCode(Button::Pause)}},
    };

    keyboardListener = EventListenerKeyboard::create();
    keyboardListener->onKeyPressed = AX_CALLBACK_2(Input::onKeyPressed, this);
    keyboardListener->onKeyReleased = AX_CALLBACK_2(Input::onKeyReleased, this);
    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(keyboardListener, 11);
    keyboardListener->retain();
    // 优先级 2：晚于场景 / 节点（优先级 0）读完输入再翻页，边沿判定才成立
    Director::getInstance()->getScheduler()->scheduleUpdate(this, 2, false);

    return true;
}

void Input::cleanup() {
    Director::getInstance()->getScheduler()->unscheduleUpdate(this);
    // 允许重复清理：清过一次就把监听器指针置空
    if (keyboardListener != nullptr) {
        Director::getInstance()->getEventDispatcher()->removeEventListener(keyboardListener);
        keyboardListener->release();
        keyboardListener = nullptr;
    }
}

void Input::update(float delta) {
    for (auto &controller : controllers) {
        // 翻页：本帧成为“上一帧”
        controller.prevDirs = controller.dirs;
        controller.prevAttacks = controller.attacks;
        controller.prevButtons = controller.buttons;

        // 按住时长（攻击键 / 开始 / 暂停）：松手归零，按着就累计
        for (size_t i = 0; i < controller.attacks.size(); ++i) {
            controller.attackHeldTime[i] = controller.attacks[i] ? controller.attackHeldTime[i] + delta : 0.f;
        }
        for (size_t i = 0; i < controller.buttons.size(); ++i) {
            controller.buttonHeldTime[i] = controller.buttons[i] ? controller.buttonHeldTime[i] + delta : 0.f;
        }
    }
}

// ─────────────── D-Pad ───────────────

Input::DPad Input::getDPad(PlayerIndex player) const {
    const auto &dirs = stateOf(player).dirs;
    const int vert = (dirs[static_cast<int>(DPadAxis::Up)] ? 1 : 0)
                     - (dirs[static_cast<int>(DPadAxis::Down)] ? 1 : 0);
    const int hori = (dirs[static_cast<int>(DPadAxis::Right)] ? 1 : 0)
                     - (dirs[static_cast<int>(DPadAxis::Left)] ? 1 : 0);

    if (vert == 0 && hori == 0) {
        return DPad::Neutral;
    }
    if (vert == 0) {
        return hori > 0 ? DPad::Right : DPad::Left;
    }
    if (hori == 0) {
        return vert > 0 ? DPad::Up : DPad::Down;
    }
    if (hori > 0) {
        return vert > 0 ? DPad::RightUp : DPad::RightDown;
    }
    return vert > 0 ? DPad::LeftUp : DPad::LeftDown;
}

bool Input::isDirPressed(DPadAxis axis, PlayerIndex player) const {
    const auto &state = stateOf(player);
    const auto i = static_cast<size_t>(axis);
    return state.dirs[i] && !state.prevDirs[i];
}

bool Input::isDirHeld(DPadAxis axis, PlayerIndex player) const {
    const auto &state = stateOf(player);
    const auto i = static_cast<size_t>(axis);
    return state.dirs[i] && state.prevDirs[i];
}

bool Input::isDirReleased(DPadAxis axis, PlayerIndex player) const {
    const auto &state = stateOf(player);
    const auto i = static_cast<size_t>(axis);
    return !state.dirs[i] && state.prevDirs[i];
}

bool Input::isUpDirection(DPad dir) {
    return dir == DPad::Up || dir == DPad::LeftUp || dir == DPad::RightUp;
}

bool Input::isDownDirection(DPad dir) {
    return dir == DPad::Down || dir == DPad::LeftDown || dir == DPad::RightDown;
}

bool Input::isLeftDirection(DPad dir) {
    return dir == DPad::Left || dir == DPad::LeftDown || dir == DPad::LeftUp;
}

bool Input::isRightDirection(DPad dir) {
    return dir == DPad::Right || dir == DPad::RightDown || dir == DPad::RightUp;
}

// ─────────────── 攻击键 ───────────────

bool Input::isAttackPressed(AttackButton button, PlayerIndex player) const {
    const auto &state = stateOf(player);
    const auto i = static_cast<size_t>(button);
    return state.attacks[i] && !state.prevAttacks[i];
}

bool Input::isAttackHeld(AttackButton button, PlayerIndex player) const {
    const auto &state = stateOf(player);
    const auto i = static_cast<size_t>(button);
    return state.attacks[i] && state.prevAttacks[i];
}

bool Input::isAttackReleased(AttackButton button, PlayerIndex player) const {
    const auto &state = stateOf(player);
    const auto i = static_cast<size_t>(button);
    return !state.attacks[i] && state.prevAttacks[i];
}

bool Input::isAnyAttackPressed(PlayerIndex player) const {
    for (int i = 0; i < attackCount; ++i) {
        if (isAttackPressed(static_cast<AttackButton>(i), player)) {
            return true;
        }
    }
    return false;
}

float Input::getAttackHeldTime(AttackButton button, PlayerIndex player) const {
    return stateOf(player).attackHeldTime[static_cast<size_t>(button)];
}

// ─────────────── 开始 / 暂停 ───────────────

bool Input::isButtonPressed(Button button, PlayerIndex player) const {
    const auto &state = stateOf(player);
    const auto i = static_cast<size_t>(button);
    return state.buttons[i] && !state.prevButtons[i];
}

bool Input::isButtonHeld(Button button, PlayerIndex player) const {
    const auto &state = stateOf(player);
    const auto i = static_cast<size_t>(button);
    return state.buttons[i] && state.prevButtons[i];
}

bool Input::isButtonReleased(Button button, PlayerIndex player) const {
    const auto &state = stateOf(player);
    const auto i = static_cast<size_t>(button);
    return !state.buttons[i] && state.prevButtons[i];
}

float Input::getButtonHeldTime(Button button, PlayerIndex player) const {
    return stateOf(player).buttonHeldTime[static_cast<size_t>(button)];
}

// ─────────────── 事件 ───────────────

void Input::applyKey(const KeyBinding &binding, bool down) {
    auto &state = stateOf(binding.player);
    switch (binding.kind) {
    case KeyBinding::Kind::Dir:
        state.dirs[binding.code] = down;
        break;
    case KeyBinding::Kind::Attack:
        state.attacks[binding.code] = down;
        break;
    case KeyBinding::Kind::Button:
        state.buttons[binding.code] = down;
        break;
    }
}

void Input::onKeyPressed(EventKeyboard::KeyCode code, Event *event) {
    if (const auto it = keyMapping.find(code); it != keyMapping.end()) {
        applyKey(it->second, true);
    }
}

void Input::onKeyReleased(EventKeyboard::KeyCode code, Event *event) {
    if (const auto it = keyMapping.find(code); it != keyMapping.end()) {
        applyKey(it->second, false);
    }
}
