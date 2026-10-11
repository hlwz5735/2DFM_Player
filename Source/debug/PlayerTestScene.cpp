//
// Created by Qoder on 2026/5/10.
//

#include "PlayerTestScene.hpp"

#include <string>

#include "2dfm/CommonResource.hpp"
#include "2dfm/KgtPlayer.hpp"
#include "engine/Input.hpp"
#include "game/CharacterController.hpp"
#include "game/GameConfig.hpp"
#include "game/GameStage.hpp"
#include "game/PlayerNode.hpp"
#include "game/PlayerScriptInterceptor.hpp"
#include "game/ResourcePool.hpp"
#include "SceneNavUtil.hpp"

USING_NS_AX;

bool PlayerTestScene::init() {
    if (!Scene::init()) {
        return false;
    }

    // 创建场景容器
    gameStage = utils::createInstance<GameStage>();
    this->addChild(gameStage);

    loadStageAndPlayer();

    this->scheduleUpdate();
    addBackToMainSceneButton(this);  // 底部“返回菜单”按钮
    return true;
}

void PlayerTestScene::loadStageAndPlayer() {
    // 加载第1个场景
    gameStage->load(0);

    // 加载第1个角色
    playerData = ResourcePool::getInstance().loadPlayer(0);
    if (!playerData) {
        AXLOGE("Failed to load player 0");
        return;
    }
    createTexturesForCommonResource(playerData.get(), 0);
    playerData->initBasicScriptInfos();

    // 地面：场景底部往上 64
    groundY = GameConfig::stageHeight - 64.f;

    // 创建角色节点
    playerNode = utils::createInstance<PlayerNode>(&PlayerNode::initWithVisibleHeight, GameConfig::stageHeight);
    auto interceptor = utils::createInstance<PlayerScriptInterceptor>();
    interceptor->setPlayerData(playerData);
    playerNode->addInterceptor(interceptor);

    // 先摆好初始位置：控制器会以此时的高度作为地面线（随后又显式 setGroundY 了一遍）
    playerNode->setLogicPosition(Vec2(GameConfig::stageWidth / 6.f, groundY));
    // 角色和背景层取同一个视口偏移，才会跟着摄像机一起滚动（见 PlayerNode::setViewCamera）
    playerNode->setViewCamera(gameStage->getCameraNode());
    playerNode->scheduleUpdate();

    this->addChild(playerNode, 10);

    // 角色控制器（状态机），默认就是“站立 + 通常”，并自动开始播放“站立”脚本
    controller = utils::createInstance<CharacterController>();
    controller->setPlayerData(playerData);
    controller->setGroundY(groundY);
    playerNode->addComponent(controller);

    // 摄像机跟随角色（水平方向），并夹在场景范围内
    gameStage->setCameraFollowTarget(playerNode);

    // 左上角的状态显示
    stateLabel = Label::createWithTTF(TTFConfig("fonts/msyh.ttc", 16), "状态");
    stateLabel->setAnchorPoint(Vec2(0, 1));
    stateLabel->setPosition(8.f, _director->getVisibleSize().height - 8.f);
    this->addChild(stateLabel, 1000);
    refreshStateLabel();
}

void PlayerTestScene::update(float delta) {
    Scene::update(delta);

    refreshStateLabel();

    // 注意：这里不要调用 Input::update()。
    // Input 自己以优先级 2 挂在调度器上（见 Input::init），而场景与节点是优先级 0，
    // 所以每帧的次序是「节点先读输入 → Input 再翻页」，读到的“上一帧按键”才是真正的上一帧，
    // 边沿判定（新按下）才成立；在这里再翻一次页，会把本帧状态写进“上一帧”，边沿判定永远为假。
}

void PlayerTestScene::refreshStateLabel() {
    if (stateLabel == nullptr || controller == nullptr) {
        return;
    }
    const std::string text = std::string("位置：") + toString(controller->getPositionState())
                             + "　姿势：" + toString(controller->getPoseState())
                             + "　动作：" + toString(controller->getNormalAction());
    // setString 会重建文字纹理，状态没变就不要每帧都刷
    if (text == lastStateText) {
        return;
    }
    lastStateText = text;
    stateLabel->setString(text);
}

void PlayerTestScene::onExit() {
    // 先解开互相持有的指针，避免拆除顺序造成的悬空访问
    if (gameStage) {
        gameStage->setCameraFollowTarget(nullptr);
        gameStage->unload();
    }
    if (playerNode) {
        playerNode->setViewCamera(nullptr);
    }
    playerData.reset();
    Scene::onExit();
}
