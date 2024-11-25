//
// Created by limen on 2024/11/21.
//

#include "DemoScene.hpp"
#include "2dfm/2dfmFileReader.hpp"
#include "DemoScriptInterceptor.hpp"
#include "GameConfig.hpp"
#include "GameManager.hpp"
#include "KgtScriptInterceptor.hpp"
#include "MoveComponent.hpp"
#include "engine/AudioSystem.hpp"
#include "engine/Input.hpp"
#include "engine/KgtNode.hpp"

USING_NS_AX;

bool DemoScene::initWithFile(std::string_view filePath, DemoType demoType) {
    if (!Scene::init()) {
        return false;
    }

    this->demoType = demoType;

    KgtDemo *demo = readDemoFile(filePath);
    createTexturesForCommonResource(demo, 0);

    for (int i = 1; i < demo->scripts.size(); ++i) {
        auto scriptNode = utils::createInstance<KgtNode>();
        scriptNode->setLogicPosition(Vec2::ZERO);

        const auto interceptor = utils::createInstance<DemoScriptInterceptor>();
        interceptor->setDemoData(demo);
        interceptor->setRunningScript(i);
        scriptNode->addComponent(interceptor);
        scriptNode->scheduleUpdate();

        this->addChild(scriptNode);
        scriptNodes.emplace_back(scriptNode);
    }

    scheduleUpdate();

    if (demoType == DemoType::TITLE) {
        return initTitle();
    }
    return true;
}
void DemoScene::update(float deltaTime) {
    Scene::update(deltaTime);
    switch (demoType) {
    case DemoType::OPENING:
        updateOpening();
        break;
    case DemoType::TITLE:
        updateTitle();
        break;
    default:
        break;
    }
}

void DemoScene::onExit() {
    AudioSystem::getInstance()->stopAll();
    Scene::onExit();
}

bool DemoScene::initTitle() {
    auto cursorNode = utils::createInstance<KgtNode>();
    auto kgtGame = GameManager::getInstance().getKgtGame();
    const auto interceptor = utils::createInstance<KgtScriptInterceptor>();
    interceptor->setKgtGame(kgtGame);
    interceptor->setRunningScript(78); // TODO: 待完善
    cursorNode->addComponent(interceptor);
    Vec2 pos = { 135, 325 };
    cursorNode->setLogicPosition(pos);
    cursorNode->scheduleUpdate();

    this->addChild(cursorNode);

    return true;
}

void DemoScene::updateOpening() {
    if (Input::getInstance().isAnyAttackButtonDown()) {
        auto openDemoName = std::format("{}/{}.demo", GameConfig::getInstance().getGameBasePath(),
                                        GameManager::getInstance().getKgtGame()->getTitleDemoName());
        const auto openDemoScene =
            utils::createInstance<DemoScene>(&DemoScene::initWithFile, openDemoName, DemoType::TITLE);
        _director->replaceScene(openDemoScene);
    }
}
void DemoScene::updateTitle() {
    if (Input::getInstance().isAnyAttackButtonDown()) {
        auto demoName = std::format("{}/{}.demo", GameConfig::getInstance().getGameBasePath(),
                                        GameManager::getInstance().getKgtGame()->getCharSelectionDemoName());
        const auto openDemoScene =
            utils::createInstance<DemoScene>(&DemoScene::initWithFile, demoName, DemoType::CHAR_SEL);
        _director->replaceScene(openDemoScene);
    }
}
