//
// Created by limen on 2024/8/1.
//

#include "InputSystem.hpp"
#include <Director.h>
#include <EventDispatcher.h>
#include <EventListenerKeyboard.h>

KeyState KeyboardState::getKeyState(ax::EventKeyboard::KeyCode keyCode) const {
    int code = static_cast<int>(keyCode);
    if (prevKeyState[code]) {
        if (currKeyState[code]) {
            return KeyState::HELD;
        } else {
            return KeyState::RELEASED;
        }
    } else {
        if (currKeyState[code]) {
            return KeyState::PRESSED;
        } else {
            return KeyState::NONE;
        }
    }
}

bool KeyboardState::isKeyDown(ax::EventKeyboard::KeyCode keyCode) const {
    return getKeyState(keyCode) == KeyState::PRESSED;
}

bool KeyboardState::isKeyUp(ax::EventKeyboard::KeyCode keyCode) const {
    return getKeyState(keyCode) == KeyState::RELEASED;
}

bool KeyboardState::isKeyHolding(ax::EventKeyboard::KeyCode keyCode) const {
    return getKeyState(keyCode) == KeyState::HELD;
}

bool KeyboardState::getKeyValue(ax::EventKeyboard::KeyCode keyCode) const {
    return currKeyState[static_cast<int>(keyCode)];
}

InputSystem::InputSystem() {
    initialize();
}

InputSystem::~InputSystem() {
    keyboardListener->release();
}

bool InputSystem::initialize() {
    memset(inputState.keyboard.currKeyState, false, MAX_KEYS);
    memset(inputState.keyboard.prevKeyState, false, MAX_KEYS);

    scheduler = ax::Director::getInstance()->getScheduler();
    scheduler->scheduleUpdate(this, 1, false);

    this->keyboardListener = ax::EventListenerKeyboard::create();
    keyboardListener->onKeyPressed  = AX_CALLBACK_2(InputSystem::onKeyPressed, this);
    keyboardListener->onKeyReleased = AX_CALLBACK_2(InputSystem::onKeyReleased, this);
    keyboardListener->retain();
    ax::Director::getInstance()
            ->getEventDispatcher()
            ->addEventListenerWithFixedPriority(keyboardListener, 1);
    return true;
}

void InputSystem::update(float deltaTime) {
    memcpy(inputState.keyboard.prevKeyState, inputState.keyboard.currKeyState, MAX_KEYS);
}

void InputSystem::onKeyPressed(ax::EventKeyboard::KeyCode code, ax::Event *event) {
    inputState.keyboard.currKeyState[static_cast<int>(code)] = true;
}

void InputSystem::onKeyReleased(ax::EventKeyboard::KeyCode code, ax::Event *event) {
    inputState.keyboard.currKeyState[static_cast<int>(code)] = false;
}

InputSystem *InputSystem::getInstance() {
    static InputSystem inputSystem;
    return &inputSystem;
}
