//
// Created by 厉猛 on 2024-07-16.
//

#ifndef INC_2DFM_PLAYER_GAME_HPP
#define INC_2DFM_PLAYER_GAME_HPP

#include <SDL.h>

class Game {
public:
    bool initialize();
    void cleanup();

    void loadData();

    void runLoop();

    static Game *getInstance();
protected:
    void processInput();
    void updateGame();
    void generateOutput();
private:
    static Game *INSTANCE;

    bool isRunning = true;
    class Renderer *renderer;
    class AudioSystem *audioSystem;
    class InputSystem *inputSystem;

    Uint64 ticksCount;
};

#endif //INC_2DFM_PLAYER_GAME_HPP
