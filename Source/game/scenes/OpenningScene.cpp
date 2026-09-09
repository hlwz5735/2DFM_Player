//
// Created by limen on 2025/1/9.
//

#include "OpenningScene.hpp"

#include "2dfm/KgtGame.hpp"
#include "TitleScene.hpp"
#include "engine/Input.hpp"
#include "game/GameConfig.hpp"
#include "game/GameDemo.hpp"
#include "game/ResourcePool.hpp"

USING_NS_AX;

bool OpenningScene::init() {
    if (!Scene::init()) {
        return false;
    }

    auto kgtGame = ResourcePool::getInstance().getKgtGame();
    if (kgtGame == nullptr) {
        AXLOGE("[OpenningScene] kgtGame is null, abort scene");
        return false;
    }

    const int openingDemoNo = kgtGame->getOpeningDemoNo();

    this->objDemo = utils::createInstance<GameDemo>();
    if (objDemo == nullptr) {
        AXLOGE("[OpenningScene] failed to create GameDemo");
        return false;
    }
    objDemo->load(openingDemoNo);
    if (!objDemo->isLoaded()) {
        // 开场 DEMO 加载失败时不要切进空场景（会表现为黑屏），让调用方保留原界面
        AXLOGE("[OpenningScene] opening demo {} failed to load, abort scene", openingDemoNo);
        return false;
    }
    addChild(objDemo);

    this->scheduleUpdate();

    return true;
}

void OpenningScene::update(float deltaTime) {
    Scene::update(deltaTime);
    if (Input::getInstance().isAnyAttackButtonDown()) {
        const auto openDemoScene = utils::createInstance<TitleScene>();
        _director->replaceScene(openDemoScene);
    }
}
