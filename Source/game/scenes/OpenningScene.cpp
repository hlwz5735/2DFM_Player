//
// Created by limen on 2025/1/9.
//

#include "OpenningScene.hpp"

#include "2dfm/KgtGame.hpp"
#include "TitleScene.hpp"
#include "engine/Input.hpp"
#include "game/GameConfig.hpp"
#include "game/GameDemo.hpp"
#include "game/GameManager.hpp"

USING_NS_AX;

bool OpenningScene::init() {
    if (!Scene::init()) {
        return false;
    }

    auto kgtGame = GameManager::getInstance().getKgtGame();

    this->objDemo = utils::createInstance<GameDemo>();
    objDemo->load(kgtGame->getOpeningDemoNo());
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
