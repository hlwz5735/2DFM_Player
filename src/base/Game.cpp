//
// Created by 厉猛 on 2024-07-16.
//

#include "Game.hpp"
#include "InputSystem.hpp"
#include "Node.hpp"
#include "Renderer.hpp"
#include "Texture.hpp"
#include "AudioSystem.hpp"
#include "SpriteComponent.hpp"
#include "../2dfm/KgtGame.hpp"
#include "../2dfm/2dfmFileReader.hpp"
#include "../game/DemoScriptInterceptor.hpp"

const char *title = "2DFM Player";
Game *Game::INSTANCE = nullptr;
constexpr int winWidth = 640, winHeight = 480;

Game::Game()
{}

bool Game::initialize() {
    INSTANCE = this;

    renderer = new Renderer(this);
    renderer->initialize(title, winWidth, winHeight);
    inputSystem = new InputSystem();
    inputSystem->initialize();
    audioSystem = new AudioSystem(this);
    audioSystem->initialize();

    loadData();

    ticksCount = SDL_GetTicks64();
    return true;
}

void Game::cleanUp() {
    audioSystem->cleanUp();
    delete audioSystem;

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
    kgt = readKgtFile(gameConfig.gameBasePath + '/' + gameConfig.kgtFileName);
    for (auto &sf : kgt->spriteFrames) {
        const auto t = new Texture(renderer, &sf);
        kgt->pictures.emplace_back(t);
    }

    auto openDemoName = gameConfig.gameBasePath + '/'
//             + kgt->demoNames[static_cast<int>(kgt->demoConfig.openingDemoId) - 1]
            + kgt->demoNames[3]
            + ".demo";

    KgtDemo *demo = readDemoFile(openDemoName);
    for (auto &sf : demo->spriteFrames) {
        demo->pictures.emplace_back(new Texture(renderer, &sf));
    }
    for (int i = 1; i < demo->scripts.size(); ++i) {
        auto scriptNode = new Node(this);
        scriptNode->setPosition(Vector2::ZERO);
        new SpriteComponent(scriptNode, i);
        auto comp = new DemoScriptInterceptor(scriptNode);
        comp->setDemoData(demo);
        comp->setRunningScript(i);
    }
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
