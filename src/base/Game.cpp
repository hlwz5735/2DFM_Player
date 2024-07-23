//
// Created by 厉猛 on 2024-07-16.
//


#include "Game.hpp"
#include "InputSystem.hpp"
#include "Node.hpp"
#include "Renderer.hpp"
#include "Texture.hpp"
#include "../2dfm/2dfmFileReader.hpp"
#include "../game/TestComponent.hpp"
#include <SDL_image.h>
#include <SDL_mixer.h>

#include "SpriteComponent.hpp"

const char *title = "Game";
Game *Game::INSTANCE = nullptr;
constexpr int winWidth = 640, winHeight = 480;

Game::Game()
{}

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
        SDL_Log("Sdl_mixer could not initialize! SDL_mixer Error: %s", Mix_GetError());
        SDL_Quit();
        return false;
    }

    INSTANCE = this;

    renderer = new Renderer(this);
    renderer->initialize(nullptr, winWidth, winHeight);
    inputSystem = new InputSystem();
    inputSystem->initialize();

    loadData();

    ticksCount = SDL_GetTicks64();
    return true;
}

void Game::cleanup() {
    Mix_CloseAudio();

    renderer->cleanUp();
    delete renderer;

    inputSystem->cleanUp();
    delete inputSystem;

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
    kgt = readKgtFile("D:\\Games\\dong_dong_never_die_170804\\GAME\\GAME.kgt");

    for (auto &sf : kgt.spriteFrames) {
        const auto t = new Texture(renderer, &sf);
        renderer->addTexture(t);
    }

    auto root = new Node(this);
    root->setPosition(Vector2(winWidth / 2, winHeight / 2));
    auto sprite = new SpriteComponent(root);
    auto tc = new TestComponent(root);
    tc->setKgtGame(&kgt);

    auto &sound = kgt.sounds[1];
    SDL_RWops *wavData = SDL_RWFromMem(sound->content, sound->header.size);
    auto chunk = Mix_LoadWAV_RW(wavData, false);
    Mix_PlayChannel(-1, chunk, 5);
}

void Game::addGameObject(Node *obj) {
    if (updatingGameObjects) {
        pendingObjects.emplace_back(obj);
    } else {
        gameObjects.emplace_back(obj);
    }
}

void Game::removeGameObject(Node *obj) {
    // 首先检查是否在待加入列表中
    auto iter = std::find(pendingObjects.begin(), pendingObjects.end(), obj);
    if (iter != pendingObjects.end()) {
        std::iter_swap(iter, pendingObjects.end() - 1);
        pendingObjects.pop_back();
    }
    iter = std::find(gameObjects.begin(), gameObjects.end(), obj);
    if (iter != gameObjects.end()) {
        std::iter_swap(iter, gameObjects.end() - 1);
        gameObjects.pop_back();
    }
}

void Game::processInput() {
    inputSystem->preUpdate();

    SDL_Event ev;
    while(SDL_PollEvent(&ev)) {
        switch (ev.type) {
            case SDL_QUIT:
                isRunning = false;
                break;
            case SDL_MOUSEWHEEL:
                inputSystem->processInput(ev);
                break;
            default:
                break;
        }
    }

    inputSystem->update();
    if (inputSystem->isKeyDown(SDL_SCANCODE_ESCAPE)) {
        isRunning = false;
    }
    //updatingGameObjects = true;
    //for (auto obj : gameObjects) {
    //    obj->processInput();
    //}
    //updatingGameObjects = false;
}

void Game::updateGame() {
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), ticksCount + 16)) {
    }
    float deltaTime = (SDL_GetTicks64() - ticksCount) / 1000.f;
    if (deltaTime > 0.05f) {
        deltaTime = 0.05f;
    }
    ticksCount = SDL_GetTicks64();

    updatingGameObjects = true;
    for (auto obj : gameObjects) {
        obj->__update(deltaTime);
    }
    // 如果在此期间有新的游戏对象添加进来，加入游戏列表
    updatingGameObjects = false;
    for (auto obj : pendingObjects) {
        gameObjects.emplace_back(obj);
    }
    pendingObjects.clear();

    // 收集标记为死亡的对象并移除之
    std::vector<Node *> deadNodes;
    for (auto obj : gameObjects) {
        if (obj->getState() == Node::State::DEAD) {
            deadNodes.emplace_back(obj);
        }
    }
    for (auto obj : deadNodes) {
        delete obj;
    }
}

void Game::generateOutput() {
    renderer->render();
}

Game *Game::getInstance() {
    return INSTANCE;
}
