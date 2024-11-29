//
// Created by limen on 2024/8/1.
//

#include "StageTestScene.hpp"
#include <format>
#include "2dfm/2dfmFileReader.hpp"
#include "2dfm/2dfmScriptItem.hpp"
#include "2dfm/KgtGame.hpp"
#include "2dfm/KgtStage.hpp"
#include "engine/AudioSystem.hpp"
#include "engine/Input.hpp"
#include "engine/KgtNode.hpp"
#include "game/GameConfig.hpp"
#include "game/GameManager.hpp"
#include "game/MoveComponent.hpp"
#include "game/ParallaxComponent.hpp"
#include "game/SeamlessScrollComponent.hpp"
#include "game/StageCameraNode.hpp"
#include "game/StageScriptInterceptor.hpp"

USING_NS_AX;

bool StageTestScene::init() {
    if (!Scene::init()) {
        return false;
    }

    cameraNode = utils::createInstance<StageCameraNode>();
    this->addChild(cameraNode);
    loadStage(0);

    this->scheduleUpdate();
    return true;
}
void StageTestScene::update(float delta) {
    Scene::update(delta);

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
    cameraNode->reset();

    for (int i = 1; i < stage->scripts.size(); ++i) {
        const auto &scriptInfo = stage->scripts[i];
        if (scriptInfo.name.empty()) {
            continue;
        }

        auto scriptNode = utils::createInstance<KgtNode>(&KgtNode::initWithVisibleHeight, GameConfig::stageHeight);
        auto interceptor = utils::createInstance<StageScriptInterceptor>();
        interceptor->setName("StageScriptInterceptor");
        interceptor->setStageData(stage);
        interceptor->setRunningScript(i);
        scriptNode->addComponent(interceptor);

        const auto startItem = reinterpret_cast<_2dfm::StageStart *>(stage->scriptItems[scriptInfo.startIdx]);
        auto parallaxComp = utils::createInstance<ParallaxComponent>(&ParallaxComponent::init, cameraNode);
        if (startItem->isHoriScroll()) {
            parallaxComp->setParallaxX(startItem->horiScroll / 100.f);
        }
        if (startItem->isVertScroll()) {
            parallaxComp->setParallaxY(startItem->vertScroll / 100.f);
        }
        scriptNode->addParallaxComp(parallaxComp);

        // if (startItem->isHoriLoop() || startItem->isVertLoop()) {
        //     auto seamlessComp = utils::createInstance<SeamlessScrollComponent>();
        //     seamlessComp->setHoriSeamless(startItem->isHoriLoop());
        //     seamlessComp->setVertSeamless(startItem->isVertLoop());
        //     scriptNode->addSeamlessComp(seamlessComp);
        // }

        scriptNode->scheduleUpdate();

        this->addChild(scriptNode);
        scriptNodes.emplace_back(scriptNode);
    }
}

void StageTestScene::unloadStage() {
    AudioSystem::getInstance().stopAll();
    while (!scriptNodes.empty()) {
        this->removeChild(scriptNodes.back());
        scriptNodes.pop_back();
    }
    delete stage;
}
