//
// Created by limen on 2024/8/1.
//

#include "StageTestScene.hpp"
#include "2dfm/2dfmFileReader.hpp"
#include "2dfm/KgtStage.hpp"
#include "2dfm/KgtGame.hpp"
#include "GlobalData.hpp"
#include "MoveComponent.hpp"
#include "StageScriptInterceptor.hpp"
#include <format>

USING_NS_AX;

bool StageTestScene::init() {
    if (!Scene::init()) {
        return false;
    }
    auto visibleSize = _director->getVisibleSize();

    auto &gameConfig = GlobalData::getInstance()->getGameConfig();
    auto kgt = GlobalData::getInstance()->getKgtGame();

    auto stageName = std::format("{}/{}.stage", gameConfig.gameBasePath, kgt->stageNames[25]);

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

    // scheduleUpdate();
    auto mouseListener           = EventListenerMouse::create();
    mouseListener->onMouseMove   = AX_CALLBACK_1(StageTestScene::onMouseMove, this);
    mouseListener->onMouseUp     = AX_CALLBACK_1(StageTestScene::onMouseUp, this);
    mouseListener->onMouseDown   = AX_CALLBACK_1(StageTestScene::onMouseDown, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);

    setPosition(-320, 480);

    return true;
}
void StageTestScene::onMouseDown(ax::Event *event) {
    auto e = static_cast<EventMouse *>(event);
    if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {
        ploc = e->getLocationInView();
    }
}
void StageTestScene::onMouseUp(ax::Event *event) {
    auto e = static_cast<EventMouse *>(event);
    if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {
        ploc = Vec2::ZERO;
        loc = Vec2::ZERO;
    }}
void StageTestScene::onMouseMove(ax::Event *event) {
    auto e = static_cast<EventMouse *>(event);
    auto btn = e->getMouseButton();
    if (btn == EventMouse::MouseButton::BUTTON_RIGHT) {
        loc = e->getLocationInView();
        Vec2 delta = loc - ploc;
        AXLOG("delta: %.0f, %.0f", delta.x, delta.y);
        auto oriPos = getPosition();
        auto visibleSize = _director->getVisibleSize();
        setPosition(
            ax::clampf(oriPos.x + delta.x, -640, 0),
            ax::clampf(oriPos.y + delta.y, 0, visibleSize.height)
        );
    }
}
