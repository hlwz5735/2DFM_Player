//
// Created by 厉猛 on 2024-07-16.
//

#include <cstring>
#include "InputSystem.hpp"

KeyState KeyboardState::getKeyState(SDL_Scancode keyCode) const {
    if (prevKeyState[keyCode]) {
        if (currKeyState[keyCode]) {
            return HELD;
        } else {
            return RELEASED;
        }
    } else {
        if (currKeyState[keyCode]) {
            return PRESSED;
        } else {
            return NONE;
        }
    }
}

bool KeyboardState::isKeyDown(SDL_Scancode keyCode) const {
    return getKeyState(keyCode) == PRESSED;
}

bool KeyboardState::isKeyUp(SDL_Scancode keyCode) const {
    return getKeyState(keyCode) == RELEASED;
}

bool KeyboardState::isKeyHolding(SDL_Scancode keyCode) const {
    return getKeyState(keyCode) == HELD;
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
