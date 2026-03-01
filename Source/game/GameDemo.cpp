//
// Created by limen on 2025/1/9.
//

#include "GameDemo.hpp"

#include "2dfm/2dfmFileReader.hpp"
#include "2dfm/KgtGame.hpp"
#include "AudioEngine.h"
#include "DemoScriptInterceptor.hpp"
#include "GameConfig.hpp"
#include "engine/KgtNode.hpp"

USING_NS_AX;

bool GameDemo::init() {
    if (!Node::init()) {
        return false;
    }

    return true;
}

void GameDemo::onExit() {
    AudioEngine::stopAll();
    AX_SAFE_DELETE(demo);
    Node::onExit();
}

void GameDemo::load(int demoNo) {
    this->demo = readDemoByNo(demoNo);
    createTexturesForCommonResource(demo, 0);

    for (int i = 1; i < demo->scripts.size(); ++i) {
        auto scriptNode = utils::createInstance<KgtNode>();
        scriptNode->setLogicPosition(Vec2::ZERO);

        const auto interceptor = utils::createInstance<DemoScriptInterceptor>();
        interceptor->setDemoData(demo);
        interceptor->initRunningScript(i);
        scriptNode->addInterceptor(interceptor);
        scriptNode->scheduleUpdate();

        this->addChild(scriptNode);
        scriptNodes.emplace_back(scriptNode);
    }
}

void GameDemo::unload() {
    AudioEngine::stopAll();
    while (!scriptNodes.empty()) {
        this->removeChild(scriptNodes.back());
        scriptNodes.pop_back();
    }
    AX_SAFE_DELETE(demo);
}
