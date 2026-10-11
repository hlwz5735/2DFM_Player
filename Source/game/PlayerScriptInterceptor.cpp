//
// Created by limen on 2025/1/9.
//

#include "PlayerScriptInterceptor.hpp"

#include "2dfm/2dfmScriptItem.hpp"
#include "CharacterController.hpp"
#include "PlayerNode.hpp"

USING_NS_AX;

// 脚本推进（含跳帧补偿）统一在 ScriptInterceptorComponent::update 里做

void PlayerScriptInterceptor::setPlayerData(std::shared_ptr<KgtPlayer> data) {
    this->stop();
    this->playerData = std::move(data);
}

PlayerNode *PlayerScriptInterceptor::getPlayerNode() const {
    if (!playerNode) {
        playerNode = dynamic_cast<PlayerNode *>(getOwner());
    }
    return playerNode;
}

bool PlayerScriptInterceptor::shouldClearSpriteOnScriptEnd() const {
    // 有状态机（主角）：保留当前画面，等状态机切下一段脚本。
    // 没有状态机（“物”、选人界面展示）：脚本结束这个对象就到头了，清空并隐藏，随后整节点回收。
    return findCharacterController() == nullptr;
}

CharacterController *PlayerScriptInterceptor::findCharacterController() const {
    if (!controllerResolved) {
        controllerResolved = true;
        characterController = nullptr;
        // 状态机与解释器挂在同一个角色节点上，按组件名取回
        if (auto *node = getPlayerNode()) {
            characterController = dynamic_cast<CharacterController *>(node->getComponent("CharacterController"));
        }
    }
    return characterController;
}

int PlayerScriptInterceptor::pickNextScriptAfterFinish() {
    // 没有状态机就没人接管：本次执行到此结束（调用方随后走销毁流程）
    if (auto *controller = findCharacterController()) {
        return controller->pickNextScriptAfterFinish();
    }
    return -1;
}

void PlayerScriptInterceptor::onScriptStopped() {
    // 没有状态机接管：这个脚本执行器的终点就是销毁，先标记回收
    if (findCharacterController() != nullptr) {
        return;
    }
    if (auto *node = getPlayerNode()) {
        node->markForRecycle();
    }
}

void PlayerScriptInterceptor::interceptObjectCmd(const _2dfm::ObjectCmd *cmd) const {
    ScriptInterceptorComponent::interceptObjectCmd(cmd);
    if (const auto playerNode = getPlayerNode()) {
        auto parentNode = playerNode->getParent();
        AXASSERT(parentNode != nullptr, "玩家Node的父节点不应为空。");
        // TODO: 似乎需要考虑场景高度问题？
        auto newPlayerNode = utils::createInstance<PlayerNode>();
        newPlayerNode->setIsObjectNode(true);
        // 物体与角色同处一个世界，同样要跟着（虚拟）摄像机滚动
        newPlayerNode->setViewCamera(playerNode->getViewCamera());
        const auto interceptor = utils::createInstance<PlayerScriptInterceptor>();
        interceptor->setPlayerData(this->playerData);
        interceptor->initRunningScript(cmd->targetScriptId, cmd->targetPos);
        newPlayerNode->addInterceptor(interceptor);
        newPlayerNode->scheduleUpdate();
        if (cmd->isUseWindowPosition()) {
            newPlayerNode->setLogicPosition(cmd->getPosition());
        } else {
            newPlayerNode->setLogicPosition(playerNode->getLogicPosition() + cmd->getPosition());
        }
        // newPlayerNode->setLocalZOrder(999);
        parentNode->addChild(newPlayerNode);
        playerNode->addObjectNode(newPlayerNode, cmd->getManageNo());
    }

}
