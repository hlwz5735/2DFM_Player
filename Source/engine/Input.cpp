//
// Created by limen on 2024/11/23.
//

#include "Input.hpp"
#include <axmol.h>

USING_NS_AX;

bool Input::init() {
    using KeyCode = EventKeyboard::KeyCode;

    keyMapping = {
        std::pair(KeyCode::KEY_W, GameButton::D_UP),      std::pair(KeyCode::KEY_S, GameButton::D_DOWN),
        std::pair(KeyCode::KEY_A, GameButton::D_LEFT),    std::pair(KeyCode::KEY_D, GameButton::D_RIGHT),
        std::pair(KeyCode::KEY_J, GameButton::A),         std::pair(KeyCode::KEY_U, GameButton::B),
        std::pair(KeyCode::KEY_K, GameButton::C),         std::pair(KeyCode::KEY_I, GameButton::D),
        std::pair(KeyCode::KEY_O, GameButton::E),         std::pair(KeyCode::KEY_L, GameButton::F),
        std::pair(KeyCode::KEY_ENTER, GameButton::START), std::pair(KeyCode::KEY_ESCAPE, GameButton::MENU),
    };

    keyboardListener = EventListenerKeyboard::create();
    keyboardListener->onKeyPressed = AX_CALLBACK_2(Input::onKeyPressed, this);
    keyboardListener->onKeyReleased = AX_CALLBACK_2(Input::onKeyReleased, this);
    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(keyboardListener, 11);
    keyboardListener->retain();
    Director::getInstance()->getScheduler()->scheduleUpdate(this, 2, false);

    return true;
}

void Input::cleanup() {
    Director::getInstance()->getScheduler()->unscheduleUpdate(this);
    Director::getInstance()->getEventDispatcher()->removeEventListener(keyboardListener);
    keyboardListener->release();
}

void Input::update(float delta) {
    prevButtonStates = currentButtonStates;
}

Input::DPadDir Input::getDPad() const {
    int vert = 0;
    int hori = 0;
    if (currentButtonStates[static_cast<int>(GameButton::D_UP)]) {
        vert += 1;
    }
    if (currentButtonStates[static_cast<int>(GameButton::D_DOWN)]) {
        vert -= 1;
    }
    if (currentButtonStates[static_cast<int>(GameButton::D_LEFT)]) {
        hori -= 1;
    }
    if (currentButtonStates[static_cast<int>(GameButton::D_RIGHT)]) {
        hori += 1;
    }
    if (vert == 0 && hori == 0) {
        return DPadDir::NONE;
    }
    if (vert == 0) {
        return hori > 0 ? DPadDir::RIGHT : DPadDir::LEFT;
    }
    if (hori == 0) {
        return vert > 0 ? DPadDir::UP : DPadDir::DOWN;
    }
    if (hori > 0 && vert > 0) {
        return DPadDir::RIGHT_UP;
    }
    if (hori > 0) {
        return DPadDir::RIGHT_DOWN;
    }
    if (vert > 0) {
        return DPadDir::LEFT_UP;
    }
    return DPadDir::LEFT_DOWN;
}

bool Input::isButtonDown(GameButton button) const {
    const int idx = static_cast<int>(button);
    return currentButtonStates[idx] && !prevButtonStates[idx];
}
bool Input::isAnyAttackButtonDown() const {
    for (auto btn = static_cast<int>(GameButton::A); btn < static_cast<int>(Input::GameButton::F); ++btn) {
        if (isButtonDown(static_cast<GameButton>(btn))) {
            return true;
        }
    }
    return false;
}
bool Input::isButtonUp(GameButton button) const {
    const int idx = static_cast<int>(button);
    return !currentButtonStates[idx] && prevButtonStates[idx];
}
bool Input::isButtonHolding(GameButton button) const {
    const int idx = static_cast<int>(button);
    return currentButtonStates[idx] && prevButtonStates[idx];
}
bool Input::isButtonNotPressed(GameButton button) const {
    const int idx = static_cast<int>(button);
    return !currentButtonStates[idx] && !prevButtonStates[idx];
}

void Input::onKeyPressed(EventKeyboard::KeyCode code, Event *event) {
    if (keyMapping.contains(code)) {
        const auto btn = keyMapping[code];
        currentButtonStates[static_cast<int>(btn)] = true;
    }
}

void Input::onKeyReleased(EventKeyboard::KeyCode code, Event *event) {
    if (keyMapping.contains(code)) {
        const auto btn = keyMapping[code];
        currentButtonStates[static_cast<int>(btn)] = false;
    }
}
