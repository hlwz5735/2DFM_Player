//
// Created by limen on 2024/8/1.
//

#include "StageTestScene.hpp"
#include <format>
#include "2dfm/2dfmFileReader.hpp"
#include "2dfm/KgtGame.hpp"
#include "2dfm/KgtStage.hpp"
#include "GlobalData.hpp"
#include "MoveComponent.hpp"
#include "StageScriptInterceptor.hpp"
#include "engine/AudioSystem.hpp"

#include "engine/InputSystem.hpp"

USING_NS_AX;

bool StageTestScene::init() {
    if (!Scene::init()) {
        return false;
    }

    loadStage(0);

    updatePositionByCameraPos();

    this->scheduleUpdate();
    return true;
}
void StageTestScene::update(float delta) {
    Scene::update(delta);

    auto &is = InputSystem::getInstance()->getInputState();
    if (is.keyboard.isKeyHolding(EventKeyboard::KeyCode::KEY_D)) {
        cameraLoc.x += cameraSpeed;
    }
    if (is.keyboard.isKeyHolding(EventKeyboard::KeyCode::KEY_A)) {
        cameraLoc.x -= cameraSpeed;
    }
    if (is.keyboard.isKeyHolding(EventKeyboard::KeyCode::KEY_W)) {
        cameraLoc.y += cameraSpeed;
    }
    if (is.keyboard.isKeyHolding(EventKeyboard::KeyCode::KEY_S)) {
        cameraLoc.y -= cameraSpeed;
    }

    if (is.keyboard.isKeyDown(EventKeyboard::KeyCode::KEY_PG_DOWN)) {
        auto kgt = GlobalData::getInstance()->getKgtGame();
        unloadStage();
        stageNo = stageNo < kgt->stageNames.size() - 1 ? stageNo + 1 : 0;
        loadStage(stageNo);
    }
    if (is.keyboard.isKeyDown(EventKeyboard::KeyCode::KEY_PG_UP)) {
        auto kgt = GlobalData::getInstance()->getKgtGame();
        unloadStage();
        stageNo = stageNo > 0 ? stageNo - 1 :  kgt->stageNames.size() - 1;
        loadStage(stageNo);
    }

    updatePositionByCameraPos();
}

void StageTestScene::updatePositionByCameraPos() {
    cameraLoc.x = clampf(cameraLoc.x, stageWidth / 4, stageWidth / 4 * 3);
    cameraLoc.y = clampf(cameraLoc.y, stageHeight / 4, stageHeight / 4 * 3);
    setPosition(stageWidth / 4 - cameraLoc.x, stageHeight / 4 * 3 - cameraLoc.y);
}

void StageTestScene::loadStage(int stageNo) {
    auto &gameConfig = GlobalData::getInstance()->getGameConfig();
    auto kgt = GlobalData::getInstance()->getKgtGame();
    auto stageName = std::format("{}/{}.stage", gameConfig.gameBasePath, kgt->stageNames[stageNo]);

    this->stage = readStageFile(stageName);
    createTexturesForCommonResource(stage, 0);

    for (int i = 1; i < stage->scripts.size(); ++i) {
        auto scriptNode = utils::createInstance<Node>();
        scriptNode->setPosition(0, stageHeight / 2);

        auto mc = utils::createInstance<MoveComponent>();
        mc->setName("MoveComponent");
        scriptNode->addComponent(mc);

        auto sprite = utils::createInstance<Sprite>();
        sprite->setName("SpriteComponent");
        sprite->setPosition(Vec2::ZERO);
        sprite->setAnchorPoint(Vec2(0, 1));
        scriptNode->addChild(sprite);

        auto interceptor = utils::createInstance<StageScriptInterceptor>();
        interceptor->setStageData(stage);
        interceptor->setRunningScript(i);
        scriptNode->addComponent(interceptor);

        scriptNode->scheduleUpdate();

        this->addChild(scriptNode);
    }
}

void StageTestScene::unloadStage() {
    AudioSystem::getInstance()->stopAll();
    this->removeAllChildren();
    delete stage;
}
