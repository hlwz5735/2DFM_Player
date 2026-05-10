//
// Created by Qoder on 2026/5/10.
//

#include "PlayerTestScene.hpp"

#include "2dfm/2dfmFileReader.hpp"
#include "2dfm/KgtPlayer.hpp"
#include "engine/Input.hpp"
#include "game/GameConfig.hpp"
#include "game/GameStage.hpp"
#include "game/PlayerNode.hpp"
#include "game/PlayerScriptInterceptor.hpp"
#include "game/ResourcePool.hpp"

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

    // 创建角色节点
    playerNode = utils::createInstance<PlayerNode>(&PlayerNode::initWithVisibleHeight, GameConfig::stageHeight);
    auto interceptor = utils::createInstance<PlayerScriptInterceptor>();
    interceptor->setPlayerData(playerData);
    interceptor->initRunningScript(playerData->standScriptId);
    playerNode->addInterceptor(interceptor);

    // 设置初始位置（场景中央偏下）
    groundY = GameConfig::stageHeight - 64.f;
    playerNode->setLogicPosition(Vec2(GameConfig::stageWidth / 6.f, groundY));
    playerNode->scheduleUpdate();

    this->addChild(playerNode, 10);

    currentState = PlayerState::STAND;
    prevState = PlayerState::STAND;
}

void PlayerTestScene::update(float delta) {
    Scene::update(delta);

    if (!playerData || !playerNode) {
        return;
    }

    auto &input = Input::getInstance();
    input.update(delta);

    handleInput();
    updatePlayerState();
}

void PlayerTestScene::handleInput() {
    auto &input = Input::getInstance();
    auto dpad = input.getDPad();

    // 下蹲状态的处理
    if (currentState == PlayerState::CROUCHING || currentState == PlayerState::CROUCH) {
        if (dpad != Input::DPadDir::DOWN && dpad != Input::DPadDir::LEFT_DOWN && dpad != Input::DPadDir::RIGHT_DOWN) {
            // 松开下方向，站起
            currentState = PlayerState::STAND_UP;
        }
        return;
    }

    // 站起中不处理输入
    if (currentState == PlayerState::STAND_UP) {
        return;
    }

    // 跳跃
    if (dpad == Input::DPadDir::UP || dpad == Input::DPadDir::LEFT_UP || dpad == Input::DPadDir::RIGHT_UP) {
        if (dpad == Input::DPadDir::RIGHT_UP) {
            currentState = PlayerState::JUMP_FORWARD;
        } else if (dpad == Input::DPadDir::LEFT_UP) {
            currentState = PlayerState::JUMP_BACKWARD;
        } else {
            currentState = PlayerState::JUMP_UP;
        }
        return;
    }

    // 下蹲
    if (dpad == Input::DPadDir::DOWN || dpad == Input::DPadDir::LEFT_DOWN || dpad == Input::DPadDir::RIGHT_DOWN) {
        currentState = PlayerState::CROUCH;
        return;
    }

    // 行走
    if (dpad == Input::DPadDir::RIGHT) {
        currentState = PlayerState::WALK_FORWARD;
        return;
    }
    if (dpad == Input::DPadDir::LEFT) {
        currentState = PlayerState::WALK_BACKWARD;
        return;
    }

    // 无输入，回到站立
    currentState = PlayerState::STAND;
}

void PlayerTestScene::updatePlayerState() {
    if (currentState == prevState) {
        return;
    }

    int scriptId = playerData->standScriptId;

    switch (currentState) {
    case PlayerState::STAND:
        scriptId = playerData->standScriptId;
        break;
    case PlayerState::WALK_FORWARD:
        scriptId = playerData->forwardScriptId;
        break;
    case PlayerState::WALK_BACKWARD:
        scriptId = playerData->backwardScriptId;
        break;
    case PlayerState::JUMP_UP:
        scriptId = playerData->jumpUpScriptId;
        break;
    case PlayerState::JUMP_FORWARD:
        scriptId = playerData->jumpForwardScriptId;
        break;
    case PlayerState::JUMP_BACKWARD:
        scriptId = playerData->jumpBackwardScriptId;
        break;
    case PlayerState::FALLING:
        scriptId = playerData->fallingScriptId;
        break;
    case PlayerState::CROUCH:
        scriptId = playerData->crouchScriptId;
        break;
    case PlayerState::CROUCHING:
        scriptId = playerData->crouchingScriptId;
        break;
    case PlayerState::STAND_UP:
        scriptId = playerData->standUpScriptId;
        break;
    }

    switchPlayerScript(scriptId);
    prevState = currentState;
}

void PlayerTestScene::switchPlayerScript(int scriptId) {
    if (!playerNode || !playerData) {
        return;
    }
    auto interceptor = dynamic_cast<PlayerScriptInterceptor *>(playerNode->getInterceptor());
    if (interceptor) {
        interceptor->initRunningScript(scriptId);
    }
}

void PlayerTestScene::onExit() {
    if (gameStage) {
        gameStage->unload();
    }
    playerData.reset();
    Scene::onExit();
}
