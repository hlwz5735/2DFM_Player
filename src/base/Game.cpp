//
// Created by 厉猛 on 2024-07-16.
//

#include <SDL_image.h>
#include <SDL_mixer.h>
#include "Game.hpp"
#include "Renderer.hpp"

const char *title = "Game";
Game *Game::INSTANCE = nullptr;

bool Game::initialize() {
    if (SDL_Init(SDL_INIT_VIDEO
                 | SDL_INIT_AUDIO
                 | SDL_INIT_GAMECONTROLLER) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }
    if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) == 0) {
        SDL_Log("Unable to initialize SDL_image: %s", SDL_GetError());
        return false;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Sdl_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        SDL_Quit();
        return -1;
    }

    renderer = new Renderer();
    renderer->initialize(nullptr, 0, 0);

    loadData();

    ticksCount = SDL_GetTicks64();
    INSTANCE = this;
    return true;
}

void Game::cleanup() {
    Mix_CloseAudio();
    renderer->cleanUp();
    delete renderer;
    SDL_Quit();
}

void Game::runLoop() {
    while (isRunning) {
        processInput();
        updateGame();
        generateOutput();
    }
}

void Game::loadData() {

}

void Game::processInput() {
    SDL_Event ev;
    while(SDL_PollEvent(&ev)) {
        switch (ev.type) {
            case SDL_QUIT:
                isRunning = false;
                break;
            default:
                break;
        }
    }
    // 获取键盘输入的状态
    const Uint8 *state = SDL_GetKeyboardState(nullptr);
}

void Game::updateGame() {
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), ticksCount + 16)) {
    }
    float deltaTime = (SDL_GetTicks64() - ticksCount) / 1000.f;
    if (deltaTime > 0.05f) {
        deltaTime = 0.05f;
    }
    ticksCount = SDL_GetTicks64();
}

void Game::generateOutput() {
    renderer->render();
}

Game *Game::getInstance() {
    return INSTANCE;
}
