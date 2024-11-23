//
// Created by limen on 2024/11/21.
//

#include "DemoScene.hpp"
#include "2dfm/2dfmFileReader.hpp"
#include "DemoScriptInterceptor.hpp"
#include "GameConfig.hpp"
#include "GameManager.hpp"
#include "MoveComponent.hpp"
#include "engine/AudioSystem.hpp"

USING_NS_AX;

bool DemoScene::initWithFile(std::string_view filePath) {
    if (!Scene::init()) {
        return false;
    }

    auto visibleSize = _director->getVisibleSize();

    KgtDemo *demo = readDemoFile(filePath);
    createTexturesForCommonResource(demo, 0);

    for (int i = 1; i < demo->scripts.size(); ++i) {
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

        auto interceptor = utils::createInstance<DemoScriptInterceptor>();
        interceptor->setDemoData(demo);
        interceptor->setRunningScript(i);
        scriptNode->addComponent(interceptor);

        scriptNode->scheduleUpdate();

        this->addChild(scriptNode);
        scriptNodes.emplace_back(scriptNode);
    }

    auto keyboardListener = EventListenerKeyboard::create();
    keyboardListener->onKeyPressed = AX_CALLBACK_2(DemoScene::onKeyPressed, this);
    keyboardListener->onKeyReleased = AX_CALLBACK_2(DemoScene::onKeyReleased, this);
    _eventDispatcher->addEventListenerWithFixedPriority(keyboardListener, 11);

    return true;
}
void DemoScene::onExit() {
    AudioSystem::getInstance()->stopAll();
    Scene::onExit();
}

void DemoScene::onKeyPressed(ax::EventKeyboard::KeyCode code, ax::Event *event) {

}

void DemoScene::onKeyReleased(ax::EventKeyboard::KeyCode code, ax::Event *event) {
    auto openDemoName = std::format("{}/{}.demo",
        GameConfig::getInstance().getGameBasePath(),
        GameManager::getInstance().getKgtGame()->getTitleDemoName());
    const auto openDemoScene = utils::createInstance<DemoScene>(&DemoScene::initWithFile, openDemoName);
    _director->replaceScene(openDemoScene);
}
