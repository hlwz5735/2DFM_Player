//
// Created by limen on 2025/1/9.
//

#include "CharSelectionScene.hpp"

#include "2dfm/KgtGame.hpp"
#include "game/GameDemo.hpp"
#include "game/GameManager.hpp"

USING_NS_AX;

bool CharSelectionScene::init() {
    if (!Scene::init()) {
        return false;
    }
    const auto kgt = GameManager::getInstance().getKgtGame();
    objDemo = utils::createInstance<GameDemo>();
    // TODO: 暂时写死用故事模式的DEMO
    objDemo->load(kgt->getStoryModeCharSelectionDemoNo());
    addChild(objDemo);

    scheduleUpdate();

    return true;
}

void CharSelectionScene::update(float delta) {
    Scene::update(delta);
}
