//
// Created by 厉猛 on 2024-07-16.
//

#ifndef INC_2DFM_PLAYER_INPUTSYSTEM_HPP
#define INC_2DFM_PLAYER_INPUTSYSTEM_HPP

#include <SDL.h>

enum KeyState
{
    NONE,
    PRESSED,
    HELD,
    RELEASED
};

struct KeyboardState
{
    friend class InputSystem;

    KeyState getKeyState(SDL_Scancode keyCode) const;
    bool getKeyValue(SDL_Scancode keyCode) const;
    bool isKeyDown(SDL_Scancode keyCode) const;
    bool isKeyUp(SDL_Scancode keyCode) const;
    bool isKeyHolding(SDL_Scancode keyCode) const;
private:
    Uint8 prevKeyState[SDL_NUM_SCANCODES];
    const Uint8 *currKeyState = nullptr;
};

struct InputState {
    KeyboardState keyboard;
};

class InputSystem {
public:
    bool initialize();
    void cleanUp();
    void preUpdate();
    void update();
    void processInput(const SDL_Event &ev);

    const InputState &getInputState() const { return inputState; }
    const KeyboardState &getKeyboard() const { return inputState.keyboard; }

    KeyState getKeyState(SDL_Scancode keyCode) const { return inputState.keyboard.getKeyState(keyCode); }
    bool getKeyValue(SDL_Scancode keyCode) const { return inputState.keyboard.getKeyValue(keyCode); }
    bool isKeyDown(SDL_Scancode keyCode) const { return inputState.keyboard.isKeyDown(keyCode); }
    bool isKeyUp(SDL_Scancode keyCode) const { return inputState.keyboard.isKeyUp(keyCode); }
    bool isKeyHolding(SDL_Scancode keyCode) const { return inputState.keyboard.isKeyHolding(keyCode); }
private:
    InputState inputState;
};


#endif //INC_2DFM_PLAYER_INPUTSYSTEM_HPP
