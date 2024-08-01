//
// Created by limen on 2024/8/1.
//

#ifndef INPUTSYSTEM_HPP
#define INPUTSYSTEM_HPP

#include <EventKeyboard.h>
#include <Object.h>
#include <Scheduler.h>

constexpr int MAX_KEYS = 200;

enum class KeyState
{
    NONE,
    PRESSED,
    HELD,
    RELEASED
};

struct KeyboardState
{
    friend class InputSystem;

    KeyState getKeyState(ax::EventKeyboard::KeyCode keyCode) const;
    bool getKeyValue(ax::EventKeyboard::KeyCode keyCode) const;
    bool isKeyDown(ax::EventKeyboard::KeyCode keyCode) const;
    bool isKeyUp(ax::EventKeyboard::KeyCode keyCode) const;
    bool isKeyHolding(ax::EventKeyboard::KeyCode keyCode) const;
private:
    bool prevKeyState[MAX_KEYS] = {false};
    bool currKeyState[MAX_KEYS] = {false};
};

struct InputState {
    KeyboardState keyboard;
};

class InputSystem : public ax::Object {
public:
    InputSystem(const InputSystem &o) = delete;
    InputSystem &operator=(const InputSystem &o) = delete;

    bool initialize();
    void update(float deltaTime);

    // Keyboard
    void onKeyPressed(ax::EventKeyboard::KeyCode code, ax::Event* event);
    void onKeyReleased(ax::EventKeyboard::KeyCode code, ax::Event* event);

    static InputSystem *getInstance();

    const InputState &getInputState() const { return inputState; }
protected:
    InputSystem();
    ~InputSystem() override;
private:
    InputState inputState;

    ax::Scheduler *scheduler = nullptr;
    class ax::EventListenerKeyboard *keyboardListener = nullptr;
};

#endif //INPUTSYSTEM_HPP
