//
// Created by limen on 2024/8/1.
//

#include "StageTestScene.hpp"
#include "2dfm/KgtGame.hpp"
#include "engine/Input.hpp"
#include "game/GameConfig.hpp"
#include "game/ResourcePool.hpp"
#include "game/GameStage.hpp"
#include "SceneNavUtil.hpp"

USING_NS_AX;

bool StageTestScene::init() {
    if (!Scene::init()) {
        return false;
    }

    stage = utils::createInstance<GameStage>();
    loadStage(0);
    addChild(stage);

    this->scheduleUpdate();
    addBackToMainSceneButton(this);  // 底部“返回菜单”按钮
    return true;
}
void StageTestScene::update(float delta) {
    Scene::update(delta);

    // 需要注意可能会遇到空场景
    const auto kgt = ResourcePool::getInstance().getKgtGame();
    if (Input::getInstance().isAttackPressed(Input::AttackButton::F)) {
        unloadStage();
        stageNo = stageNo < kgt->stageNames.size() - 1 ? stageNo + 1 : 0;
        loadStage(stageNo);
    }
    if (Input::getInstance().isAttackPressed(Input::AttackButton::E)) {
        unloadStage();
        stageNo = stageNo > 0 ? stageNo - 1 :  kgt->stageNames.size() - 1;
        loadStage(stageNo);
    }
}

void StageTestScene::loadStage(int stageNo) const {
    stage->load(stageNo);
}

void StageTestScene::unloadStage() const {
    stage->unload();
}
