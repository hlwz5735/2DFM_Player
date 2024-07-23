//
// Created by 厉猛 on 2024-07-16.
//

#ifndef INC_2DFM_PLAYER_GAME_HPP
#define INC_2DFM_PLAYER_GAME_HPP

#include <SDL.h>
#include "../2dfm/KgtGame.hpp"
#include "../game/GameConfig.hpp"
#include <vector>

class Node;
class Renderer;
class AudioSystem;
class InputSystem;

class Game {
public:
    friend class Renderer;

    Game();

    bool initialize();
    void cleanUp();
    void runLoop();

    void loadData();

    static Game *getInstance();
    Renderer *getRenderer() const { return renderer; }
    AudioSystem *getAudioSystem() const { return audioSystem; }
    InputSystem *getInputSystem() const { return inputSystem; }
    const GameConfig *getGameConfig() const { return &gameConfig; }

    void addGameObject(Node *obj);
    void removeGameObject(Node *obj);
protected:
    void processInput();
    void updateGame();
    void generateOutput();
private:
    static Game *INSTANCE;

    bool isRunning = true;
    Uint64 ticksCount;

    Renderer *renderer = nullptr;
    AudioSystem *audioSystem = nullptr;
    InputSystem *inputSystem = nullptr;

    std::vector<Node *> gameObjects;
    std::vector<Node *> pendingObjects;
    bool updatingGameObjects = false;

    KgtGame kgt;
    GameConfig gameConfig;
};

#endif //INC_2DFM_PLAYER_GAME_HPP
