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

#include "engine/InputSystem.hpp"

USING_NS_AX;

bool StageTestScene::init() {
    if (!Scene::init()) {
        return false;
    }
    auto visibleSize = _director->getVisibleSize();

    auto &gameConfig = GlobalData::getInstance()->getGameConfig();
    auto kgt = GlobalData::getInstance()->getKgtGame();

    auto stageName = std::format("{}/{}.stage", gameConfig.gameBasePath, kgt->stageNames[4]);

    KgtStage *stage = readStageFile(stageName);
    createTexturesForCommonResource(stage, 0);

    for (int i = 1; i < stage->scripts.size(); ++i) {
        auto scriptNode = utils::createInstance<Node>();
        scriptNode->setPosition(0, visibleSize.height);

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

    updatePositionByCameraPos();
}
void StageTestScene::updatePositionByCameraPos() {
    cameraLoc.x = clampf(cameraLoc.x, stageWidth / 4, stageWidth / 4 * 3);
    cameraLoc.y = clampf(cameraLoc.y, stageHeight / 4, stageHeight / 4 * 3);
    setPosition(-cameraLoc.x + stageWidth / 4, stageHeight / 4 * 3 - cameraLoc.y);
}
