//
// Created by limen on 2025/1/9.
//

#include "GameStage.hpp"

#include "2dfm/2dfmFileReader.hpp"
#include "2dfm/2dfmScriptItem.hpp"
#include "AudioEngine.h"
#include "GameConfig.hpp"
#include "GameManager.hpp"
#include "ParallaxComponent.hpp"
#include "SeamlessScrollComponent.hpp"
#include "StageCameraNode.hpp"
#include "StageNode.hpp"
#include "StageScriptInterceptor.hpp"
#include "engine/KgtNode.hpp"

USING_NS_AX;

bool GameStage::init() {
    if (!Node::init()) {
        return false;
    }

    cameraNode = utils::createInstance<StageCameraNode>();
    this->addChild(cameraNode);

    this->scheduleUpdate();
    return true;
}

void GameStage::update(float delta) {
    Node::update(delta);

    // 需要注意可能会遇到空场景
    // const auto kgt = GameManager::getInstance().getKgtGame();
}

void GameStage::onExit() {
    AX_SAFE_DELETE(stage);
    Node::onExit();
}

void GameStage::load(int stageNo) {
    this->stage = readStageByNo(stageNo);
    GameManager::getInstance().setKgtStage(stage);
    createTexturesForCommonResource(stage, 0);
    cameraNode->reset();

    for (int i = 1; i < stage->scripts.size(); ++i) {
        const auto &scriptInfo = stage->scripts[i];
        if (scriptInfo.name.empty()) {
            continue;
        }

        auto scriptNode = utils::createInstance<StageNode>(&StageNode::initWithVisibleHeight, GameConfig::stageHeight);
        auto interceptor = utils::createInstance<StageScriptInterceptor>();
        interceptor->setStageData(stage);
        interceptor->initRunningScript(i);
        scriptNode->addInterceptor(interceptor);

        const auto startItem = reinterpret_cast<_2dfm::StageStart *>(stage->scriptItems[scriptInfo.startIdx]);
        auto parallaxComp = utils::createInstance<ParallaxComponent>(&ParallaxComponent::init, cameraNode);
        if (startItem->isHoriScroll()) {
            parallaxComp->setParallaxX(startItem->horiScroll / 100.f);
        } else {
            parallaxComp->setParallaxX(0.f);
        }
        if (startItem->isVertScroll()) {
            parallaxComp->setParallaxY(startItem->vertScroll / 100.f);
        } else {
            parallaxComp->setParallaxY(0.f);
        }
        scriptNode->addParallaxComp(parallaxComp);

        if (startItem->isHoriLoop() || startItem->isVertLoop()) {
            auto seamlessComp = utils::createInstance<SeamlessScrollComponent>();
            seamlessComp->setHoriSeamless(startItem->isHoriLoop());
            seamlessComp->setVertSeamless(startItem->isVertLoop());
            scriptNode->addSeamlessComp(seamlessComp);
        }

        scriptNode->scheduleUpdate();

        this->addChild(scriptNode);
        scriptNodes.emplace_back(scriptNode);
    }
}

void GameStage::unload() {
    AudioEngine::stopAll();
    while (!scriptNodes.empty()) {
        this->removeChild(scriptNodes.back());
        scriptNodes.pop_back();
    }
    AX_SAFE_DELETE(stage);
    GameManager::getInstance().setKgtStage(nullptr);
}
