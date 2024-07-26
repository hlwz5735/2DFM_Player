//
// Created by 厉猛 on 2024-07-16.
//

#include <cstring>
#include "InputSystem.hpp"

KeyState KeyboardState::getKeyState(SDL_Scancode keyCode) const {
    if (prevKeyState[keyCode]) {
        if (currKeyState[keyCode]) {
            return KeyState::HELD;
        } else {
            return KeyState::RELEASED;
        }
    } else {
        if (currKeyState[keyCode]) {
            return KeyState::PRESSED;
        } else {
            return KeyState::NONE;
        }
    }
}

bool KeyboardState::isKeyDown(SDL_Scancode keyCode) const {
    return getKeyState(keyCode) == KeyState::PRESSED;
}

bool KeyboardState::isKeyUp(SDL_Scancode keyCode) const {
    return getKeyState(keyCode) == KeyState::RELEASED;
}

bool KeyboardState::isKeyHolding(SDL_Scancode keyCode) const {
    return getKeyState(keyCode) == KeyState::HELD;
}

bool KeyboardState::getKeyValue(SDL_Scancode keyCode) const {
    return currKeyState[keyCode];
}

bool InputSystem::initialize() {
    inputState.keyboard.currKeyState = SDL_GetKeyboardState(nullptr);
    memset(inputState.keyboard.prevKeyState, false, SDL_NUM_SCANCODES);

    return true;
}

void InputSystem::cleanUp() {}

void InputSystem::preUpdate() {
    memcpy(inputState.keyboard.prevKeyState, inputState.keyboard.currKeyState, SDL_NUM_SCANCODES);
}

void InputSystem::update() {
}

void InputSystem::processInput(const SDL_Event &ev) {
    switch (ev.type) {
        default:
            break;
    }
}
