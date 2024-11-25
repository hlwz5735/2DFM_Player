//
// Created by limen on 2024/8/1.
//

#include "StageTestScene.hpp"
#include "2dfm/2dfmFileReader.hpp"
#include "2dfm/KgtGame.hpp"
#include "2dfm/KgtStage.hpp"
#include "game/GameConfig.hpp"
#include "game/GameManager.hpp"
#include "game/MoveComponent.hpp"
#include "game/StageScriptInterceptor.hpp"
#include "engine/AudioSystem.hpp"
#include "engine/Input.hpp"
#include "engine/KgtNode.hpp"
#include <format>

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

    auto dpad = Input::getInstance().getDPad();
    if (dpad == Input::DPadDir::LEFT_DOWN) {
        cameraLoc.x -= cameraSpeed;
        cameraLoc.y -= cameraSpeed;
    } else if (dpad == Input::DPadDir::DOWN) {
        cameraLoc.y -= cameraSpeed;
    } else if (dpad == Input::DPadDir::RIGHT_DOWN) {
        cameraLoc.x += cameraSpeed;
        cameraLoc.y -= cameraSpeed;
    } else if (dpad == Input::DPadDir::LEFT) {
        cameraLoc.x -= cameraSpeed;
    } else if (dpad == Input::DPadDir::RIGHT) {
        cameraLoc.x += cameraSpeed;
    } else if (dpad == Input::DPadDir::LEFT_UP) {
        cameraLoc.x -= cameraSpeed;
        cameraLoc.y += cameraSpeed;
    } else if (dpad == Input::DPadDir::UP) {
        cameraLoc.y += cameraSpeed;
    } else if (dpad == Input::DPadDir::RIGHT_UP) {
        cameraLoc.x += cameraSpeed;
        cameraLoc.y += cameraSpeed;
    }

    // 需要注意可能会遇到空场景
    const auto kgt = GameManager::getInstance().getKgtGame();
    if (Input::getInstance().isButtonDown(Input::GameButton::F)) {
        unloadStage();
        stageNo = stageNo < kgt->stageNames.size() - 1 ? stageNo + 1 : 0;
        loadStage(stageNo);
    }
    if (Input::getInstance().isButtonDown(Input::GameButton::E)) {
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
    auto kgt = GameManager::getInstance().getKgtGame();
    auto stageName = kgt->stageNames[stageNo];
    if (stageName.empty()) {
        return;
    }
    auto &gameConfig = GameConfig::getInstance();
    auto fullStageName = std::format("{}/{}.stage", gameConfig.getGameBasePath(), kgt->stageNames[stageNo]);

    this->stage = readStageFile(fullStageName);
    createTexturesForCommonResource(stage, 0);

    for (int i = 1; i < stage->scripts.size(); ++i) {
        auto scriptNode = utils::createInstance<KgtNode>();
        scriptNode->setPosition(0, stageHeight / 2);

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
