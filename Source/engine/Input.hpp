//
// Created by limen on 2024/11/23.
//
#pragma once

#include <array>
#include <unordered_map>
#include "Singleton.hpp"

#include <EventKeyboard.h>
#include <EventListenerKeyboard.h>

class Input final : public Singleton<Input> {
public:
    enum class DPadDir {
        LEFT_DOWN = 1,
        DOWN = 2,
        RIGHT_DOWN = 3,
        LEFT = 4,
        NONE = 5,
        RIGHT = 6,
        LEFT_UP = 7,
        UP = 8,
        RIGHT_UP = 9,
    };

    enum class GameButton {
        D_UP,
        D_DOWN,
        D_LEFT,
        D_RIGHT,
        A,
        B,
        C,
        D,
        E,
        F,
        START,
        MENU,
        MAX_BUTTON_COUNT,
    };

    bool init();
    void cleanup();
    void update(float delta);

    /// 摇杆方向
    DPadDir getDPad() const;
    bool isButtonDown(GameButton button) const;
    bool isAnyAttackButtonDown() const;
    bool isButtonUp(GameButton button) const;
    bool isButtonHolding(GameButton button) const;
    bool isButtonNotPressed(GameButton button) const;
private:
    // Keyboard
    void onKeyPressed(ax::EventKeyboard::KeyCode code, ax::Event *event);
    void onKeyReleased(ax::EventKeyboard::KeyCode code, ax::Event *event);

    ax::EventListenerKeyboard *keyboardListener = nullptr;

    std::array<bool, static_cast<int>(GameButton::MAX_BUTTON_COUNT)> currentButtonStates {};
    std::array<bool, static_cast<int>(GameButton::MAX_BUTTON_COUNT)> prevButtonStates {};
    std::unordered_map<ax::EventKeyboard::KeyCode, GameButton> keyMapping;
};
